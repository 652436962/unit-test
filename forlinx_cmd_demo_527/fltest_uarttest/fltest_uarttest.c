#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <getopt.h>
#include <time.h>


void get_rand_str(char s[], size_t num)
{
    char *str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    size_t i, lstr;
    struct timespec seedp = { 0, 0 };
    lstr = strlen(str);
    clock_gettime(CLOCK_REALTIME, &seedp);

    srand(seedp.tv_sec + seedp.tv_nsec * 1000000000L);
    for(i = 0; i < num; i++) {
        s[i] = str[(rand() % lstr)];
    }
}

void print_usage(void)
{
    printf("Usage:\n");
    printf("  fltest_uarttest -d <device> [-w/-r] [-b <baudrate>] [-f] [-D <databit>]\n");
    printf("                  [-s <stopbit>] [-c <check>] [-n <num>] [-l <len>]\n\n");
    printf("  -d, --device <device>\n");
    printf("  -w/-r, --write/--read\n");
    printf("    if there is no '-w' or '-r', it is for looptest\n");
    printf("  -b, --baudrate <baudrate>, default is 115200bps\n");
    printf("  -f, --hard_flow default is off\n");
    printf("  -D, --databit default is 8\n");
    printf("  -s, --stopbit default is 1\n");
    printf("  -c, --check default is N\n");
    printf("  -n, --num <num>, the count of messages to read or write\n");
    printf("  -l, --len <len>, Bytes per message\n");
}

int get_delay(int databit, int stopbit, int speed, int size) 
{
	float delay = (1.0f / speed) * 1000000 * (1 + databit + stopbit) * size;
	return (int)(delay) + 1;
}

