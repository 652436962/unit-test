#include <asm/types.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <condition_variable>
#include <atomic>


#include <awnn_lib.h>
#ifdef	USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif
#include "yolov3_postprocess.h"


#include "AWVideoEncoder.h"
#include "sunxiMemInterface.h"
#include "sdklog.h"
#include "Live555RTSPServer.hh"
#include "Log.hh"

#define		CLEAN(x)		(memset(&(x), 0, sizeof(x)))
#define		WIDTH			640
#define		HEIGHT			480
#define		ENC_WIDTH		WIDTH
#define		ENC_HEIGHT		HEIGHT
#define		IMAGE_WIDTH		640
#define		IMAGE_HEIGHT	384
#define		BUFFER_SIZE		4
#define		VIDEO_DEV		"/dev/video1"

#define		YOLOV3S_NBG				"/etc/4.0.0_Beta.nb"

struct det_object {
	int x;
	int y;
	int w;
	int h;
}det_object_t[5];

typedef struct video_buffer {
    void *start;
    unsigned int length;
} video_buffer;
static video_buffer *buffer = NULL;

paramStruct_t encoderIonMem;

static int rtsp_cli_sockfd;

Awnn_Context_t *context = NULL;

using namespace awvideoencoder;
using namespace std;

class MyEncoderCallback: public AWVideoEncoderDataCallback
{
	public:
		MyEncoderCallback(FILE* file, int sockfd, EncodeParam* param): mFile(file), mSocket(sockfd), mParam(param) {}
		virtual ~MyEncoderCallback() {}
		virtual int encoderDataReady(AVPacket* packet)
		{
			//Aprintf(" encdoe data id=0x%x,pts=0x%llx",packet->id, packet->pts);
			if ((CODEC_JPEG == mParam->codecType) &&
					(ONLY_ONE_FRAME == mParam->frameCount)) {

			} else {
				if (!h264headerflags) {
					memcpy(h264head, packet->pAddrVir0, packet->dataLen0);
					h264headerflags = true;
				}

				if ((!rtsp_index) && (get_rtsp_connect_status() > 0)) {
                    rtsp_start = true;
                    rtsp_index++;
                } else {
                    if (get_rtsp_connect_status() == 0) {
                        rtsp_start = false;
                        rtsp_index = 0;
                    }
                }

				if (rtsp_start) {
					if (rtsp_index == 1) {
						unsigned char *Ptr = (unsigned char *)packet->pAddrVir0;
						printf("========= ppt:%d\n", Ptr[4] & 0x1f);
						if ((Ptr[4] & 0x1f) == 5) {
							rtsp_first_flag = true;
						}

						if (rtsp_first_flag) {
							rtsp_index = 2;
							rtsp_first_flag = false;

							write(mSocket, h264head, sizeof(h264head));
							write(mSocket, packet->pAddrVir0, packet->dataLen0);
							if ((nullptr != packet->pAddrVir1) && (packet->dataLen1 > 0))
							{
								write(mSocket, packet->pAddrVir1, packet->dataLen1);
							}
						}
					} else {
						write(mSocket, packet->pAddrVir0, packet->dataLen0);
						if ((nullptr != packet->pAddrVir1) && (packet->dataLen1 > 0))
						{
							write(mSocket, packet->pAddrVir1, packet->dataLen1);
						}
					}
				}
			}
			return 0;
		}
	private:
		FILE		*mFile;
		int			mSocket;
		EncodeParam	*mParam;
		bool		h264headerflags = false;
		uint8_t		h264head[32];
        bool		rtsp_first_flag = false;
		int			rtsp_index  = 0;
		bool		rtsp_start = false;
};

void YUYV_2_NV21(unsigned char*src, unsigned char*dst, int width, int height)
{
    for (int i = 0; i < width * height * 2; i += 2) {
        *dst++ = *(src + i);
    }

    for (int y = 0; y < height - 1; y +=2) {
        for (int j = 0; j < width * 2; j += 4) {
            *dst++ = (*(src + 3 + j) + *(src + 3 + j + width * 2) + 1) >> 1;
            *dst++ = (*(src + 1 + j) + *(src + 1 + j + width * 2) + 1) >> 1;
        }
        src += width * 2 * 2;
    }
}

