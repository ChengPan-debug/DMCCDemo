/**
 * \file ipprotocollog.h
 */

#ifndef IPPROTOCOLDEFLOG_H
#define IPPROTOCOLDEFLOG_H

#include "ipprotocollog.h"
// #include <log4cplus/logger.h>
// #include <log4cplus/configurator.h>
// #include <log4cplus/socketappender.h>
// #include <log4cplus/helpers/socket.h>
// #include <log4cplus/helpers/loglog.h>
// #include <log4cplus/helpers/stringhelper.h>
// #include <log4cplus/thread/threads.h>
// #include <log4cplus/spi/loggingevent.h>
// #include <log4cplus/thread/syncprims.h>
// #include <log4cplus/log4cplus.h>

#ifdef __ANDROID__
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG,__VA_ARGS__)
#endif //__ANDROID__

class IpprotocolDefLog : public IpprotocolLog
{
public:
	IpprotocolDefLog();
	void LogD(char const* const msg, ...) override;
	void LogI(char const* const msg, ...) override;
	void LogW(char const* const msg, ...) override;
	void LogE(char const* const msg, ...) override;
	void LogF(char const* const msg, ...) override;
};

#endif
