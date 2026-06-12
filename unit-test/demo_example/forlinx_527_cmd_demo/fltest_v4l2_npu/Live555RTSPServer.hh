#ifndef __LIVE555_RTSP_SERVER_H__
#define __LIVE555_RTSP_SERVER_H__

#include <queue>
#include <thread>
#include <mutex>

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

using namespace std;

void create_rtsp_server(int fd);
int get_rtsp_connect_status(void);
 
class H264LiveServerMediaSession : public OnDemandServerMediaSubsession
{
public:
    static H264LiveServerMediaSession *createNew(UsageEnvironment &env, Boolean reuseFirstSource);
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();
 
protected:
    H264LiveServerMediaSession(UsageEnvironment &env, Boolean reuseFirstSource);
    virtual ~H264LiveServerMediaSession(void);
    void setDoneFlag() { fDoneFlag = ~0; }
 
protected:
    virtual char const *getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource);
    virtual FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
    virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource *inputSource);
 
private:
    char *fAuxSDPLine;
    char fDoneFlag;
    RTPSink *fDummyRTPSink;
};
 
// 创建一个自定义的实时码流数据源类
class H264VideoStreamSource : public FramedSource
{
public:
    static H264VideoStreamSource *createNew(UsageEnvironment &env);
    unsigned maxFrameSize() const;
 
protected:
    H264VideoStreamSource(UsageEnvironment &env);
    virtual ~H264VideoStreamSource();

private:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();
};

#endif
