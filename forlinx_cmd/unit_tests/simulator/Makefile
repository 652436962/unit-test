# Makefile for network test programs

# 交叉编译器路径
CXX = /home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-g++

# 编译选项
CXXFLAGS = -Wall -O2

# 目标文件
TARGETS = serial_test tcp_server tcp_client serial_send data_simulator watchdog_test

# 构建规则
all: $(TARGETS)

serial_test: serial_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
	/home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-strip $@

tcp_server: tcp_server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
	/home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-strip $@

tcp_client: tcp_client.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
	/home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-strip $@

serial_send: serial_send.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
	/home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-strip $@

data_simulator: data_simulator.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
	/home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-strip $@

watchdog_test: watchdog_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
	/home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-strip $@

# 清理规则
clean:
	rm -f $(TARGETS)

# 运行规则
run_serial:
	./serial_test

run_server:
	./tcp_server

run_client:
	./tcp_client

run_send:
	./serial_send

run_simulator:
	./data_simulator

run_watchdog:
	./watchdog_test
