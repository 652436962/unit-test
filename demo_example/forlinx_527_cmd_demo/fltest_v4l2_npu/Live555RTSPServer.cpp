#include <iostream>
#include <thread>
#include <mutex>

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

#include "Live555RTSPServer.hh"
#include "Log.hh"

using namespace std;

static int send_status = 0;
static int rtsp_srv_local_fb = 0;

int get_rtsp_connect_status(void)
{
	return send_status;
}

// H264LiveServerMediaSession 实现：
H264LiveServerMediaSession *H264LiveServerMediaSession::createNew(UsageEnvironment &env, Boolean reuseFirstSource)
{
	return new H264LiveServerMediaSession(env, reuseFirstSource);
}

H264LiveServerMediaSession::H264LiveServerMediaSession(UsageEnvironment &env, Boolean reuseFirstSource) : OnDemandServerMediaSubsession(env, reuseFirstSource)
{
	fAuxSDPLine = NULL;
	fDoneFlag = 0;
	fDummyRTPSink = NULL;
}

H264LiveServerMediaSession::~H264LiveServerMediaSession()
{
	delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void *clientData)
{
	H264LiveServerMediaSession *subsess = (H264LiveServerMediaSession *)clientData;
	subsess->afterPlayingDummy1();
}

void H264LiveServerMediaSession::afterPlayingDummy1()
{
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	setDoneFlag();
}

static void checkForAuxSDPLine(void *clientData)
{
	H264LiveServerMediaSession *subsess = (H264LiveServerMediaSession *)clientData;
	subsess->checkForAuxSDPLine1();
}

void H264LiveServerMediaSession::checkForAuxSDPLine1()
{
	nextTask() = NULL;

	char const *dasl;
	if (fAuxSDPLine != NULL)
	{
		setDoneFlag();
	}
	else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL)
	{
		fAuxSDPLine = strDup(dasl);
		fDummyRTPSink = NULL;
		setDoneFlag();
	}
	else if (!fDoneFlag)
	{
		// try again after a brief delay:
		int uSecsToDelay = 100000; // 100 ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
				(TaskFunc *)checkForAuxSDPLine, this);
	}
}

char const *H264LiveServerMediaSession::getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource)
{
	if (fAuxSDPLine != NULL)
	{
		return fAuxSDPLine;
	}

	if (fDummyRTPSink == NULL)
	{
		fDummyRTPSink = rtpSink;
		fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);
		checkForAuxSDPLine(this);
	}
	envir().taskScheduler().doEventLoop(&fDoneFlag);

	return fAuxSDPLine;
}

FramedSource *H264LiveServerMediaSession::createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate)
{
	estBitrate = 1024000; // kbps, estimate

	H264VideoStreamSource *videoSource = H264VideoStreamSource::createNew(envir());
	if (videoSource == NULL)
	{
		return NULL;
	}

	return H264VideoStreamFramer::createNew(envir(), videoSource);
}

RTPSink *H264LiveServerMediaSession ::createNewRTPSink(Groupsock *rtpGroupsock,
		unsigned char rtpPayloadTypeIfDynamic,
		FramedSource *inputSource)
{
	OutPacketBuffer::maxSize = 5 * 1024 * 1024; //521366;
	send_status = 1;
	return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

// H264VideoStreamSource 实现：
H264VideoStreamSource *H264VideoStreamSource::createNew(UsageEnvironment &env)
{
	return new H264VideoStreamSource(env);
}

H264VideoStreamSource::H264VideoStreamSource(UsageEnvironment &env) : FramedSource(env)
{
}

H264VideoStreamSource::~H264VideoStreamSource()
{
}

unsigned  int H264VideoStreamSource::maxFrameSize() const
{
	return (1572864); //150000; // 设置fMaxSize的值
}

void H264VideoStreamSource::doGetNextFrame()
{
	int read_bytes = 0;

	// 还没准备好要数据
	if (!isCurrentlyAwaitingData())
	{
		std::cout << "isCurrentlyAwaitingData" << std::endl;
		return;
	}

	gettimeofday(&fPresentationTime, NULL);

	read_bytes = read(rtsp_srv_local_fb, fTo, maxFrameSize() - 4);

	if (read_bytes <= 0) {
		fFrameSize = 0;
		//FramedSource::afterGetting(this);
		return ;
	}
	fFrameSize = read_bytes;
	
	FramedSource::afterGetting(this);
}

void H264VideoStreamSource::doStopGettingFrames()
{
	send_status = 0;
	std::cout << "doStopGettingFrames" << std::endl;
}

static RTSPServer *rtspServer;
static TaskScheduler *scheduler;
static UsageEnvironment *env;
ServerMediaSession *tmp = NULL;


void deleteServerMediaSession()
{
	rtspServer->removeServerMediaSession(tmp);
}

void addServerMediaSession()
{
	char const* descriptionString = "Session streamed by \"BHT507-H H264 RTSP\"";
	tmp = ServerMediaSession::createNew(*env, "main/sub/av_stream", "main/sub/av_stream", descriptionString);
	tmp->addSubsession(H264LiveServerMediaSession::createNew(*env, true));
	rtspServer->addServerMediaSession(tmp);
	char *url = rtspServer->rtspURL(tmp);
	dev_info("Play the stream using url %s\n", url);
	delete[] url;
}

void create_rtsp_server(int fd)
{
	char const* descriptionString = "Session streamed by \"BHT507-H H264 RTSP\"";

	rtsp_srv_local_fb = accept(fd, NULL, NULL);

	UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
	authDB = new UserAuthenticationDatabase;
	authDB->addUserRecord("username1", "password1"); // replace these with real strings
#endif

	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	rtspServer = RTSPServer::createNew(*env, 554);
	//rtspServer = RTSPServerSupportingHTTPStreaming::createNew(*env, 554, authDB);
	if (rtspServer == NULL)
	{
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		return;
	}

	ServerMediaSession *sms = ServerMediaSession::createNew(*env, "main/sub/av_stream", "main/sub/av_stream", descriptionString);
	sms->addSubsession(H264LiveServerMediaSession::createNew(*env, true));
	rtspServer->addServerMediaSession(sms);
	char *url = rtspServer->rtspURL(sms);
	dev_info("Play the stream using url %s\n", url);
	delete[] url;
	addServerMediaSession();
	env->taskScheduler().doEventLoop(); // 进入事件循环
}
