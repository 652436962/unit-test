TARGET = fltest_backlight

include ../makefile_cfg

SDK_PATH = ../../sdk_lib

INC = -I./ \
	-I$(SDK_PATH)/include \
	-I$(SDK_PATH)/include/disp2 \
	-I$(SDK_PATH)/include/utils \
	-I$(SDK_PATH)/include/cutils/ \
	-I$(SDK_PATH)/include/media \
	-I$(SDK_PATH)/include/camera \
	-I$(SDK_PATH)/sdk_camera/moudle \
	-I$(SDK_PATH)/sdk_camera \
	-I$(SDK_PATH)/include/storage/ \
	-I$(SDK_PATH)/include/sound/ \
	-I$(SDK_PATH)/include/memory/ \
	-I$(SDK_PATH)/sdk_misc/ \
	-I$(SDK_PATH)/include/audioenc/ 

all: check install
	@echo "make finish!!!"

$(TARGET): $(TARGET).c
	$(CPP) -c $(TARGET).c -o $(TARGET).o -g $(INC) $(CEDAR_MACORS) $(DEFINES)
	$(CPP) -o $(TARGET) $(TARGET).o $(LIBS) $(CEDAR_LINK_NEW)
	
.PHONY: check clean cleanall distclean install

check:
	@echo -e $(TOOLTRAIN_ERR_STRING)

clean: 
	@rm -f $(TARGET)
cleanall:clean
	-rm -f $(INSTALL_PATH)/$(TARGET) 
distclean: cleanall 

install: $(TARGET)
	@cp $(TARGET) $(INSTALL_PATH)
	@echo -e '\e[1;33m cp -f $(TARGET) $(INSTALL_PATH) \e[0m'
