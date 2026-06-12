# 嵌入式单元测试项目

本目录包含多个嵌入式系统的单元测试项目，按接口类型分类存放在不同目录下：

```
unit_tests/
├── adc/              # ADC 采集测试
│   └── gpadc/
│       ├── ads1120/      # TI ADS1120 SPI ADC
│       └── allwin_adc/   # 全志 GPIO ADC
├── network/          # 网络通信测试
│   ├── tcp_server
│   ├── tcp_client
│   └── web_monse/    # 设备监控仪表盘
├── peripheral/       # 外设测试
│   ├── embedded_peripheral_test/
│   └── embedded_peripheral_test_separated/
├── serial/           # 串口通信测试
│   ├── serial_test
│   ├── serial_send
│   ├── serial_reader/
│   └── model_test/   # UART 回显测试
├── simulator/        # 数据模拟器
└── watchdog/        # 看门狗测试
```

---

## 1. serial/ - 串口通信测试

### 1.1 serial_test - 串口 Modbus 测试

基于 C 的串口 Modbus RTU 测试程序，定时发送 Modbus 命令并接收响应。

**编译器：** `arm-linux-gnueabi-g++`

**功能特性：**
- 打开并配置串口参数
- 发送 Modbus 功能码 0x03（读取保持寄存器）
- 定时发送和接收（默认 5 秒间隔）
- 带时间戳的日志输出

**使用方法：**
```bash
cd serial
make
./serial_test [串口设备]
# 例如：
./serial_test /dev/ttyS1
```

**默认配置：**
- 串口设备：/dev/ttyS1
- 波特率：9600

---

### 1.2 serial_send - 串口发送命令

基于 C++ 的串口命令发送程序，一次性发送指定命令。

**使用方法：**
```bash
./serial_send [串口设备]
```

**默认配置：**
- 串口设备：/dev/ttyS1
- 波特率：9600
- 发送命令：$01ADO\r\n

---

### 1.3 serial_reader - 串口数据读取

基于 C 的串口数据读取程序，实时读取并显示串口数据。

**使用方法：**
```bash
cd serial/serial_reader
make
./serial_read <串口设备>
```

**默认配置：**
- 波特率：115200

---

### 1.4 model_test - UART 回显测试

基于 C 的 UART 回显测试程序。

**使用方法：**
```bash
cd serial/model_test
make
./uart_echo
```

---

## 2. network/ - 网络通信测试

### 2.1 tcp_server - TCP 服务器

基于 C++ 的简单 TCP 服务器示例程序。

**功能特性：**
- 监听 8888 端口
- 接受客户端连接
- 发送欢迎消息和响应

**使用方法：**
```bash
cd network
make
./tcp_server
```

---

### 2.2 tcp_client - TCP 客户端

基于 C++ 的简单 TCP 客户端示例程序。

**使用方法：**
```bash
./tcp_client
```

**说明：**
- 默认连接地址：127.0.0.1:8888

---

### 2.3 web_monse - 设备监控仪表盘

基于 Mongoose 的嵌入式 Web 服务器，提供设备监控仪表盘。

**功能特性：**
- 实时 CPU 使用率监控
- 内存使用率监控
- 设备温度监控
- RESTful API 接口

**使用方法：**
```bash
cd network/web_monse
make
./device_dashboard
```

**访问地址：** http://localhost:38080

---

## 3. adc/ - ADC 采集测试

### 3.1 gpadc/ads1120 - ADS1120 SPI ADC 驱动

基于 C 的 TI ADS1120 SPI ADC 驱动程序。

**使用方法：**
```bash
cd adc/gpadc/ads1120
make
./ads1120
```

**硬件配置：**
- SPI 设备：/dev/spidev1.0
- SPI 速度：1 MHz
- 参考电压：2.048V

---

### 3.2 gpadc/allwin_adc - 全志 GPIO ADC 驱动

基于 C++ 的全志平台 GPIO ADC 驱动程序。

**使用方法：**
```bash
cd adc/gpadc/allwin_adc
make
./gpadc
```

---

## 4. peripheral/ - 外设测试

### 4.1 embedded_peripheral_test - 外设测试（整合版）

基于 Mongoose 的嵌入式外设测试系统。

**使用方法：**
```bash
cd peripheral/embedded_peripheral_test
make
./peripheral_test
```

---

### 4.2 embedded_peripheral_test_separated - 外设测试（前后端分离）

前后端分离架构的嵌入式外设测试系统。

**使用方法：**
```bash
cd peripheral/embedded_peripheral_test_separated/backend
make
./backend_server

# 浏览器访问
http://localhost:8080
```

---

## 5. watchdog/ - 看门狗测试

基于 C++ 的看门狗设备测试程序，测试系统看门狗功能。

**使用方法：**
```bash
cd watchdog
make

# 正常喂狗模式
./watchdog_test 1

# 测试超时模式（会触发系统重启）
./watchdog_test 2
```

**说明：**
- 默认设备：/dev/watchdog
- 喂狗间隔：5 秒
- 测试超时模式会触发系统重启，请谨慎使用

---

## 6. data_simulator/ - 数据模拟器

基于 C++ 的传感器数据模拟程序，按指定频率生成模拟数据并写入文件。

**使用方法：**
```bash
cd data_simulator
make
./data_simulator
```

**默认配置：**
- 输出文件：/run/media/mmcblk0p9/simulated_data.txt
- 写入频率：每秒 10 次

**输出数据格式：**
```
2024-01-01 12:00:00.123,-0.01,-0.07,-0.05,-12.5,277.0,0.07,00,CE,00
```

---

## 编译指南

每个子项目都可通过 Makefile 快速编译：

```bash
cd <项目目录>
make
./<可执行文件>
make clean
```

## 注意事项

1. 串口设备需要根据实际情况修改设备路径
2. 看门狗测试可能导致系统重启，请谨慎使用
3. 部分项目需要 root 权限运行
4. TCP 服务器/客户端需要配合使用