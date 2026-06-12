#CC = /home/ubuntu/tronlong/tina5.0_v1.0/out/t113_i/tlt113-evm-emmc/buildroot/buildroot/host/usr/bin/arm-linux-gnueabi-g++
CC = gcc
CFLAGS = -Wall -Wextra -O2 
LDFLAGS = -lpthread

TARGET = backend_server
SRC = main.c mongoose.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)


install: $(TARGET)
	@echo "拷贝可执行文件和web资源到目标目录..."
	@mkdir -p /opt/backend
	@cp $(TARGET) /opt/backend/
	@cp -r ../frontend /opt/backend/
	@echo "拷贝完成！"
