Target = fltest_watchdog

CROSS_COMPILE ?= /media/lechi/linuxsdk/linuxsdk/openharmony_T527/forlinx-5.0-release/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
CC=$(CROSS_COMPILE)gcc

LIBS =

all: $(Target)

$(Target):watchdog.c
	$(CC) watchdog.c -o $(Target) $(LIBS) -static
	@echo "generate $(Target) success!!!"

	
.PHONY:clean cleanall

clean: 
	@rm -f $(Target)

cleanall:clean
	@echo -e '\e[1;33m -rm -f $(INSTATLL_PATH)/$(Target) \e[0m'

distclean:cleanall 
	
install:
	@echo -e '\e[1;33m  install -m 0755 $(Target) $(INSTATLL_PATH)/$(Target) \e[0m'

