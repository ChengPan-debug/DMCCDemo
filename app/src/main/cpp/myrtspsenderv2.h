#ifndef MYRTSPSENDERV2_H
#define MYRTSPSENDERV2_H

#include <string.h>
#include <list>

#include "rtsppusher/rtsp_send_dll.h"

class RtspSenderLog : public PusherLog{
public:
    RtspSenderLog();
    virtual void LogDp(char const* const msg, ...) override;
    virtual void LogIp(char const* const msg, ...) override;
    virtual void LogWp(char const* const msg, ...) override;
    virtual void LogEp(char const* const msg, ...) override;
    virtual void LogFp(char const* const msg, ...) override;

    char *tag;
};

class MyRtspSenderV2
{
public:
    MyRtspSenderV2();
    bool create(std::list<char*>url, int timeout, int samplerate, int channels, bool has_adts);
    void close();
    bool push(int type, char *data, int len);

    volatile bool m_rtspIsRuning = false;
    std::list<Crtspsenddll *> m_rtspSenderList;
    std::list<RtspSenderLog *> m_rtspSenderLogList;
    RtspSenderLog m_log;
};

#endif //MYRTSPSENDERV2_H