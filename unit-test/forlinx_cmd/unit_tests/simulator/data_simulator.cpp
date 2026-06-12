#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <iomanip>

// 数据文件路径
const std::string FILE_PATH = "/run/media/mmcblk0p9/simulated_data.txt";
// 每秒写入次数
const int WRITE_PER_SECOND = 10;
// 写入间隔（微秒）
const int INTERVAL_US = 1000000 / WRITE_PER_SECOND;

// 生成随机数据
void generateData(std::string &data) {
    // 获取当前时间
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    // 构建时间字符串
    char time_str[30];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // 获取当前微秒
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    int ms = ts.tv_nsec / 1000000;
    
    // 构建完整的时间戳
    std::ostringstream oss;
    oss << std::string(time_str) << "." << std::setw(3) << std::setfill('0') << ms;
    std::string timestamp = oss.str();
    
    // 生成模拟数据（使用固定值模拟）
    std::string values = "-0.01,-0.07,-0.05,-12.5,277.0,0.07,00,CE,00";
    
    // 组合数据
    data = timestamp + "," + values;
}

int main() {
    // 打开文件
    std::ofstream outfile(FILE_PATH, std::ios::app);
    if (!outfile.is_open()) {
        std::cerr << "无法打开文件: " << FILE_PATH << std::endl;
        return 1;
    }
    
    std::cout << "数据模拟程序启动..." << std::endl;
    std::cout << "每秒写入 " << WRITE_PER_SECOND << " 次数据" << std::endl;
    std::cout << "数据文件: " << FILE_PATH << std::endl;
    std::cout << "按 Ctrl+C 停止程序" << std::endl;
    
    try {
        while (true) {
            // 生成数据
            std::string data;
            generateData(data);
            
            // 写入文件
            outfile << data << std::endl;
            
            // 打印到控制台
            std::cout << data << std::endl;
            
            // 等待指定间隔
            usleep(INTERVAL_US);
        }
    } catch (const std::exception &e) {
        std::cerr << "发生错误: " << e.what() << std::endl;
    }
    
    // 关闭文件
    outfile.close();
    
    return 0;
}