void YUYV_2_NV12(uint8_t *src, uint8_t *dst, uint8_t *modbuf, int width, int height, int modwidth, int modheight)
{
	uint8_t *nv12 = dst;
    for (int i = 0; i < width * height * 2; i += 2) {
        *dst++ = *(src + i);
    }

    for (int y = 0; y < height - 1; y +=2) {
        for (int j = 0; j < width * 2; j += 4) {
            *dst++ = (*(src + 1 + j) + *(src + 1 + j + width * 2) + 1) >> 1;
            *dst++ = (*(src + 3 + j) + *(src + 3 + j + width * 2) + 1) >> 1;
        }
        src += width * 2 * 2;
    }
	
	int offset = (height - modheight);
	memcpy(modbuf, nv12 + WIDTH * offset , modheight * WIDTH);
	memcpy(modbuf + modheight * WIDTH, nv12 + WIDTH * HEIGHT + (offset / 2) * WIDTH, (modheight / 2) * WIDTH);
}

void yuyv_to_rgb24(uint8_t* yuyv, uint8_t* rgb, int width, int height) 
{
	for (int i = 0; i < width * height; i += 2) {
		uint8_t y0 = yuyv[i * 2];
		uint8_t u = yuyv[i * 2 + 1];
		uint8_t y1 = yuyv[i * 2 + 2];
		uint8_t v = yuyv[i * 2 + 3];

		// 转换Y0为R, G, B
		rgb[i * 3] = (uint8_t)(1.164 * (y0 - 16) + 1.596 * (v - 128)); // R
		rgb[i * 3 + 1] = (uint8_t)(1.164 * (y0 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128)); // G
		rgb[i * 3 + 2] = (uint8_t)(1.164 * (y0 - 16) + 2.018 * (u - 128)); // B

		// 转换Y1为R, G, B
		rgb[i * 3 + 3] = (uint8_t)(1.164 * (y1 - 16) + 1.596 * (v - 128)); // R
		rgb[i * 3 + 4] = (uint8_t)(1.164 * (y1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128)); // G
		rgb[i * 3 + 5] = (uint8_t)(1.164 * (y1 - 16) + 2.018 * (u - 128)); // B
	}
}

//左上角（rect_x, rect_y），宽rect_w，高rect_h
void i420_rect_region(unsigned char* pFrm, int pic_w, int pic_h, \
		int rect_x, int rect_y, int rect_w, int rect_h, int R, int G, int B)	
{

    unsigned char* pLuma = (unsigned char*)pFrm;    //Y
    unsigned char* pChroma = (unsigned char*)pFrm + pic_w* pic_h;  //UV

    /* Set up the rectangle border size */
    const int border = 5;

    /* RGB convert YUV */
    int Y, U, V;
    Y = 0.299 * R + 0.587 * G + 0.114 * B;
    U = -0.1687 * R + 0.3313 * G + 0.5 * B + 128;
    V = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;

    /* Locking the scope of rectangle border range */
    for (int j = rect_y; j < rect_y + rect_h; j++)
    {
        for (int k = rect_x; k < rect_x + rect_w; k++)
        {
            if (k < (rect_x + border) || k >(rect_x + rect_w - border) ||
                j < (rect_y + border) || j >(rect_y + rect_h - border))
            {
                /* Components of YUV's storage address index */
				// YV12格式分布
				// Y Y Y Y Y Y Y Y
				// U U U U
				// V V V V
                int y_index = j * pic_w + k;
                int u_index = j / 2 * pic_w / 2 + k / 2;
                int v_index = u_index + (pic_w * pic_h) / 4;
                /* set up YUV's conponents value of rectangle border */
                pLuma[y_index] = Y;
                pChroma[u_index] = U;
                pChroma[v_index] = V;
            }
        }
    }
}

