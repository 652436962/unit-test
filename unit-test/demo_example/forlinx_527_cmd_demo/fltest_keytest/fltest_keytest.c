#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/select.h>
#define NOKEY 0

int main(int argc, const char *argv[])
{
	int keys_fd;
	struct input_event t;
	fd_set rset;
	int flag_key_test_pass = 0;	//the flag to exit when all keys have been triggered

	setvbuf(stdout, (char *)NULL, _IONBF, 0);//disable stdio out buffer;

	FD_ZERO(&rset);

	keys_fd = open("/dev/input/event0", O_RDONLY);
	if (keys_fd<=0)
	{
		printf("open %s device error!\n", "/dev/input/keypad");
		return 0;
	}
	FD_SET(keys_fd, &rset);


	while(1)
	{
		FD_ZERO(&rset);
		FD_SET(keys_fd, &rset);

		int num = select(keys_fd + 1, &rset, NULL, NULL, NULL);
		if (num <= 0)
			continue;
		if (read(keys_fd, &t, sizeof(t)) == sizeof(t)) {
			if(t.type == EV_KEY) {
				if(t.value == 0 || t.value == 1) {
					printf("key%d %s\n", t.code, (t.value) ? "Presse" : "Released");
				}
				switch(t.code)
				{
					case 113: flag_key_test_pass |= 0x1 << 0; break;	//userkey
					case 115: flag_key_test_pass |= 0x1 << 1; break;	//vol+
					case 114: flag_key_test_pass |= 0x1 << 2; break;	//vol-
					case 139: flag_key_test_pass |= 0x1 << 3; break;	//menu
					case  28: flag_key_test_pass |= 0x1 << 4; break;	//enter
					case 102: flag_key_test_pass |= 0x1 << 5; break;	//home
				}
			}
		}
		if((flag_key_test_pass & 0x3f) == 0x3f)  //0x3f = 0011 1111b
			break;
	}
	close(keys_fd);

	return 0;
}

