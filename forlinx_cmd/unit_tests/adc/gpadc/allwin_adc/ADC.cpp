#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <unistd.h> 

// 定义gpadc节点的路径（固定路径）
const std::string GPADC_DATA_PATH = "/sys/class/gpadc/data";

/**
 * @brief 计算CH0通道电流值
 * @param raw_value CH0通道原始数据
 * @return float 计算后的电流值（单位：A）
 */
float calculate_ch0_current(int raw_value) {
    // 计算逻辑：raw_value / (4096 * 50 * 0.1) * 1.8 * 2
    return raw_value / (4096 * 50 * 0.1) * 1.8 * 2;
}

/**
 * @brief 计算CH1通道电压值
 * @param raw_value CH1通道原始数据
 * @return float 计算后的电压值（单位：V）
 */
float calculate_ch1_voltage(int raw_value) {
    // 计算逻辑：raw_value * (10+82.5) / 10 / 1000
    return raw_value * (10 + 82.5) / 10 / 1000;
}


/**
 * @brief 设置gpadc采集通道（对应echo 0 > data）
 * @param channel 采集通道号（此处传入0，可根据需求扩展）
 */
void setGpadcChannel(int channel) {

    std::ofstream out_file(GPADC_DATA_PATH, std::ios::out | std::ios::trunc);
    if (!out_file.is_open()) {
        throw std::runtime_error("无法打开gpadc data节点进行写入：" + GPADC_DATA_PATH);
    }

    // 将通道号写入文件（对应echo 0）
    out_file << channel;
    if (out_file.fail()) {
        throw std::runtime_error("写入gpadc通道号失败");
    }

    out_file.close();
}


/**
 * @brief 读取gpadc电压对应数据
 * @return int 读取到的原始数据
 */
int readGpadcValue() {
    // 以读取模式打开文件
    std::ifstream in_file(GPADC_DATA_PATH, std::ios::in);
    if (!in_file.is_open()) {
        throw std::runtime_error("无法打开gpadc data节点进行读取：" + GPADC_DATA_PATH);
    }

    // 读取数据（原始数据为整数格式，直接读取int类型）
    int gpadc_value = 0;
    in_file >> gpadc_value;
    if (in_file.fail()) {
        throw std::runtime_error("读取gpadc数据失败，数据格式异常");
    }

    in_file.close();
    return gpadc_value;
}

int main() {
    try {
        

        while (true) { 
        
            // 1. 电流设置采集通道为CH0
            std::cout << "正在设置gpadc采集通道为CH0..." << std::endl;
            setGpadcChannel(0);
            std::cout << "通道设置成功！" << std::endl;

            // 2. 读取当前电压对应数据
            std::cout << "正在读取gpadc CH0电压数据..." << std::endl;
            int voltage_data_ch0 = readGpadcValue();
            std::cout << "当前gpadc CH0原始数据：" << voltage_data_ch0 << std::endl;
            
            // 计算CH0通道电流值
            float calculated_current_ch0 = calculate_ch0_current(voltage_data_ch0);
            std::cout << "CH0计算后电流值：" << calculated_current_ch0 << " A" << std::endl;
            
            // 3. 设置采集通道为CH1
            std::cout << "正在设置gpadc采集通道为CH1..." << std::endl;
            setGpadcChannel(1);
            std::cout << "通道设置成功！" << std::endl;

            // 4. 读取通道1的电压对应数据
            std::cout << "正在读取gpadc CH1电压数据..." << std::endl;
            int voltage_data_ch1 = readGpadcValue();
            std::cout << "当前gpadc CH1原始数据：" << voltage_data_ch1 << std::endl;
            
            // 计算CH1通道电压值
            float calculated_voltage_ch1 = calculate_ch1_voltage(voltage_data_ch1);
            std::cout << "CH1计算后电压值：" << calculated_voltage_ch1 << " V" << std::endl;
            
            std::cout << std::endl; // 添加空行分隔不同采集次数的数据
            
            // 等待5秒，然后进行下一次采集
            std::cout << "等待5秒后进行下一次采集..." << std::endl;
            std::cout << std::endl;
            sleep(5); // 延迟5秒
        }
        
        // 注意：由于使用了无限循环，这里的代码不会被执行到
        std::cout << "ADC数据采集完成！" << std::endl;

    } catch (const std::exception& e) {
        // 捕获并打印异常信息
        std::cerr << "错误：" << e.what() << std::endl;
        return 1; // 异常退出，返回非0状态码
    }

    return 0; // 正常退出
}