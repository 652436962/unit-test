#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <pthread.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#define DEFAULT_GAP 200 /* ms */

#define MODE_RANDOM	0
#define MODE_INCREMENT	1
#define MODE_FIX	2

static int write_count = 0;
static int read_count = 0;
static int err_count = 0;
static const unsigned char dlc2len[] = {0, 1, 2, 3, 4, 5, 6, 7,
                    8, 12, 16, 20, 24, 32, 48, 64};

/* get data length from can_dlc with sanitized can_dlc */
unsigned char can_dlc2len(unsigned char can_dlc)
{
    return dlc2len[can_dlc & 0x0F];
}

static const unsigned char len2dlc[] = {0, 1, 2, 3, 4, 5, 6, 7, 8,      /* 0 - 8 */
                    9, 9, 9, 9,             /* 9 - 12 */
                    10, 10, 10, 10,             /* 13 - 16 */
                    11, 11, 11, 11,             /* 17 - 20 */
                    12, 12, 12, 12,             /* 21 - 24 */
                    13, 13, 13, 13, 13, 13, 13, 13,     /* 25 - 32 */
                    14, 14, 14, 14, 14, 14, 14, 14,     /* 33 - 40 */
                    14, 14, 14, 14, 14, 14, 14, 14,     /* 41 - 48 */
                    15, 15, 15, 15, 15, 15, 15, 15,     /* 49 - 56 */
                    15, 15, 15, 15, 15, 15, 15, 15};    /* 57 - 64 */

/* map the sanitized data length to an appropriate data length code */
unsigned char can_len2dlc(unsigned char len)
{
    if (len > 64)
        return 0xF;

    return len2dlc[len];
}


static volatile int running = 1;
static unsigned long long enobufs_count;

void sigterm(int signo)
{
	running = 0;
	printf("\nCounted %llu ENOBUFS return values on write().\n\n",
		       enobufs_count);
	printf("write_count = %d read_count = %d err_count = %d\n", write_count, read_count, err_count);
	exit(0);
}

void *canfd_recv(void *argv)
{
	int mtu, maxdlen;
	int opt;
	int s; /* socket */

	struct sockaddr_can addr;
	static struct canfd_frame frame;
	int nbytes;
	int i;
	struct ifreq ifr;

	struct timeval now;

	/* set seed value for pseudo random numbers */
	gettimeofday(&now, NULL);
	srandom(now.tv_usec);

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
	}

	addr.can_family = AF_CAN;

	strcpy(ifr.ifr_name, "can1");
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		perror("SIOCGIFINDEX");
	}
	addr.can_ifindex = ifr.ifr_ifindex;

//	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	int enable_canfd = 1;

	/* check if the frame fits into the CAN netdevice */
	if (ioctl(s, SIOCGIFMTU, &ifr) < 0) {
		perror("SIOCGIFMTU");
	}

	if (ifr.ifr_mtu != CANFD_MTU) {
		printf("CAN interface ist not CAN FD capable - sorry.\n");
	}

	/* interface is ok - try to switch the socket into CAN FD mode */
	if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd))){
		printf("error when enabling CAN FD support\n");
	}

	/* ensure discrete CAN FD length values 0..8, 12, 16, 20, 24, 32, 64 */
	frame.len = can_dlc2len(can_len2dlc(frame.len));

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
	}

	while (1) {
        int nbytes = read(s, &frame, sizeof(frame));
		if (nbytes > 0) {
			printf("nbytes = %d\n", nbytes);
			printf("ID=0x%x\n", frame.can_id);
			for (uint8_t i = 0; i < sizeof(frame.data); i++)  {
				if (frame.data[i] == (i + 0x10))
					printf("%02x ", frame.data[i]);
				else
					err_count++;
			}
			printf("\n");
			++read_count;
        }
	}

}

int main(int argc, char **argv)
{
	unsigned char canfd = 0;
	unsigned char mix = 0;
	int mtu, maxdlen;

	int opt;
	int s; /* socket */
	struct pollfd fds;

	struct sockaddr_can addr;
	static struct canfd_frame frame;
	int nbytes;
	int i;
	struct ifreq ifr;

	struct timeval now;

	system("ip link set can0 up type can bitrate 500000 sample-point 0.75 dbitrate 4000000 dsample-point 0.8 fd on");
	system("ip link set can1 up type can bitrate 500000 sample-point 0.75 dbitrate 4000000 dsample-point 0.8 fd on");
	sleep(1);

	/* set seed value for pseudo random numbers */
	gettimeofday(&now, NULL);
	srandom(now.tv_usec);

	signal(SIGTERM, sigterm);
	signal(SIGHUP, sigterm);
	signal(SIGINT, sigterm);

	mix = 1;
	canfd = 1; /* to switch the socket into CAN FD mode */

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("socket");
		return 1;
	}

	addr.can_family = AF_CAN;

	strcpy(ifr.ifr_name, "can1");
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		perror("SIOCGIFINDEX");
		return 1;
	}
	addr.can_ifindex = ifr.ifr_ifindex;

	/* disable default receive filter on this RAW socket */
	/* This is obsolete as we do not read from the socket at all, but for */
	/* this reason we can remove the receive list in the Kernel to save a */
	/* little (really a very little!) CPU usage.                          */
	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	int enable_canfd = 1;

	/* check if the frame fits into the CAN netdevice */
	if (ioctl(s, SIOCGIFMTU, &ifr) < 0) {
		perror("SIOCGIFMTU");
		return 1;
	}

	if (ifr.ifr_mtu != CANFD_MTU) {
		printf("CAN interface ist not CAN FD capable - sorry.\n");
		return 1;
	}

	/* interface is ok - try to switch the socket into CAN FD mode */
	if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd))){
		printf("error when enabling CAN FD support\n");
		return 1;
	}

	/* ensure discrete CAN FD length values 0..8, 12, 16, 20, 24, 32, 64 */
	frame.len = can_dlc2len(can_len2dlc(frame.len));


	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	pthread_t tid;
	pthread_create(&tid, NULL, canfd_recv, NULL);

	while (1) {
		frame.flags = 0;

		mtu = CANFD_MTU;

		frame.can_id = random();
		frame.can_id &= CAN_SFF_MASK;
		frame.len = can_dlc2len(0xF);

		for(uint8_t i = 0x0; i < frame.len; i++) {
			frame.data[i] = 0x10 + i;
		}

resend:
		nbytes = write(s, &frame, mtu);
		if (nbytes < 0) {
			if (errno != ENOBUFS) {
				perror("write");
			}
			enobufs_count++;

		} else if (nbytes < mtu) {
			fprintf(stderr, "write: incomplete CAN frame\n");
		}

		write_count++;

		frame.can_id++;

		if (mix) {
			i = random();
			canfd = i&2;
		}
		usleep(100 * 1000);
	}

	close(s);

	return 0;
}
