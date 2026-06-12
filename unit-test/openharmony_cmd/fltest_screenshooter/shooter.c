#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void sendevent(int fd, int type, int code, int value)
{
	
	struct input_event event;
	ssize_t ret;
	
	memset(&event, 0, sizeof(event));
	event.type = type;
	event.code = code;
	event.value = value;
	ret = write(fd, &event, sizeof(event));
	if(ret < (ssize_t) sizeof(event)) {
		fprintf(stderr, "write event failed, %s\n", strerror(errno));
	}
}

int main(int argc, char *argv[])
{
	int fd;
	int version;

	fd = open("/dev/input/virtkey", O_RDWR);
	if(fd < 0) {
		fprintf(stderr, "could not open %s\n", "/dev/input/virtkey");
		return 1;
	}
	if (ioctl(fd, EVIOCGVERSION, &version)) {
		fprintf(stderr, "could not get driver version for %s, %s\n", argv[optind], strerror(errno));
		return 1;
	}

	sendevent(fd, 1, 125, 1);	//win
	sendevent(fd, 1,  31, 1);	//S
	
	usleep(10000);
	
	sendevent(fd, 1,  31, 0);
	sendevent(fd, 1, 125, 0);
	sendevent(fd, 0,   0, 0);
	
	close(fd);
	
	return 0;
}
