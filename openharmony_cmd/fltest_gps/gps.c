#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define DEFAULT_DEV     "/dev/ttyS1"
#define DEFAULT_BAUD    B9600
#define BUF_SIZE        512
#define NMEA_MAX_TOKENS 32

static int gps_fd = -1;

static void signal_handler(int sig)
{
	if (gps_fd >= 0)
		close(gps_fd);
	printf("\nGPS reader stopped.\n");
	exit(0);
}

static speed_t get_baud(int baud)
{
	switch (baud) {
	case 4800:   return B4800;
	case 9600:   return B9600;
	case 19200:  return B19200;
	case 38400:  return B38400;
	case 57600:  return B57600;
	case 115200: return B115200;
	default:     return B9600;
	}
}

static int uart_init(const char *dev, speed_t speed)
{
	struct termios newtio;

	gps_fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (gps_fd < 0) {
		perror("open");
		return -1;
	}

	fcntl(gps_fd, F_SETFL, 0);

	tcgetattr(gps_fd, &newtio);
	memset(&newtio, 0, sizeof(newtio));

	newtio.c_cflag = speed | CS8 | CLOCAL | CREAD;
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag &= ~PARENB;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 0;

	tcflush(gps_fd, TCIFLUSH);
	tcsetattr(gps_fd, TCSANOW, &newtio);

	return 0;
}

static int nmea_checksum(const char *sentence)
{
	int i, cs = 0;
	for (i = 1; sentence[i] && sentence[i] != '*'; i++)
		cs ^= sentence[i];
	return cs;
}

#define TOK(i) ((i) < count ? tokens[i] : "")

static void parse_gpgga(const char *tokens[], int count)
{
	printf("  [GGA] Fix Data\n");
	printf("    UTC Time:     %s\n", TOK(1));
	printf("    Latitude:     %s %s\n", TOK(2), TOK(3));
	printf("    Longitude:    %s %s\n", TOK(4), TOK(5));
	printf("    Fix Quality:  %s (%s)\n", TOK(6),
	       !strcmp(TOK(6),"0") ? "Invalid" :
	       !strcmp(TOK(6),"1") ? "GPS fix" :
	       !strcmp(TOK(6),"2") ? "DGPS fix" : TOK(6)[0] ? "Other" : "N/A");
	printf("    Satellites:   %s\n", TOK(7));
	printf("    Altitude:     %s m\n", TOK(9));
}

static void parse_gprmc(const char *tokens[], int count)
{
	printf("  [RMC] Recommended Minimum\n");
	printf("    UTC Time:     %s\n", TOK(1));
	printf("    Status:       %s (%s)\n", TOK(2),
	       !strcmp(TOK(2),"A") ? "Valid" : TOK(2)[0] ? "Invalid" : "N/A");
	printf("    Latitude:     %s %s\n", TOK(3), TOK(4));
	printf("    Longitude:    %s %s\n", TOK(5), TOK(6));
	printf("    Speed:        %s knots\n", TOK(7));
	printf("    Date:         %s\n", TOK(9));
}

static void parse_gpvtg(const char *tokens[], int count)
{
	printf("  [VTG] Track & Speed\n");
	printf("    True Track:   %s°\n", TOK(1));
	printf("    Speed:        %s km/h\n", TOK(7));
}

static void parse_gpgsa(const char *tokens[], int count)
{
	printf("  [GSA] Satellite Status\n");
	printf("    Mode:         %s (%s)\n", TOK(2),
	       !strcmp(TOK(2),"M") ? "Manual" : TOK(2)[0] ? "Auto" : "N/A");
	printf("    3D Fix:       %s (%s)\n", TOK(3),
	       !strcmp(TOK(3),"3") ? "3D" : !strcmp(TOK(3),"2") ? "2D" : TOK(3)[0] ? "None" : "N/A");
	printf("    PDOP:         %s\n", TOK(15));
	printf("    HDOP:         %s\n", TOK(16));
	printf("    VDOP:         %s\n", TOK(17));
}

static void parse_zda(const char *tokens[], int count)
{
	printf("  [ZDA] Time & Date\n");
	printf("    UTC Time:     %s\n", TOK(1));
	printf("    Day:          %s\n", TOK(2));
	printf("    Month:        %s\n", TOK(3));
	printf("    Year:         %s\n", TOK(4));
}

static void parse_gst(const char *tokens[], int count)
{
	printf("  [GST] Pseudorange Error Statistics\n");
	printf("    UTC Time:     %s\n", TOK(1));
	printf("    RMS Dev:      %s m\n", TOK(2));
	printf("    Semi-major:   %s m\n", TOK(3));
	printf("    Semi-minor:   %s m\n", TOK(4));
	printf("    Orientation:  %s°\n", TOK(5));
	printf("    Lat Error:    %s m\n", TOK(6));
	printf("    Lon Error:    %s m\n", TOK(7));
	printf("    Alt Error:    %s m\n", TOK(8));
}

