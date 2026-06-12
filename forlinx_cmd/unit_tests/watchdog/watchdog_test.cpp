#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

// 看门狗设备路径
const std::string WATCHDOG_DEVICE = "/dev/watchdog";
// 测试模式：1=正常喂狗，2=测试超时
int test_mode = 1;

// 打开看门狗设备
int openWatchdog() {
    int fd = open(WATCHDOG_DEVICE.c_str(), O_WRONLY);
    if (fd < 0) {
        perror("打开看门狗设备失败");
        return -1;
    }
    std::cout << "成功打开看门狗设备: " << WATCHDOG_DEVICE << std::endl;
    return fd;
}

// 获取看门狗设备的当前超时时间
int getWatchdogTimeout(int fd) {
    int timeout;
    int ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
    if (ret < 0) {
        perror("获取看门狗超时时间失败");
        return 10000; // 默认值
    }
    
    std::cout << "看门狗当前超时时间: " << timeout << "秒" << std::endl;
    return timeout;
}

// 喂狗（重置看门狗计时器）
bool feedWatchdog(int fd) {
    int ret = write(fd, "", 1);
    if (ret != 1) {
        perror("喂狗失败");
        return false;
    }
    return true;
}

// 关闭看门狗设备
bool closeWatchdog(int fd) {
    // 关闭前发送魔术字符，告诉看门狗不要重启系统
    int magic = WDIOC_SETOPTIONS;
    int option = WDIOS_DISABLECARD;
    int ret = ioctl(fd, magic, &option);
    if (ret < 0) {
        perror("禁用看门狗失败");
        // 继续关闭文件描述符
    }
    
    ret = close(fd);
    if (ret < 0) {
        perror("关闭看门狗设备失败");
        return false;
    }
    
    std::cout << "成功关闭看门狗设备" << std::endl;
    return true;
}

int main(int argc, char *argv[]) {
    // 处理命令行参数
    if (argc > 1) {
        test_mode = atoi(argv[1]);
    }
    
    std::cout << "看门狗测试程序启动..." << std::endl;
    std::cout << "测试模式: " << (test_mode == 1 ? "正常喂狗" : "测试超时") << std::endl;
    
    // 打开看门狗设备
    int fd = openWatchdog();
    if (fd < 0) {
        return 1;
    }
    
    // 获取看门狗当前超时时间
    int actual_timeout = getWatchdogTimeout(fd);
    
    if (test_mode == 1) {
        // 正常喂狗模式
        std::cout << "开始定期喂狗，每5秒一次..." << std::endl;
        std::cout << "按 Ctrl+C 停止程序" << std::endl;
        std::cout << "注意: 硬件当前超时时间为" << actual_timeout << "秒，需要定期喂狗以避免系统重启" << std::endl;
        
        try {
            while (true) {
                if (feedWatchdog(fd)) {
                    std::cout << "喂狗成功!" << std::endl;
                }
                sleep(5);
            }
        } catch (const std::exception &e) {
            std::cerr << "发生错误: " << e.what() << std::endl;
        }
    } else {
        // 测试超时模式
        int wait_time = actual_timeout + 10; // 等待时间比超时时间长10秒
        std::cout << "开始测试看门狗超时，" << wait_time << "秒内不喂狗..." << std::endl;
        std::cout << "系统应该在" << actual_timeout << "秒后重启" << std::endl;
        std::cout << "注意: 硬件当前超时时间为" << actual_timeout << "秒，这是硬件设置" << std::endl;
        
        // 等待足够的时间，观察系统是否重启
        sleep(wait_time);
    }
    
    // 关闭看门狗设备
    closeWatchdog(fd);
    
    return 0;
}
