#CC = gcc
CC = /home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-gcc
CFLAGS = -Wall -Wextra -O2

TARGET = serial_read
SRC = serial_read.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

run:
	./$(TARGET) /dev/ttyS0
