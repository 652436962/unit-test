CXX = /persistent/home/lechi/tronlong/rk3562_sdk/ubuntu/rk3562-ubuntu20.04-sdk/rk3562-ubuntu20.04-sdk-v1.0/sysroots/x86_64-linux/bin/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-g++
CFLAGS = -Wall -g
LDFLAGS =

TARGETS = serial_test serial_send

all: $(TARGETS)

serial_test: serial_test.cpp
	$(CXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

serial_send: serial_send.cpp
	$(CXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
