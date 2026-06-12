#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

// 串口设备路径
char *SERIAL_PORT = "/dev/ttyS1";
// 波特率
#define BAUD_RATE 115200
// 发送间隔（秒）
#define SEND_INTERVAL 5

// Modbus命令
unsigned char modbusCommand[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A, 0xC5, 0xCD};
int commandLength = sizeof(modbusCommand);

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
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    
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

// 发送Modbus命令
void sendModbusCommand(int fd) {
    // 获取当前时间
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%H:%M:%S.%03d", tm_info);
    
    // 打印发送信息
    printf("[%s]发→", time_str);
    for (int i = 0; i < commandLength; i++) {
        printf("%02X ", modbusCommand[i]);
    }
    printf("\n");
    
    // 发送命令
    ssize_t written = write(fd, modbusCommand, commandLength);
    if (written != commandLength) {
        perror("发送命令失败");
    }
}

// 接收并打印响应
void receiveResponse(int fd) {
    unsigned char buffer[1024];
    ssize_t len;
    
    // 等待并读取响应
    usleep(100000); // 等待100ms
    
    len = read(fd, buffer, sizeof(buffer));
    if (len > 0) {
        // 获取当前时间
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%H:%M:%S.%03d", tm_info);
        
        // 打印响应信息
        printf("[%s]收←", time_str);
        for (ssize_t i = 0; i < len; i++) {
            printf("%02X ", buffer[i]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    // 处理命令行参数
    if (argc > 1) {
        SERIAL_PORT = argv[1];
    }
    
    int fd = openSerialPort();
    if (fd < 0) {
        return 1;
    }
    
    printf("串口测试程序启动，定时发送Modbus命令...\n");
    printf("串口设备: %s\n", SERIAL_PORT);
    printf("发送间隔: %d秒\n", SEND_INTERVAL);
    printf("发送命令: 05 03 00 00 00 12 C4 43\n");
    printf("====================================\n");
    
    while (1) {
        sendModbusCommand(fd);
        receiveResponse(fd);
        sleep(SEND_INTERVAL);
    }
    
    close(fd);
    return 0;
}