void nv12_rect_region(unsigned char* pFrm, int pic_w, int pic_h, \
		int rect_x, int rect_y, int rect_w, int rect_h, int R, int G, int B)
{

	unsigned char* pLuma = (unsigned char*)pFrm;    //Y
	unsigned char* pChroma = (unsigned char*)pFrm + pic_w* pic_h;  //UV

	/* Set up the rectangle border size */
	const int border = 5;

	/* RGB convert YUV */
	int Y, U, V;
	Y = 0.299 * R + 0.587 * G + 0.114 * B;
	U = -0.1687 * R + 0.3313 * G + 0.5 * B + 128;
	V = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;

	/* Locking the scope of rectangle border range */
	for (int j = rect_y; j < rect_y + rect_h; j++)
	{
		for (int k = rect_x; k < rect_x + rect_w; k++)
		{
			if (k < (rect_x + border) || k >(rect_x + rect_w - border) ||
					j < (rect_y + border) || j >(rect_y + rect_h - border))
			{
				/* Components of YUV's storage address index */
				// MV12格式分布
				// Y Y Y Y Y Y Y Y
				// U V U V
				int y_index = j * pic_w + k;
				int u_index = (j / 2) * pic_w + k;
				int v_index = u_index + 1;
				/* set up YUV's conponents value of rectangle border */
				pLuma[y_index] = Y;
				pChroma[u_index] = U;
				pChroma[v_index] = V;
			}
		}
	}
}