int main(int argc, char *const *argv)
{
    int fd;
    size_t nread;
    size_t nwrite;
    size_t n = 0;
    char* dev  = NULL;
    struct termios newtio;
    speed_t speed = B115200;
    int baudrate = 115200;
    int next_option, havearg = 0, flow = 0;
    int num = 1;
    int data_bit = 8;
    int stop_bit = 1;
    char check = 'N';
    size_t len = 32;
    const char *const short_opt = "fd:b:wrn:D:s:c:l:";
    const struct option long_opt[] = {
        {"devices", 1, NULL, 'd'},
        {"hard_flow", 0, NULL, 'f'},
        {"baudrate", 1, NULL, 'b'},
        {"write", 0, NULL, 'w'},
        {"read", 0, NULL, 'r'},
        {"num", 1, NULL, 'n'},
        {"stopbit", 1, NULL, 's'},
        {"databit", 1, NULL, 'D'},
        {"check", 1, NULL, 'c'},
        {"len", 1, NULL, 'l'},
        {NULL, 0, NULL, 0},
    };
    int direction = -1;  //-1 for loop test(shortcut tx and rx), 0 for reading and 1 for writing


    do{
        next_option = getopt_long(argc, argv, short_opt ,long_opt, NULL);
        switch(next_option){
            case 'd':
                dev = optarg;
                havearg = 1;
                break;
            case 'b':
                baudrate = atoi(optarg);
                switch (baudrate){
                    case 50:      speed = B50;      break;
                    case 75:      speed = B75;      break;
                    case 110:     speed = B110;     break;
                    case 134:     speed = B134;     break;
                    case 150:     speed = B150;     break;
                    case 200:     speed = B200;     break;
                    case 300:     speed = B300;     break;
                    case 600:     speed = B600;     break;
                    case 1200:    speed = B1200;    break;
                    case 1800:    speed = B1800;    break;
                    case 2400:    speed = B2400;    break;
                    case 4800:    speed = B4800;    break;
                    case 9600:    speed = B9600;    break;
                    case 19200:   speed = B19200;   break;
                    case 38400:   speed = B38400;   break;
                    case 57600:   speed = B57600;   break;
                    case 115200:  speed = B115200;  break;
                    case 230400:  speed = B230400;  break;
                    case 460800:  speed = B460800;  break;
                    case 500000:  speed = B500000;  break;
                    case 576000:  speed = B576000;  break;
                    case 921600:  speed = B921600;  break;
                    case 1000000: speed = B1000000; break;
                    case 1152000: speed = B1152000; break;
                    case 1500000: speed = B1500000; break;
                    case 2000000: speed = B2000000; break;
                    case 2500000: speed = B2500000; break;
                    case 3000000: speed = B3000000; break;
                    case 3500000: speed = B3500000; break;
                    case 4000000: speed = B4000000; break;
                    default: printf("ERROR: baudrate %d\n", baudrate); return -1;
                }
                break;
            case 'w':
                if(direction != -1)
                {
                    printf("ERROR: -w and -r can't be used at the same time\n");
                    return -1;
                }
                direction = 1;
                break;
            case 'r':
                if(direction != -1)
                {
                    printf("ERROR: -w and -r can't be used at the same time\n");
                    return -1;
                }
                direction = 0;
                break;
            case 'n':
                num = atoi(optarg);
                break;
            case 'f':
                flow = 1;
                break;
            case 'D':
                switch (atoi(optarg))
                {
                    case 5:
                    case 6:
                    case 7:
                    case 8: data_bit = atoi(optarg); break;
                    default:
                        printf("illegal date bit [%s]\n", optarg);
                        return -1;
                }
                break;
            case 's':
                switch (atoi(optarg))
                {
                    case 1:
                    case 2: stop_bit = atoi(optarg); break;
                    default:
                        printf("illegal stop bit [%s]\n", optarg);
                        return -1;
                }
                break;
            case 'c':
                switch (optarg[0])
                {
                    case 'N':
                    case 'O':
                    case 'E':
                    case 'M':
                    case 'S': check = optarg[0]; break;
                    default:
                        printf("illegal check [%s]\n", optarg);
                        return -1;
                }
                break;
            case 'l':
                len = atoi(optarg);
                break;
            case -1:
                if(havearg)
                break;
            default:
                print_usage();
                return -1;
        }
    }while(next_option != -1);


    if(dev == NULL)
    {
        print_usage();
        return -1;
    }

    fd = open(dev, O_RDWR);
    if (fd < 0)    {
        printf("Can't Open Serial Port!\n");
        return -1;
    }

    // printf("Welcome to uart test\n");

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = speed | CLOCAL | CREAD;
    newtio.c_cflag &= ~CSTOPB;

    switch (data_bit)
    {
    case 5:
        newtio.c_cflag |= CS5;
        break;
    case 6:
        newtio.c_cflag |= CS6;
        break;
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    default:
        newtio.c_cflag |= CS8;
        break;
    }
    
    if (stop_bit == 1)
    {
        newtio.c_cflag &= ~CSTOPB;
    }
    else if (stop_bit == 2)
    {
        newtio.c_cflag |= CSTOPB;
    }

    switch (check)
    {
    case 'O': //奇校验
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= INPCK;
        break;
    case 'E': //偶校验
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        newtio.c_iflag |= INPCK;
        break;
    case 'M': //MARK校验
        newtio.c_cflag |= PARODD;
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= CMSPAR;
        newtio.c_iflag &= ~INPCK;
        break;
    case 'S': //SPACE校验
        newtio.c_cflag &= ~PARODD;
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= CMSPAR;
        newtio.c_iflag &= ~INPCK;
        break;
    case 'N': //无校验
        newtio.c_cflag &= ~PARENB;
        break;
    default:
        newtio.c_cflag &= ~PARENB;
        break;
    }

    if (flow) {
        newtio.c_cflag |= CRTSCTS;
    } else {
        newtio.c_cflag &= ~CRTSCTS;
    }
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VTIME] = 0;

    tcsetattr(fd, TCSANOW, &newtio);
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    tcflush(fd, TCIOFLUSH);

    usleep(10000);

    char *buffer_tx = (char *)malloc(len + 1);
    char *buffer_rx = (char *)malloc(len + 1);
    memset(buffer_tx, 0, len + 1);
    memset(buffer_rx, 0, len + 1);
    buffer_tx[len] = 0;
    buffer_rx[len] = 0;

    if(direction == 1){
        while(num--)
        {
            memset(buffer_tx, 0x00, len);
            get_rand_str(buffer_tx, len);
            n = 0;
            do {
                nwrite = write(fd, &buffer_tx[n], len - n);
                n += nwrite;
                usleep(get_delay(data_bit, stop_bit, baudrate, len));
            }while (n != len);
            printf("tx_%d: %s\n", num, buffer_tx);

            usleep(1000);
        }
        close(fd);
    }else if(direction == 0){
        while(num--)
        {
            memset(buffer_rx, 0, len);
            n = 0;
            do {
                nread = read(fd, &buffer_rx[n], len - n);
                n += nread;
            }while (n != len);
            printf("rx_%d: %s\n", num, buffer_rx);

            usleep(1000);
        }
        close(fd);
    }else if(direction == -1){
        while(num--)
        {
            memset(buffer_tx, 0x00, len);
            get_rand_str(buffer_tx, len);
            write(fd, buffer_tx, len);
            printf("tx_%d: %s\n", num, buffer_tx);
            usleep(get_delay(data_bit, stop_bit, baudrate, len));

            memset(buffer_rx, 0, len);
            n = 0;
            do {
                nread = read(fd, &buffer_rx[n], len - n);
                n += nread;
            }while (n != len);
            printf("rx_%d: %s\n", num, buffer_rx);
            usleep(1000);
        }
    }
    free(buffer_tx);
    free(buffer_rx);
}
