#include "myrtspsenderv2.h"
#include <android/log.h>

RtspSenderLog::RtspSenderLog()
{
    tag = new char[250];
    snprintf(tag, 250, "%s", "RtspSender");
}

void RtspSenderLog::LogDp(char const* const msg, ...)
{
    char buf[2048] = { 0 };
    va_list _ArgList;
    va_start(_ArgList, msg);
    vsnprintf(buf, sizeof(buf), msg, _ArgList);
    va_end(_ArgList);
    __android_log_print(ANDROID_LOG_DEBUG, tag, "%s", buf);
}
void RtspSenderLog::LogIp(char const* const msg, ...)
{
    char buf[2048] = { 0 };
    va_list _ArgList;
    va_start(_ArgList, msg);
    vsnprintf(buf, sizeof(buf), msg, _ArgList);
    va_end(_ArgList);
    __android_log_print(ANDROID_LOG_INFO, tag, "%s", buf);
}
void RtspSenderLog::LogWp(char const* const msg, ...)
{
    char buf[2048] = { 0 };
    va_list _ArgList;
    va_start(_ArgList, msg);
    vsnprintf(buf, sizeof(buf), msg, _ArgList);
    va_end(_ArgList);
    __android_log_print(ANDROID_LOG_WARN, tag, "%s", buf);
}
void RtspSenderLog::LogEp(char const* const msg, ...)
{
    char buf[2048] = { 0 };
    va_list _ArgList;
    va_start(_ArgList, msg);
    vsnprintf(buf, sizeof(buf), msg, _ArgList);
    va_end(_ArgList);
    __android_log_print(ANDROID_LOG_ERROR, tag, "%s", buf);
}
void RtspSenderLog::LogFp(char const* const msg, ...)
{
    char buf[2048] = { 0 };
    va_list _ArgList;
    va_start(_ArgList, msg);
    vsnprintf(buf, sizeof(buf), msg, _ArgList);
    va_end(_ArgList);
    __android_log_print(ANDROID_LOG_FATAL, tag, "%s", buf);
}

MyRtspSenderV2::MyRtspSenderV2()
{

}

bool MyRtspSenderV2::create(std::list<char*>urls, int timeout, int samplerate, int channels, bool has_adts)
{
    if(!m_rtspIsRuning && urls.size() > 0)
    {
        bool ret = true;
        int i = 1;
        std::list<char*>::iterator urlsIte = urls.begin();

        for(urlsIte; urlsIte != urls.end(); urlsIte++)
        {
            Crtspsenddll* rtspSender = new Crtspsenddll();
            ret = rtspSender->CreateNewRtspPusher((*urlsIte), timeout, samplerate, channels, has_adts);
            if(!ret)
            {
                //打印失败信息
                m_log.LogEp("CreateNewRtspPusher fial -- 2 url:%s",(*urlsIte));
                ret = 0;
                break;
            }

            m_log.LogIp("CreateNewRtspPusher success");
            m_rtspSenderList.push_back(rtspSender);

            RtspSenderLog *log = new RtspSenderLog;
            snprintf(log->tag, 250, "%s%d", "RtspSenderObject", i);
            m_rtspSenderLogList.push_back(log);
            rtspSender->setLog(log);

            i++;
        }

        if(!ret)
        {
            //创建失败了，关闭所有
            close();
            m_rtspIsRuning = false;
            return false;
        }

        m_rtspIsRuning = true;
        return true;
    }
    else
    {
        m_log.LogEp("rtsp sender runing");
    }

    return false;
}

void MyRtspSenderV2::close()
{
    std::list<Crtspsenddll*>::iterator ite;
    for(ite = m_rtspSenderList.begin(); ite != m_rtspSenderList.end(); ite++)
    {
        Crtspsenddll *tempRespSender = *ite;
        if(tempRespSender != nullptr)
        {
            tempRespSender->Close();
            delete tempRespSender;
            tempRespSender = nullptr;
        }
    }

    m_rtspSenderList.clear();


    std::list<RtspSenderLog*>::iterator logite;
    for(logite = m_rtspSenderLogList.begin(); logite != m_rtspSenderLogList.end(); logite++)
    {
        RtspSenderLog *tempLog = *logite;
        if(tempLog != nullptr)
        {
            delete tempLog;
            tempLog = nullptr;
        }
    }

    m_rtspSenderLogList.clear();

    m_rtspIsRuning = false;
}

bool MyRtspSenderV2::push(int type, char *data, int len)
{
    bool ret = false;
    if(m_rtspIsRuning)
    {
        std::list<Crtspsenddll*>::iterator ite;
        for(ite = m_rtspSenderList.begin(); ite != m_rtspSenderList.end(); ite++) {
            Crtspsenddll *tempRespSender = *ite;
            if(tempRespSender != nullptr)
                ret = tempRespSender->pushFrameV2((xop::MediaChannelId) type, data, len);
        }
    }
    return ret;
}