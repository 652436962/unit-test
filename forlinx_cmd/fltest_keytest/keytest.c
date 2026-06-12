#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#define NOKEY 0

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

	ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, alphasort);
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
		if (strncmp(name, "adc-keys", strlen("adc-keys")) != 0)
			continue;

		fprintf(stderr, "%s:    %s\n", fname, name);

		sscanf(namelist[i]->d_name, "event%d", &devnum);
		if (devnum > max_device)
			max_device = devnum;

		free(namelist[i]);
	}

	if (devnum > max_device || devnum < 0)
		return NULL;

	asprintf(&filename, "%s/%s%d",
			DEV_INPUT_EVENT, EVENT_DEV_NAME,
			devnum);

	return filename;
}


int main(int argc, const char *argv[])
{
	int keys_fd;	
	struct input_event t;

	setvbuf(stdout, (char *)NULL, _IONBF, 0);//disable stdio out buffer;
	
	char *event_name = scan_devices();
    if (!event_name)
        return -1;

	keys_fd = open(event_name, O_RDONLY);
	if(keys_fd<=0)
	{
		printf("open %s device error!\n", event_name);
		return 0;
	}

	while(1)
	{	
		if(read(keys_fd,&t,sizeof(t))==sizeof(t)) {
			if(t.type==EV_KEY)
				if(t.value==0 || t.value==1)
				{
					printf("key%d %s\n",t.code, (t.value)?"Presse":"Released");

				}
		}
	}	
	close(keys_fd);

	return 0;
}

