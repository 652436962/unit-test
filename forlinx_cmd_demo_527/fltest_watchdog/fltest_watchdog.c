/*
 * Watchdog Driver Test Program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <getopt.h>

static int fd;
static int timeout = 10;
static int benable = 0;
static int bdisable = 0;
static int with_feed = 0;

static void print_usage(const char *prog)
{
	printf("Usage: %s [-t <timeout>] [-c] [-d/-e]\n", prog);
	printf( "  -t --timeout   set timeout (default 10), range ( 1 - 16)\n"
			"  -c --continue  enable watchdog with feed dogs\n"
			"  -d --disable   disable watchdog, conflict with enable\n"
			"  -e --enable    enable watchdog, conflict with disable\n"
			);
	fflush(stdout);
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	static const struct option lopts[] = {
		{ "timeout",  1, 0, 't' },
		{ "continue", 0, 0, 'c' },
		{ "disable",  0, 0, 'd' },
		{ "enable",  0, 0, 'e' },
		{ NULL, 0, 0, 0 },
	};
	int c = 0;
	int hasargs = 0;
	while (c != -1) {
		c = getopt_long(argc, argv, "t:cde", lopts, NULL);

		if (c != -1) {
			hasargs = 1;
		}

		switch (c) {
			case 't':
				timeout = atoi(optarg);
				break;
			case 'c':
				with_feed = 1;
				break;
			case 'd':
				bdisable = 1;
				break;
			case 'e':
				benable = 1;
				break;
			case -1:
				if (hasargs) {
					break;
				}
			default:
				print_usage(argv[0]);
				break;
		}
	}
}

/*
 * This function simply sends an IOCTL to the driver, which in turn ticks
 * the PC Watchdog card to reset its internal timer so it doesn't trigger
 * a computer reset.
 */
static void keep_alive(void)
{
	int dummy;

	ioctl(fd, WDIOC_KEEPALIVE, &dummy);
}

/*
 * The main program.  Run the program with "-d" to disable the card,
 * or "-e" to enable the card.
 */
int main(int argc, char *argv[])
{
	int flags;

	parse_opts(argc, argv);

	fd = open("/dev/watchdog", O_WRONLY);

	if (fd == -1) {
		fprintf(stderr, "Watchdog device not enabled.\n");
		fflush(stderr);
	exit(-1);
	}

	flags = timeout;
	ioctl(fd,WDIOC_SETTIMEOUT,&flags);

	if (benable | bdisable) {
		if (bdisable) {
			flags = WDIOS_DISABLECARD;
			ioctl(fd, WDIOC_SETOPTIONS, &flags);
			fprintf(stderr, "Watchdog disabled.\n");
			fflush(stderr);
			return 0;
		} else if (benable) {
			flags = WDIOS_ENABLECARD;
			ioctl(fd, WDIOC_SETOPTIONS, &flags);
			fprintf(stderr, "Watchdog enabled.\n");
			fprintf(stderr, "Restart after %d seconds.\n", timeout);
			fflush(stderr);
			return 0;
		}
	}
	else {
		fprintf(stderr, "Watchdog Ticking Away!\n");
		fflush(stderr);
	}

	while(with_feed) {
		keep_alive();
		sleep(1);
	}

	return 0;
}
