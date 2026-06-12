#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define PKT_HEAD_LEN 6
#define OP_MASK 0
#define BITS_MASK   1
#define ADDR_MASK_0 2
#define ADDR_MASK_1 3
#define LEN_MASK_0 4
#define LEN_MASK_1 5
#define SUNXI_OP_WRITE 0x01
#define SUNXI_OP_READ 0x02
#define PKT_HEAD_DELAY 200
#define PKT_XFER_DELAY 1000
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define KB (1024)
#define MB (1024 * KB)
#define US (1000)
#define MS (1000 * US)
#define S (1000 * MS)
#include <stdint.h>
struct sunxi_spi_slave_head {
	uint8_t op_code;
	uint8_t bits;
	uint16_t addr;
	uint16_t len;

};
static const char *device = "/dev/spidev1.0";
static uint32_t addr;
static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 1000000;
static int verbose;
static int transfer_size = 32;
static int iterations;
static int ready;

static void pabort(const char *s) 
{
	if (errno != 0)
		perror(s);
	else
		printf("%s\n", s);
	abort();
}

static void hex_dump(const void *src, size_t length, size_t line_size,
                     char *prefix) 
{
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;
	unsigned char c;
	printf("%s | ", prefix);
	while (length-- > 0) {
		printf("%02X ", *address++);
		if (!(++i % line_size) || (length == 0 && i % line_size)) {
			if (length == 0) {
				while (i++ % line_size)
					printf("__ ");
			}
			printf(" |");
			while (line < address) {
				c = *line++;
				printf("%c", (c < 32 || c > 126) ? '.' : c);
			}
			printf("|\n");
			if (length > 0)
				printf("%s | ", prefix);
		}
	}
}

static void show_transfer_info(unsigned long size, unsigned long time) 
{
	double rate;
	printf("total size : ");
	if (size >= MB) {
		printf("%.2lf MB", (double)size / (double)MB);
	} else if (size >= KB) {
		printf("%.2lf KB", (double)size / (double)KB);
	} else {
		printf("%lu B", size);
	}
	printf("\n");
	printf("total time : ");
	if (time >= S) {
		printf("%.2lf s", (double)time / (double)S);
	} else if (time >= MS) {
		printf("%.2lf ms", (double)time / (double)MS);
	} else if (time >= US) {
		printf("%.2lf us", (double)time / (double)US);
	} else {
		printf("%lu ns", time);
	}
	printf("\n");
	rate = ((double)size / (double)MB) / ((double)time / (double)S);
	printf("averange rate: %.2lf MB/s\n", rate);
}

static int transfer_pkg_create(char *buf, struct sunxi_spi_slave_head *head) 
{
	buf[OP_MASK] = head->op_code;
	buf[BITS_MASK] = 0x00;
	buf[ADDR_MASK_0] = (head->addr >> 8) & 0xff;
	buf[ADDR_MASK_1] = head->addr & 0xff;
	buf[LEN_MASK_0] = (head->len >> 8) & 0xff;
	buf[LEN_MASK_1] = head->len & 0xff;
	return 0;
}

static int transfer_slave_package(int fd, struct sunxi_spi_slave_head *head,
		char *tx_buf, char *rx_buf) 
{
	uint8_t head_buf[PKT_HEAD_LEN + head->len];
	struct spi_ioc_transfer tr[2];
	int i;
	int ret;
	memset(tr, 0, sizeof(tr));
	memset(head_buf, 0x00, sizeof(head_buf));
	transfer_pkg_create(head_buf, head);
	if (tx_buf) {
		memcpy(&head_buf[PKT_HEAD_LEN], tx_buf, head->len);	
	} else if (rx_buf) {
		memcpy(&head_buf[PKT_HEAD_LEN], rx_buf, head->len);	
	}
	if (1) {
		printf("package head : { ");
		for (i = 0; i < PKT_HEAD_LEN; i++) {
			printf("0x%02x ", head_buf[i]);
		}
		printf("}\n");
	}
#if 1
	tr[0].tx_buf = (unsigned long)head_buf;
	tr[0].tx_nbits = 1;
	tr[0].rx_buf = (unsigned long)rx_buf;
	tr[0].rx_nbits = 1;
	tr[0].len = sizeof(head_buf);
	tr[0].speed_hz = speed;
	tr[0].bits_per_word = bits;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr[0]);