/* Match NMEA sentence suffix (last 3 chars) regardless of talker ID */
static const char *nmea_suffix(const char *sentence)
{
	const char *p = strrchr(sentence, ',');
	/* Not tokenized yet, just look at raw $XXYYY */
	size_t len = sentence ? strlen(sentence) : 0;
	if (len >= 3)
		return sentence + len - 3;
	return NULL;
}

static void parse_nmea(const char *line)
{
	char buf[BUF_SIZE];
	const char *tokens[NMEA_MAX_TOKENS];
	int count = 0;
	char *p;

	strncpy(buf, line, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	/* Strip trailing \r\n */
	size_t len = strlen(buf);
	while (len > 0 && (buf[len-1] == '\r' || buf[len-1] == '\n'))
		buf[--len] = '\0';

	/* Always print raw NMEA line */
	printf(">>> %s\n", buf);

	/* Verify checksum */
	p = strchr(buf, '*');
	if (p) {
		int expected, actual;
		sscanf(p + 1, "%x", &expected);
		*p = '\0';
		actual = nmea_checksum(buf);
		*p = '*';
		if (expected != actual) {
			printf("  [WARN] Checksum mismatch (calc=0x%02X)\n", actual);
			return;
		}
	}

	/* Tokenize */
	strcpy(buf, line);
	p = strtok(buf, ",");
	while (p && count < NMEA_MAX_TOKENS) {
		char *nl = strchr(p, '\r');
		if (nl) *nl = '\0';
		nl = strchr(p, '\n');
		if (nl) *nl = '\0';
		nl = strchr(p, '*');
		if (nl) *nl = '\0';
		tokens[count++] = p;
		p = strtok(NULL, ",");
	}

	if (count < 1) return;

	/* Dispatch by sentence suffix (supports GP/GB/GL/GA/BD/GN talkers) */
	const char *suffix = nmea_suffix(tokens[0]);

	if (suffix && !strcmp(suffix, "GGA"))
		parse_gpgga(tokens, count);
	else if (suffix && !strcmp(suffix, "RMC"))
		parse_gprmc(tokens, count);
	else if (suffix && !strcmp(suffix, "VTG"))
		parse_gpvtg(tokens, count);
	else if (suffix && !strcmp(suffix, "GSA"))
		parse_gpgsa(tokens, count);
	else if (suffix && !strcmp(suffix, "ZDA"))
		parse_zda(tokens, count);
	else if (suffix && !strcmp(suffix, "GST"))
		parse_gst(tokens, count);
	else
		printf("  [%s]\n", tokens[0] + 1);
}

int main(int argc, char *argv[])
{
	const char *dev = DEFAULT_DEV;
	int baud = 9600;
	char buf[BUF_SIZE];
	char line[BUF_SIZE];
	int line_pos = 0;
	int opt;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	while ((opt = getopt(argc, argv, "d:b:h")) != -1) {
		switch (opt) {
		case 'd':
			dev = optarg;
			break;
		case 'b':
			baud = atoi(optarg);
			break;
		case 'h':
		default:
			printf("Usage: %s [-d <dev>] [-b <baud>] [-h]\n",
			       argv[0]);
			printf("  -d  Serial device (default: %s)\n",
			       DEFAULT_DEV);
			printf("  -b  Baud rate (default: 9600)\n");
			printf("  -h  Show this help\n");
			return 0;
		}
	}

	printf("GPS NMEA Reader\n");
	printf("Device: %s, Baud: %d\n", dev, baud);

	if (uart_init(dev, get_baud(baud)) < 0)
		return -1;

	printf("Waiting for NMEA data... (Ctrl+C to stop)\n\n");

	memset(line, 0, sizeof(line));
	line_pos = 0;

	while (1) {
		ssize_t n;
		char c;

		n = read(gps_fd, &c, 1);
		if (n <= 0) {
			usleep(10000);
			continue;
		}

		if (c == '\n') {
			line[line_pos] = '\0';
			if (line_pos > 0 && line[0] == '$')
				parse_nmea(line);
			memset(line, 0, sizeof(line));
			line_pos = 0;
		} else if (line_pos < (int)sizeof(line) - 1) {
			line[line_pos++] = c;
		} else {
			/* Line too long, reset */
			memset(line, 0, sizeof(line));
			line_pos = 0;
		}
	}

	close(gps_fd);
	return 0;
}
