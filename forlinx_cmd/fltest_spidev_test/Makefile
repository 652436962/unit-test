Target = fltest_spidev_test

CPP=aarch64-none-linux-gnu-gcc

LIBS =

all: $(Target)

$(Target):spidev_test.c
	$(CPP) spidev_test.c -o $(Target) $(LIBS) 
	@echo "generate $(Target) success!!!"

	
.PHONY:clean cleanall

clean: 
	@rm -f $(Target)
cleanall:clean
	@echo -e '\e[1;33m -rm -f $(INSTATLL_PATH)/$(Target) \e[0m'

distclean:cleanall 
	
install:
	@echo -e '\e[1;33m  install -m 0755 $(Target) $(INSTATLL_PATH)/$(Target) \e[0m'

