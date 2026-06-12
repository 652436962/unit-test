#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <errno.h>

//--- ADS1120 配置 ---
const char *spi_device_path = "/dev/spidev1.0"; // 根据系统修改
uint8_t spi_mode = SPI_MODE_1;
uint32_t spi_speed_hz = 1000000; // 1 MHz
uint8_t spi_bits_per_word = 8;

// ADS1120 命令
#define CMD_RESET       0x06
#define CMD_START_SYNC  0x08
#define CMD_POWERDOWN   0x02
#define CMD_RDATA       0x10
#define CMD_RREG        0x20
#define CMD_WREG        0x40

// 寄存器地址
#define REG_CONFIG_0    0x00
#define REG_CONFIG_1    0x01
#define REG_CONFIG_2    0x02
#define REG_CONFIG_3    0x03

// 内部参考电压
#define VREF_INTERNAL 2.048

// 全局变量
int spi_fd = -1;

//--- SPI 初始化与控制函数 ---
int spi_init() {
    spi_fd = open(spi_device_path, O_RDWR);
    if (spi_fd < 0) {
        perror("无法打开SPI设备");
        return -1;
    }

    // 设置SPI模式
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode) == -1 ||
        ioctl(spi_fd, SPI_IOC_RD_MODE, &spi_mode) == -1) {
        perror("设置SPI模式失败");
        close(spi_fd);
        return -1;
    }

    // 设置每字位数
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits_per_word) == -1 ||
        ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bits_per_word) == -1) {
        perror("设置每字位数失败");
        close(spi_fd);
        return -1;
    }

    // 设置SPI速度
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed_hz) == -1 ||
        ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed_hz) == -1) {
        perror("设置SPI速度失败");
        close(spi_fd);
        return -1;
    }

    printf("SPI设备初始化成功: mode=%d, bits=%d, speed=%d Hz\n",
           spi_mode, spi_bits_per_word, spi_speed_hz);
    return 0;
}

void spi_close() {
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
        printf("SPI设备已关闭\n");
    }
}

int spi_transfer(uint8_t *tx, uint8_t *rx, size_t len) {
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .speed_hz = spi_speed_hz,
        .bits_per_word = spi_bits_per_word,
        .cs_change = 0,
    };

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
        perror("SPI传输失败");
        return -1;
    }
    return 0;
}

//--- ADS1120 控制函数 ---
int ads1120_send_command(uint8_t cmd) {
    printf("发送命令: 0x%02X\n", cmd);
    return spi_transfer(&cmd, NULL, 1);
}

int ads1120_write_register(uint8_t reg, uint8_t val) {
    uint8_t tx[3] = {CMD_WREG | (reg << 2), val};
    printf("写入寄存器 0x%02X: 0x%02X\n", reg, val);
    return spi_transfer(tx, NULL, 2);
}

int ads1120_read_register(uint8_t reg, uint8_t *val) {
    uint8_t tx[2] = {CMD_RREG | (reg << 2), 0x00};
    uint8_t rx[2] = {0};
    
    if (spi_transfer(tx, rx, 2) != 0) return -1;
    *val = rx[1];
    printf("读取寄存器 0x%02X: 0x%02X\n", reg, *val);
    return 0;
}

int ads1120_read_data(int16_t *raw) {
    uint8_t tx[3] = {CMD_RDATA, 0x00, 0x00};
    uint8_t rx[3] = {0};
    
    if (spi_transfer(tx, rx, 3) != 0) return -1;
    *raw = (int16_t)((rx[1] << 8) | rx[2]);
    return 0;
}

float calculate_voltage(int16_t raw, float vref, int gain) {
    return ((float)raw / 32768.0) * (vref / (float)gain);
}

//--- 主函数 ---
int main() {
    if (spi_init() != 0) return 1;

    // 复位设备
    ads1120_send_command(CMD_RESET);
    usleep(2000); // 等待复位完成

    // 配置为单端输入模式
    uint8_t config0_ain0 = 0b10000001; // AIN0单端输入，增益1，PGA禁用
    uint8_t config0_ain1 = 0b10010001; // AIN1单端输入，增益1，PGA禁用
    uint8_t config1 = 0b01001000;      // 连续模式，20SPS，内部参考

    // 写入配置寄存器
    ads1120_write_register(REG_CONFIG_0, config0_ain0);
    ads1120_write_register(REG_CONFIG_1, config1);

    // 启动转换
    ads1120_send_command(CMD_START_SYNC);
    usleep(50000); // 等待第一个转换周期

    // 读取AIN0数据
    printf("\n读取AIN0单端输入数据:\n");
    for(int i=0; i<5; i++) {
        int16_t raw;
        if(ads1120_read_data(&raw) == 0) {
            float voltage = calculate_voltage(raw, VREF_INTERNAL, 1);

            printf("IN0原始电压数据 %d: %.4f V ;", i+1, voltage);
            voltage = voltage/50;
            voltage = voltage*10;
            printf("IN0电流数据 %d: 0x%04X, %.4f A\n", i+1, raw, voltage);
        }
        usleep(50000); // 等待下一个采样周期
    }

    // 切换到AIN1通道
    ads1120_write_register(REG_CONFIG_0, config0_ain1);
    usleep(10000); // 等待配置生效

    // 读取AIN1数据
    printf("\n读取AIN1单端输入数据:\n");
    for(int i=0; i<5; i++) {
        int16_t raw;
        if(ads1120_read_data(&raw) == 0) {
            float voltage = calculate_voltage(raw, VREF_INTERNAL, 1);
            //计算电压
            printf("IN1原始电压数据 %d: %.4f V ;", i+1, voltage);
            voltage = voltage * 832.0 / 100.0;
            printf("IN1电压数据 %d: 0x%04X, %.4f V\n", i+1, raw, voltage);
        }
        usleep(50000);
    }

    // 停止转换
    ads1120_send_command(CMD_POWERDOWN);
    spi_close();
    return 0;
}