void v4l2_video_capture(const char *str)
{
	int ret = 0;
	int fd = 0;
	struct timeval tvptr;
	fd_set fdread;
    struct v4l2_capability cap;
    struct v4l2_format fmt;

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	struct v4l2_fmtdesc fmt_1;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;

	EncodeParam		encodeParam;
	struct timeval	now;
	AVPacket		packet;
	int				frameIndex			= 0;
	memset(&encodeParam, 0, sizeof(encodeParam));
	encodeParam.srcW = ENC_WIDTH;
    encodeParam.srcH = ENC_HEIGHT;
    encodeParam.dstW = ENC_WIDTH;
    encodeParam.dstH = ENC_HEIGHT;
    encodeParam.rotation = Angle_0;
    encodeParam.bitRate = 1 * 1024 * 1024;
    encodeParam.frameRate = 25;
    encodeParam.maxKeyFrame = 25;
    //encodeParam.codecType = CODEC_JPEG;
    encodeParam.codecType = CODEC_H264;
    encodeParam.pixelFormat = PIXEL_YUV420SP; //PIXEL_YVU420SP;
    encodeParam.frameCount = ONLY_ONE_FRAME;
    encodeParam.jpgQuality = 50;
    encodeParam.rcMode = VBR;
    encodeParam.minQp = 10;
    encodeParam.maxQp = 50;

	while (access(VIDEO_DEV, F_OK) != 0) {
		sleep(1);	
		continue;
	}

	fd = open(VIDEO_DEV, O_RDWR);
	if (fd < 0) {
        printf("can not open '%s'\n", VIDEO_DEV);
		return ;
	}
 
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("VIDIOC_QUERYCAP");
		close(fd);
		return ;
    }

    fmt_1.index = 0;
	fmt_1.type = type;
	while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt_1) >= 0) {
		struct v4l2_frmsizeenum frmsize;
		frmsize.pixel_format = fmt_1.pixelformat;
		frmsize.index = 0;
		while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0){
			std::cout << "width: " << frmsize.discrete.width << "\t height: " << frmsize.discrete.height << std::endl;
			frmsize.index++;
		}
		fmt_1.index++;
	}


    CLEAN(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        printf("VIDIOC_S_FMT IS ERROR! LINE:%d\n", __LINE__);
    }

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) {
        printf("VIDIOC_G_FMT IS ERROR! LINE:%d\n", __LINE__);
    }

    printf("width:%d\nheight:%d\npixelformat:%c%c%c%c\n",
           fmt.fmt.pix.width, fmt.fmt.pix.height,
           fmt.fmt.pix.pixelformat & 0xFF,
           (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 16) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 24) & 0xFF);


    CLEAN(req);
    req.count = BUFFER_SIZE;
    req.memory = V4L2_MEMORY_MMAP;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        printf("VIDIOC_REQBUFS IS ERROR! LINE:%d\n", __LINE__);
    }

    // 获取每个帧信息，并映射到用户空间
    buffer = (video_buffer *)calloc(req.count, sizeof(video_buffer));
    if (buffer == NULL) {
        printf("calloc is error! LINE:%d\n", __LINE__);
    }

    for (int buf_index = 0; buf_index < BUFFER_SIZE; buf_index++) {
        CLEAN(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.index = buf_index;
        buf.memory = V4L2_MEMORY_MMAP;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) // 获取每个帧缓冲区的信息 如length和offset
        {
            printf("VIDIOC_QUERYBUF IS ERROR! LINE:%d\n", __LINE__);
        }
        buffer[buf_index].length = buf.length;
        buffer[buf_index].start = mmap(NULL,
                                       buf.length,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED,
                                       fd,
                                       buf.m.offset);
        if (buffer[buf_index].start == MAP_FAILED) {
            printf("MAP_FAILED LINE:%d\n", __LINE__);
        }
        // 将帧缓冲区放入视频输入队列
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            printf("VIDIOC_QBUF IS ERROR! LINE:%d\n", __LINE__);
        }
        printf("Frame buffer :%d   address :0x%x    length:%d\n", \
				buf_index, reinterpret_cast<unsigned long>(buffer[buf_index].start), buffer[buf_index].length);
    }

	//FILE *fp = fopen("video.h264", "w+");

	struct sockaddr_un addr;
	addr.sun_family = PF_UNIX;
	strcpy(addr.sun_path, "/rtsp_srv.sock");
	rtsp_cli_sockfd  = socket(PF_UNIX, SOCK_STREAM, 0);
	do {
		usleep(1000 * 100);
		ret = connect(rtsp_cli_sockfd, (struct sockaddr *)&addr, sizeof(addr));
		if (ret == -1)
			perror("connect failed!");

		printf("=========== connect rtsp_srv.sock is success\n");
	}while (ret < 0);

	AWVideoEncoder* pEncoder = AWVideoEncoder::create();
	MyEncoderCallback* pCallback = new MyEncoderCallback(NULL, rtsp_cli_sockfd, &encodeParam);
	if (pEncoder->init(&encodeParam, pCallback) < 0) {
		ALOGE("v4l2_encoder failed\n");
		exit(EXIT_FAILURE);
	}

    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        printf("VIDIOC_STREAMON IS ERROR! LINE:%d\n", __LINE__);
        exit(1);
    }

	int		filterFramePeriod = 0;
	bool	find_obj = false;
	int		find_index = 0;
	int		obj_len = 0;

	int modelWidth = IMAGE_WIDTH;
	int modelHeight = IMAGE_HEIGHT;
	unsigned char *modelBuf = (unsigned char *) malloc((modelWidth * modelHeight * 3) >> 1);

	while (1)  {
		tvptr.tv_usec = 0;
		tvptr.tv_sec = 2;
		FD_ZERO(&fdread);
		FD_SET(fd, &fdread);
		if (select(fd + 1, &fdread, NULL, NULL, &tvptr) <= 0) {
			printf("==capture timeout! \n");
			continue;
		}

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
			printf("VIDIOC_DQBUF! err:%s\n", strerror(errno));
		}
		filterFramePeriod ++;

		uint8_t *i420_buffer = reinterpret_cast<uint8_t *>(encoderIonMem.vir);
		
		//YUYV_2_NV21(reinterpret_cast<uint8_t *>(buffer[buf.index].start), i420_buffer, WIDTH, HEIGHT);
		uint8_t *i420_y = i420_buffer;
        uint8_t *i420_u = i420_y + WIDTH * HEIGHT;
        uint8_t *i420_v = i420_u + WIDTH * HEIGHT / 4;
		YUYV_2_NV12(reinterpret_cast<uint8_t *>(buffer[buf.index].start), i420_buffer, modelBuf, \
				WIDTH, HEIGHT, modelWidth, modelHeight);
		
		flushCache(MEM_TYPE_CDX_NEW, &encoderIonMem, NULL);
		
		if (filterFramePeriod == 5) {
			int scaledWidth = 0;
			int scaledHeight = 0;
	
			unsigned char *input_buffers[2] = {modelBuf, modelBuf + modelWidth * modelHeight};

			printf("Process set buffers\n");
			awnn_set_input_buffers(context, input_buffers);
			// process network
			printf("Process run\n");
			awnn_run(context);
			// get result
			printf("Process get result\n");
			float **results = awnn_get_output_buffers(context);
			{
				Awnn_Result_t res;
				printf("yolov3 process\n");
				yolov3_postprocess(results, &res, IMAGE_WIDTH, IMAGE_HEIGHT, modelWidth, modelHeight, 0.35);
				obj_len = res.valid_cnt;
				obj_len = obj_len > 5 ? 5 : obj_len;
				for (int i = 0; i < obj_len; i++) { 
					printf("============ %d: cls %d, prob %f, rect [%d, %d, %d, %d]\n",
							i, res.boxes[i].label, res.boxes[i].score,
							res.boxes[i].xmin, res.boxes[i].ymin,
							res.boxes[i].xmax, res.boxes[i].ymax);
					det_object_t[i].x = res.boxes[i].xmin;
					det_object_t[i].y = res.boxes[i].ymin;
					det_object_t[i].w = res.boxes[i].xmax - det_object_t[i].x;
					det_object_t[i].h = res.boxes[i].ymax - det_object_t[i].y;
				} 

				if (obj_len > 0) {
					find_obj = true;

					for (int i = 0; i < obj_len; i++) {
						nv12_rect_region(modelBuf, IMAGE_WIDTH, IMAGE_HEIGHT, \
								det_object_t[i].x, det_object_t[i].y, det_object_t[i].w, det_object_t[i].h, \
								18, 255, 232);
					}
				} else {
					find_obj = false;
				}
			}
			filterFramePeriod = 0;
		}

		if (find_obj) {
			for (int i = 0; i < obj_len; i++) {
				nv12_rect_region(i420_y, WIDTH, HEIGHT, \
						det_object_t[i].x, det_object_t[i].y + (HEIGHT - IMAGE_HEIGHT), det_object_t[i].w, det_object_t[i].h, \
						18, 255, 232);
			}
		}

		flushCache(MEM_TYPE_CDX_NEW, &encoderIonMem, NULL);
		
		memset(&packet, 0, sizeof(AVPacket));
		packet.id = frameIndex + 0x100;

		gettimeofday(&now, NULL);
		packet.pts = now.tv_sec * 1000000 + now.tv_usec;

		packet.pAddrPhy0 = (unsigned char*)encoderIonMem.phy;
		packet.dataLen0 = ENC_WIDTH * ENC_HEIGHT;

		packet.pAddrPhy1 = (unsigned char*)encoderIonMem.phy + packet.dataLen0;
		packet.dataLen1 =  ENC_WIDTH * ENC_HEIGHT / 2;

		pEncoder->encode(&packet);

		if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
			printf("VIDIOC_QBUF buf.index %d failed\n", buf.index);
			exit(EXIT_FAILURE);
		}

	}

    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
        printf("VIDIOC_STREAMOFF IS ERROR! LINE:%d\n", __LINE__);
    }

    for (int i = 0; i < BUFFER_SIZE; i++) {
        munmap(buffer[i].start, buffer[i].length);
    }
    free(buffer);
	close(fd);
	free(modelBuf);
	
	AWVideoEncoder::destroy(pEncoder);

	return;
}
 
