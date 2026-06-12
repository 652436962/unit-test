# Makefile for ads1120 program

# 编译器
CC := /home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-gcc

# 编译选项
CFLAGS = -Wall -O2

# 目标文件
TARGET = ads1120

# 源文件
SRCS = adc1120.c	

# 构建规则
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# 清理规则
clean:
	rm -f $(TARGET)

# 运行规则
run:
	./$(TARGET)
