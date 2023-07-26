#pragma once

#if defined(WIN32) || defined(_WIN32)
#ifdef RTSPSENDDLL_EXPORTS
#define RTSPSENDDLL_API __declspec(dllexport)
#else
#define RTSPSENDDLL_API __declspec(dllimport)
#endif
#else
#define RTSPSENDDLL_API
#endif

#include <memory>
#include <string>

#include "RtspPusher.h"
#include "media.h"
#include "pusherdeflog.h"

// 此类是从 dll 导出的
class RTSPSENDDLL_API Crtspsenddll {
public:
	Crtspsenddll(void);
	bool CreateNewRtspPusher(const char* url,int timeout,uint32_t samplerate = 44100, uint32_t channels = 2, bool has_adts = true);
	bool IsConnect();
	bool PushFrame(xop::MediaChannelId channelId, xop::AVFrame frame);
	void Close();
	long GetH264Timestamp();
	long GetAACTimestamp(uint32_t samplerate = 44100);
	void setLog(PusherLog *log);

	bool pushFrameV2(xop::MediaChannelId channelId, const char* buffer, int size);
private:
	std::unique_ptr<xop::EventLoop> event_loop_;
	std::shared_ptr<xop::RtspPusher> rtsp_pusher;
	bool m_isSetLog;
};

//extern RTSPSENDDLL_API int nrtspsenddll;
//
//RTSPSENDDLL_API int fnrtspsenddll(void);
