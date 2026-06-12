include ../makefile_cfg

all: quectelCM install

quectelCM:
	$(CC) -Wall -s QmiWwanCM.c GobiNetCM.c main.c MPQMUX.c QMIThread.c util.c qmap_bridge_mode.c mbim-cm.c device.c udhcpc.c -o quectelCM -lpthread -ldl
	@cp -f $@ $(INSTALL_PATH)
	@echo -e '\e[1;33m cp -f $@ $(INSTALL_PATH) \e[0m'

install::
	@echo -e '\e[1;33m cp -f quectelCM $(INSTALL_PATH) \e[0m'

clean:
	-rm -rf quectelCM *~
	-rm -rf quectel-qmi-proxy
