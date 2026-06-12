TARGET = fltest_v4l2_npu

CUR_MODULE_VERSION = "V2.1.20220920"

DEFINES += -DMODULE_VERSION=\"$(CUR_MODULE_VERSION)\"

include ./makefile_cfg

LDFLAGS  := -L$(SDK_LIBS) -L$(CEDAR_LIBS)
ifeq ("$(NPU_SW_VERSION)", "v2.0")
    LDFLAGS += -L${LICHEE_TOP_DIR}/platform/allwinner/vision/ai-sdk/viplite-tina/lib/aarch64-none-linux-gnu/v2.0/
else
    LDFLAGS += -L${LICHEE_TOP_DIR}/platform/allwinner/vision/ai-sdk/viplite-tina/lib/aarch64-none-linux-gnu/v1.13/
endif

LDFLAGS  += -L$(SDK_LIBS) -L$(CEDAR_LIBS)
CXXFLAGS += -g -O0 -pipe -Wall -W -Wl,--no-undefined -Wno-unused-parameter -Wno-unused-function  -Wformat=0 -fPIC
CXXFLAGS += -Ilive555/include

# 声明宏定义
DEFINES += -DUSE_LOGCAT -DLOG_LEVEL=6

# 声明头文件
DEFINC = -I./ \
		 -I$(SDK_PATH)/cedarx/include/libcedarc/include \
         -I$(LICHEE_KERN_DIR)/arch/arm64/include \
         -I$(SDK_PATH)/include/npu/awnn \
                 -I$(SDK_PATH)/include/npu/postprocess

ifeq ("$(NPU_SW_VERSION)", "v2.0")
DEFINC += -I${LICHEE_TOP_DIR}/platform/allwinner/vision/ai-sdk/viplite-tina/lib/aarch64-none-linux-gnu/v2.0/inc
else
DEFINC += -I${LICHEE_TOP_DIR}/platform/allwinner/vision/ai-sdk/viplite-tina/lib/aarch64-none-linux-gnu/v1.13/inc
endif
INCS += $(DEFINC) $(SDKLIB_INC)

LIBS += -lsdk_log -lsdk_awnn
ifeq ("$(NPU_SW_VERSION)", "v2.0")
LIBS += -lNBGlinker -lVIPhal
else
LIBS += -lVIPlite -lVIPuser
endif

INCS += $(DEFINC)

LIBS += -lMemAdapter -lvencoder -lVE -lcdc_base -lvenc_jpeg -lvenc_common -lvenc_h264 -lvenc_h265  -lvenc_base -lsdk_log \
		-lsdk_encoder -lsdk_memory -lpthread
LIBS += live555/lib/libliveMedia.a live555/lib/libgroupsock.a \
             live555/lib/libBasicUsageEnvironment.a live555/lib/libUsageEnvironment.a

#LIBS += -lopencv_highgui -lopencv_imgcodecs  -lopencv_imgproc -lopencv_core


####################################################################

SRC=fltest_v4l2_npu.cpp Live555RTSPServer.cpp

all: $(TARGET) install

$(TARGET):$(SRC)
	$(CPP) $^ -o $(TARGET) $(CXXFLAGS) $(LIBS) $(LDFLAGS) $(INCS) -fpermissive -Wno-missing-field-initializers


.PHONY: clean install

clean:
	rm $(TARGET)

install: $(TARGET)
	@cp $(TARGET) $(INSTALL_PATH)
	@echo -e '\e[1;33m cp -f $(TARGET) $(INSTALL_PATH) \e[0m'

