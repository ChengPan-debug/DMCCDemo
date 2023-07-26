/**
 * \file ipprotocollog.h
 */

#ifndef IPPROTOCOLLOG_H
#define IPPROTOCOLLOG_H

#include <stdio.h>
#include <string>
using namespace std;

#define LOG_BUF_SIZE 4096

class IpprotocolLog
{
public:
	virtual void LogD(char const* const msg, ...) = 0;
	virtual void LogI(char const* const msg, ...) = 0;
	virtual void LogW(char const* const msg, ...) = 0;
	virtual void LogE(char const* const msg, ...) = 0;
	virtual void LogF(char const* const msg, ...) = 0;
};

#ifdef __ANDROID__ 
#include <jni.h>
#include <android/log.h>

#define TAG "myjni_log" // 这个是自定义的LOG的标识
#endif

//#ifdef __ANDROID__ 
//#include <jni.h>
//#include <android/log.h>
//
//#define TAG "myjni_log" // 这个是自定义的LOG的标识
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
//#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
//#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
//#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型
//
//#endif
//
//#ifdef _CONSOLELOG_
//#define LOGD(format,...) printf(format"\n",##__VA_ARGS__)
//#define LOGI(format,...) printf(format"\n",##__VA_ARGS__)
//#define LOGW(format,...) printf(format"\n",##__VA_ARGS__)
//#define LOGE(format,...) printf(format"\n",##__VA_ARGS__)
//#define LOGF(format,...) printf(format"\n",##__VA_ARGS__)
//#endif
//
//#ifndef LOGD
//#define LOGD(format,...) 
//#endif
//#ifndef LOGI
//#define LOGI(format,...) 
//#endif
//#ifndef LOGW
//#define LOGW(format,...) 
//#endif
//#ifndef LOGE
//#define LOGE(format,...) 
//#endif
//#ifndef LOGF
//#define LOGF(format,...) 
//#endif

#endif
