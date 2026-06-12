# 可执行文件名
TARGET = gpadc
ADC_TARGET = adc

# 源文件
SRCS = gpadc.cpp
ADC_SRCS = ADC.cpp

# 编译选项
CXX := /home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-g++
CXXFLAGS = -Wall -g

# 链接选项
LDFLAGS = 

# 默认目标
all: $(TARGET) $(ADC_TARGET)

# 编译规则
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(ADC_TARGET): $(ADC_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 清理目标
clean:
	rm -f $(TARGET) $(ADC_TARGET)

# 安装目标
install:
	cp $(ADC_TARGET) /mnt/c/Users/PC/Desktop/test/
	scp $(ADC_TARGET) root@192.168.10.100:/lechi

# 伪目标
.PHONY: all clean install