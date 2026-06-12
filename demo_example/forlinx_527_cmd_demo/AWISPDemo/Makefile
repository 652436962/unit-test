TARGET := fltest_awIsp

include ../../makefile_cfg

COM_SOURCES:= \
	AWISPdemo.c


LOCAL_CFLAGS := \
	-I${LICHEE_TOP_DIR}/platform/forlinx/sdk_lib/include/isp/ \
	-I${LICHEE_TOP_DIR}/platform/forlinx/sdk_lib/include/isp/V4l2Camera \
	-I./


OBJS = $(patsubst %.c, %.c.o, $(COM_SOURCES))

LOCAL_LDFLAGS = -lisp -lisp_ini -lAWIspApi

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_LDFLAGS) -o $@ $^

$(OBJS):%.c.o:%.c
	$(CC) $(LOCAL_CFLAGS) $(CFLAGS) $(LDFLAGS) -c $< -o $@


all: install

install : $(TARGET)
	@cp $(TARGET) $(INSTALL_PATH)
	@echo -e '\e[1;33m cp -f $(TARGET) $(INSTALL_PATH) \e[0m'

clean :
	-rm -f $(TARGET) *.o

cleanall : clean
	-rm -f $(INSTALL_PATH)/$(TARGET)