int main(int argc, char *argv[])
{
	cpu_set_t cpuset1, cpuset2;
	struct sockaddr_un addr;
	int rtsp_srv_sockfd;

	int ret = allocOpen(MEM_TYPE_CDX_NEW, &encoderIonMem, NULL);
	if (ret < 0) {
		printf("ion_alloc_open failed");
		return ret;
	}
	
	encoderIonMem.size = ENC_WIDTH * ENC_WIDTH * 3 / 2;
	allocAlloc(MEM_TYPE_CDX_NEW, &encoderIonMem, NULL);

	if (access("/rtsp_srv.sock", F_OK) == 0)
		unlink("/rtsp_srv.sock");

	rtsp_srv_sockfd  = socket(PF_UNIX, SOCK_STREAM, 0);
	if (rtsp_srv_sockfd < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// npu init
    awnn_init();
    context = awnn_create(YOLOV3S_NBG);

	addr.sun_family = PF_UNIX;
	strcpy(addr.sun_path, "/rtsp_srv.sock");
	int res = bind(rtsp_srv_sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (res == -1)
	{
		perror("bind failed!");
		exit(EXIT_FAILURE);
	}
	listen(rtsp_srv_sockfd, 2);
	
	CPU_ZERO(&cpuset1);
    CPU_SET(1, &cpuset1);

	std::thread v4l2_capture(v4l2_video_capture, "video_capture");
	pthread_setaffinity_np(v4l2_capture.native_handle(), sizeof(cpuset1), &cpuset1);

    create_rtsp_server(rtsp_srv_sockfd);

	// destroy network
	awnn_destroy(context);
	awnn_uninit();

    return 0;
}
