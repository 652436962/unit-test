CC = /persistent/home/lechi/tronlong/rk3562_sdk/ubuntu/rk3562-ubuntu20.04-sdk/rk3562-ubuntu20.04-sdk-v1.0/sysroots/x86_64-linux/bin/gcc-linaro-10.2.1-2021.01-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++
CFLAGS = -Wall -g
LDFLAGS =

TARGET = uart_echo
SRCS = uart_echo.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
