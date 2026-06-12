#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

int main(int argc, char *argv[]) {
    int fd;
    struct termios options;
    char buffer[256];
    ssize_t bytes_read;
    
    // 检查参数
    if (argc != 2) {
        fprintf(stderr, "用法: %s <串口设备>\n", argv[0]);
        fprintf(stderr, "例如: %s /dev/ttyS0\n", argv[0]);
        return 1;
    }
    
    // 打开串口设备
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("无法打开串口设备");
        return 1;
    }
    
    // 获取当前串口配置
    tcgetattr(fd, &options);
    
    // 设置波特率为 115200
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    
    // 设置数据位为 8 位
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    
    // 设置无校验位
    options.c_cflag &= ~PARENB;
    
    // 设置停止位为 1 位
    options.c_cflag &= ~CSTOPB;
    
    // 设置为原始模式
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    
    // 设置读取超时
    options.c_cc[VMIN] = 1;  // 至少读取 1 个字符
    options.c_cc[VTIME] = 0; // 不设置超时
    
    // 应用配置
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        perror("无法设置串口属性");
        close(fd);
        return 1;
    }
    
    printf("串口 %s 已打开，波特率 115200\n", argv[1]);
    printf("正在读取串口数据...\n");
    printf("按 Ctrl+C 退出\n\n");
    
    // 循环读取串口数据
    char line_buffer[256] = {0};
    size_t line_index = 0;
    
    while (1) {
        bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            for (int i = 0; i < bytes_read; i++) {
                if (buffer[i] == '\n') {
                    // 检测到换行符，打印完整的一行
                    line_buffer[line_index] = '\0';
                    printf("接收到: %s\n", line_buffer);
                    line_index = 0;
                    memset(line_buffer, 0, sizeof(line_buffer));
                } else {
                    // 存储字符到行缓冲区
                    if (line_index < sizeof(line_buffer) - 1) {
                        line_buffer[line_index++] = buffer[i];
                    }
                }
            }
        }
    }
    
    // 关闭串口
    close(fd);
    
    return 0;
}
