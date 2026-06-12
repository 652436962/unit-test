#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for asprintf */
#endif
#include <stdio.h>
#include <stdint.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <linux/version.h>
#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME "event"
static int stop = 0;

static int is_event_device(const struct dirent *dir) {
	return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}

static char* scan_devices(void)
{
	struct dirent **namelist;
	int i, ndev, devnum;
	char *filename;
	int max_device = 0;

	ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, NULL);
	if (ndev <= 0)
		return NULL;

	fprintf(stderr, "Available devices:\n");

	for (i = 0; i < ndev; i++)
	{
		char fname[64];
		int fd = -1;
		char name[256] = "???";

		snprintf(fname, sizeof(fname),
			 "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);
		fd = open(fname, O_RDONLY);
		if (fd < 0)
			continue;
		ioctl(fd, EVIOCGNAME(sizeof(name)), name);
		close(fd);

		if (strncmp(name, "sunxi-gpadc", 11) != 0)
			continue;

		fprintf(stderr, "%s:	%s\n", fname, name);

		sscanf(namelist[i]->d_name, "event%d", &devnum);
		if (devnum > max_device)
			max_device = devnum;

		free(namelist[i]);
	}

	fprintf(stderr, "Select the device event number: ");
	scanf("%d", &devnum);

	if (devnum > max_device || devnum < 2)
		return NULL;

	asprintf(&filename, "%s/%s%d",
		 DEV_INPUT_EVENT, EVENT_DEV_NAME,
		 devnum);

	return filename;
}


static int print_events(int fd)
{
	struct input_event ev[64];
	int i, rd;
	fd_set rdfs;

	FD_ZERO(&rdfs);
	FD_SET(fd, &rdfs);

	while (!stop) {
		select(fd + 1, &rdfs, NULL, NULL, NULL);
		if (stop)
			break;
		rd = read(fd, ev, sizeof(ev));

		if (rd < (int) sizeof(struct input_event)) {
			printf("expected %d bytes, got %d\n", (int) sizeof(struct input_event), rd);
			perror("\nevtest: error reading");
			return 1;
		}

		for (i = 0; i < rd / sizeof(struct input_event); i++) {
			unsigned int type, code;

			type = ev[i].type;
			code = ev[i].code;

			if (type == EV_MSC) {
				printf("value %d\n", ev[i].value);
			}
		}
	}

	ioctl(fd, EVIOCGRAB, (void*)0);
	return EXIT_SUCCESS;
}

static void quit(int sig) 
{ 
	(void)sig; 
	stop = 1;
} 

int main() {
	char name[256];
	char cmd[256];
	int adc = 0;
	int index = 0;

	signal(SIGINT, quit); 

	char *event_name = scan_devices();
	if (!event_name)
		return -1;

	int fd = open(event_name, O_RDONLY);
	ioctl(fd, EVIOCGNAME(sizeof(name)), name);

	printf("%s\n", name);
	adc = atoi(name + 11);
	index = atoi(name + 20);

	sprintf(cmd, "echo gpadc%d,1 > /sys/class/gpadc/gpadc_chip%d/status", index, adc);
	system(cmd);
	print_events(fd);
	
	sprintf(cmd, "echo gpadc%d,0 > /sys/class/gpadc/gpadc_chip%d/status", index, adc);
	system(cmd);
	close(fd);
	printf("end\n");
	return 0;
}

