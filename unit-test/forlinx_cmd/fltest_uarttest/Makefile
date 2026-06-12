Target=fltest_uarttest

CC=aarch64-none-linux-gnu-gcc

OBJS=uarttest.o

CFLAGS+=-c -Wall -g
LDFLAGS+= -lpthread

$(Target):$(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@  

%.o:%.c                  
	$(CC) $^ $(CFLAGS) -o $@

.PHONY:clean cleanall

clean:
	@rm -f $(Target)
	@rm *.o

cleanall:clean
	@echo -e '\e[1;33m -rm -f $(INSTATLL_PATH)/$(Target) \e[0m'

distclean:cleanall

install:
	@echo -e '\e[1;33m  install -m 0755 $(Target) $(INSTATLL_PATH)/$(Target) \e[0m'


