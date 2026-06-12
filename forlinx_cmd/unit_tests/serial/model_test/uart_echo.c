#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

// 串口配置函数（9600波特率、8N1、无流控）
int uart_config(int fd, speed_t baudrate) {
    struct termios opt;
    // 获取当前串口配置
    if (tcgetattr(fd, &opt) < 0) {
        perror("tcgetattr failed");
        return -1;
    }

    // 清空输入输出模式
    cfmakeraw(&opt);

    // 设置波特率
    cfsetispeed(&opt, baudrate);
    cfsetospeed(&opt, baudrate);

    // 配置数据位（8位）、停止位（1位）、校验位（无）
    opt.c_cflag &= ~CSIZE;    // 清空数据位掩码
    opt.c_cflag |= CS8;       // 8位数据位
    opt.c_cflag &= ~PARENB;   // 无校验位
    opt.c_cflag &= ~CSTOPB;   // 1位停止位
    opt.c_cflag &= ~CRTSCTS;  // 禁用硬件流控
    opt.c_cflag |= CLOCAL | CREAD;  // 忽略调制解调器状态 | 启用接收

    // 关闭软件流控
    opt.c_iflag &= ~(IXON | IXOFF | IXANY);

    // 超时配置（select已处理超时，此处设为非阻塞读取）
    opt.c_cc[VTIME] = 0;  // 读取时无超时（依赖select超时）
    opt.c_cc[VMIN] = 1;   // 最少读取1个字节

    // 刷新串口缓冲区，应用配置
    tcflush(fd, TCIOFLUSH);
    if (tcsetattr(fd, TCSANOW, &opt) < 0) {
        perror("tcsetattr failed");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int uart_fd;
    fd_set read_fds;
    struct timeval timeout;
    char buf[128];  // 数据接收缓冲区
    int ret, n;

    // 检查参数（需传入串口设备名，如 /dev/ttyS0 或 /dev/ttyUSB0）
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <uart_device>\n", argv[0]);
        fprintf(stderr, "Example: %s /dev/ttyUSB0\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 打开串口（O_RDWR：读写 | O_NOCTTY：不成为控制终端 | O_NONBLOCK：非阻塞）
    uart_fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (uart_fd < 0) {
        perror("open uart failed");
        exit(EXIT_FAILURE);
    }

    // 配置串口为9600波特率
    if (uart_config(uart_fd, B9600) < 0) {
        close(uart_fd);
        exit(EXIT_FAILURE);
    }

    printf("UART echo start (9600 8N1), device: %s\n", argv[1]);
    printf("Press Ctrl+C to exit\n");

    while (1) {
        // 初始化select的文件描述符集
        FD_ZERO(&read_fds);
        FD_SET(uart_fd, &read_fds);

        // 设置select超时（5秒，可根据需求调整）
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // 等待串口可读（或超时）
        ret = select(uart_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (ret < 0) {
            // select出错（非EINTR中断）
            if (errno != EINTR) {
                perror("select failed");
                break;
            }
            continue;
        } else if (ret == 0) {
            // select超时（无数据），继续循环
            continue;
        }

        // 串口有数据可读
        if (FD_ISSET(uart_fd, &read_fds)) {
            // 读取串口数据
            n = read(uart_fd, buf, sizeof(buf) - 1);
            if (n < 0) {
                perror("read uart failed");
                continue;
            } else if (n == 0) {
                // 无数据（非阻塞模式下正常）
                continue;
            }

            // 打印收到的数据（十六进制+ASCII）
            buf[n] = '\0';
            printf("Received [%d bytes]: ", n);
            for (int i = 0; i < n; i++) {
                printf("%02X ", (unsigned char)buf[i]);
            }
            printf("| %s\n", buf);

            // 回显数据（收到什么发什么）
            n = write(uart_fd, buf, n);
            if (n < 0) {
                perror("write uart failed");
            } else {
                printf("Echo [%d bytes] success\n", n);
            }

            // 刷新串口输出缓冲区（确保数据立即发送）
            tcdrain(uart_fd);
        }
    }

    // 关闭串口
    close(uart_fd);
    printf("UART echo exit\n");
    return 0;
}