#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

// 串口设备路径（默认值）
char *SERIAL_PORT = "/dev/ttyS1";
// 波特率
#define BAUD_RATE 9600

// 打开并配置串口
int openSerialPort() {
    int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("打开串口失败");
        return -1;
    }
    
    struct termios options;
    if (tcgetattr(fd, &options) != 0) {
        perror("获取串口配置失败");
        close(fd);
        return -1;
    }
    
    // 配置串口
    cfmakeraw(&options);
    options.c_cflag &= ~CSIZE;
    
    // 设置波特率
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    
    // 设置数据位
    options.c_cflag |= CS8;
    
    // 设置奇偶校验（无）
    options.c_cflag &= ~PARENB;
    options.c_iflag &= ~INPCK;
    
    // 设置停止位（1位）
    options.c_cflag &= ~CSTOPB;
    
    // 设置超时
    options.c_cc[VTIME] = 1; // 1秒超时
    options.c_cc[VMIN] = 0;
    
    tcflush(fd, TCIFLUSH);
    
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("设置串口配置失败");
        close(fd);
        return -1;
    }
    
    return fd;
}

// 打印帮助信息
void printHelp(const char *progName) {
    printf("用法: %s [选项] [串口设备]\n", progName);
    printf("\n");
    printf("选项:\n");
    printf("  -h, --help    显示此帮助信息并退出\n");
    printf("\n");
    printf("参数:\n");
    printf("  串口设备      串口设备路径，默认为 /dev/ttyS1\n");
    printf("\n");
    printf("说明:\n");
    printf("  串口回显程序，持续监听串口数据，\n");
    printf("  收到数据后在开头添加 rk3562 并回复。\n");
    printf("\n");
    printf("配置:\n");
    printf("  波特率: %d\n", BAUD_RATE);
    printf("  数据位: 8\n");
    printf("  校验:   无\n");
    printf("  停止位: 1\n");
}

// 接收并在开头添加rk3562后回复
void replyWithPrefix(int fd) {
    char buffer[1024];
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("收到: %s", buffer);
        
        char reply[1024 + 10];
        snprintf(reply, sizeof(reply), "rk3562%s", buffer);
        
        ssize_t written = write(fd, reply, strlen(reply));
        if (written > 0) {
            printf("回复: %s", reply);
        } else {
            perror("回复失败");
        }
    }
}

int main(int argc, char *argv[]) {
    // 处理命令行参数
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            printHelp(argv[0]);
            return 0;
        }
        SERIAL_PORT = argv[1];
    }
    
    int fd = openSerialPort();
    if (fd < 0) {
        return 1;
    }
    
    printf("串口回显程序启动...\n");
    printf("串口设备: %s\n", SERIAL_PORT);
    printf("波特率: %d\n", BAUD_RATE);
    printf("====================================\n");
    
    while (1) {
        replyWithPrefix(fd);
    }
    
    // 关闭串口
    close(fd);
    printf("串口已关闭\n");
    
    return 0;
}