#endif
#if 0
	tr[0].tx_buf = (unsigned long)head_buf;
	tr[0].tx_nbits = 1;
	tr[0].rx_buf = (unsigned long)NULL;
	tr[0].rx_nbits = 0;
	tr[0].len = sizeof(head_buf);
	tr[0].speed_hz = speed;
	tr[0].bits_per_word = bits;
	tr[1].tx_buf = (unsigned long)tx_buf;
	tr[1].tx_nbits = tx_buf == NULL ? 0 : 1;
	tr[1].rx_buf = (unsigned long)rx_buf;
	tr[1].rx_nbits = rx_buf == NULL ? 0 : 1;
	tr[1].len = head->len;
	tr[1].speed_hz = speed;
	tr[1].bits_per_word = bits;

	ret = ioctl(fd, SPI_IOC_MESSAGE(2), tr);
#endif
	if (ret < 1)
		pabort("can't send spi message");
	return 0;
}

static int transfer_slave(int fd, uint32_t addr, uint32_t size) 
{
	struct sunxi_spi_slave_head pkt_head;
	char *tx_buf = NULL;
	char *rx_buf = NULL;
	int i, ret = -1;

#if 0
	tx_buf = malloc(size);
	for (i = 0; i < size; i++)
		tx_buf[i] = 0x50 + i;

	// Write forward
	pkt_head.op_code = SUNXI_OP_WRITE;
	pkt_head.addr = addr;
	pkt_head.len = size;
	ret = transfer_slave_package(fd, &pkt_head, tx_buf, NULL);
	hex_dump(tx_buf, size, 32, "TX");
	free(tx_buf);
	
	sleep(2);
#else
	// Read back
	rx_buf = malloc(size);
	memset(rx_buf, 0, size);
	pkt_head.op_code = SUNXI_OP_READ;
	pkt_head.addr = addr;
	pkt_head.len = size;
	transfer_slave_package(fd, &pkt_head, NULL, rx_buf);

	hex_dump(rx_buf, size, 32, "RX");

	free(rx_buf);
#endif
	return ret; 
}

static void print_usage(const char *prog) 
{
	printf("Usage: %s [-DsbdlHOLC3vpNR24SI]\n", prog);
	puts(" -D --device device to use (default /dev/spidev1.1)\n"
			" -s --speed max speed (Hz)\n"
			" -H --cpha clock phase\n"
			" -O --cpol clock polarity\n"
			" -L --lsb least significant bit first\n"
			" -C --cs-high chip select active high\n"
			" -R --ready slave pulls low to pause\n"
			" -v --verbose Verbose (show tx buffer)\n"
			" -a --addr operation address\n"
			" -S --size transfer size\n"
			" -I --iter iterations\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[]) 
{
	while (1) {
		static const struct option lopts[] = {
			{"device", 1, 0, 'D'},  {"addr", 1, 0, 'a'},  {"speed", 1, 0, 's'},
			{"cpha", 0, 0, 'H'},    {"cpol", 0, 0, 'O'},  {"lsb", 0, 0, 'L'},
			{"cs-high", 0, 0, 'C'}, {"ready", 0, 0, 'R'}, {"verbose", 0, 0, 'v'},
			{"size", 1, 0, 'S'},    {"iter", 1, 0, 'I'},  {NULL, 0, 0, 0},
		};
		int c;
		c = getopt_long(argc, argv, "D:a:s:o:HOLCRvS:I:", lopts, NULL);
		if (c == -1)
			break;
		switch (c) {
			case 'D':
				device = optarg;
				break;
			case 'a':
				addr = atoi(optarg);
				break;
			case 's':
				speed = atoi(optarg);
				break;
			case 'H':
				mode |= SPI_CPHA;
				break;
			case 'O':
				mode |= SPI_CPOL;
				break;
			case 'L':
				mode |= SPI_LSB_FIRST;
				break;
			case 'C':
				mode |= SPI_CS_HIGH;
				break;
			case 'R':
				//mode |= SPI_READY;
				ready = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'S':
				transfer_size = atoi(optarg);
				break;
			case 'I':
				iterations = atoi(optarg);
				break;
			default:
				print_usage(argv[0]);
		}
	}
}

int main(int argc, char *argv[]) 
{
	int fd;
	unsigned long nsec;
	unsigned long total_nsec = 0;
	unsigned long total_size = 0;
	int i;
	int ret = 0;
	parse_opts(argc, argv);
	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");
	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");
	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");
	printf("spi mode: 0x%x\n", mode);
	printf("max speed: %u Hz (%u kHz)\n", speed, speed / 1000);
	printf("op addr : %d\n", addr);
	printf("op size : %d\n", transfer_size);
	transfer_slave(fd, addr, transfer_size);
	close(fd);
	return ret;
}
