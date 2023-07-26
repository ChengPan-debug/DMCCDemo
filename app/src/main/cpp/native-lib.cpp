#include <jni.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <thread>

#include "myrtspsenderv2.h"

#include <android/log.h>
#include <android/native_window_jni.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include "GLCoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "ipprotocol/ipprotocol.h"
#include "ipprotocol/IRTPSender.h"
#include "ipprotocol/IRTPReceiver.h"
#include "ipprotocol/VideoFECRTPSender.h"
#include "ipprotocol/VideoRTPSender.h"
#include "ipprotocol/VideoFECRTPReceiver.h"
#include "ipprotocol/AndroidVideoFECRTPReceiver.h"
#include "ipprotocol/AndroidVideoFECRTPReceiver_v2.h"
#include "ipprotocol/VideoRTPReceiver.h"
#include "ipprotocol/ipprotocoldeflog.h"
#include "rtspclient/rtspmodule.h"

#include "FRtpDecoder.h"
#include "myvideodecoder.h"

#include <mutex>
#include <list>

using namespace jrtplib;

using namespace std;
static IRTPSender *videoSender = NULL;
static IRTPReceiver *videoRecv = NULL;

static IpprotocolLog *m_log = new IpprotocolDefLog;

static RtspModule* g_rtspModule = nullptr;
static std::mutex g_rtsprecvmetex;

MyRtspSenderV2 *g_rtspSender = new MyRtspSenderV2;

extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_testSocket(JNIEnv *env, jobject obj) {
    LOGD("connect socket.");
    //	const char* response =  connectRemote("192.168.1.234", 1234); //response就是服务端传给客户端的内容
    //	LOGD("%s", response);
    //	release();
    LOGD("connect socket end.");
    jint res = -1;

    return res;
}

// (const char* ip, int meetingId, float isFec=0.8)
extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_createSender(JNIEnv *env, jobject obj,
                                                                         jstring ip,
                                                                         jint meetingId,
                                                                         jfloat isFec = 0.8) {
    LOGI("start createSender");
    jint res = -1;
    if (videoSender != NULL)
        return res;

    setenv("LOGNAME", "myAndroid", 0);

    if (isFec != 0)
        videoSender = new VideoFECRTPSender(meetingId, isFec);
    else
        videoSender = new VideoRTPSender(meetingId);

    const char *nativeIP = env->GetStringUTFChars(ip, JNI_FALSE);

    res = videoSender->SetRTPParams(nativeIP, meetingId, 0);

    //release
    env->ReleaseStringUTFChars(ip, nativeIP);

    // std::thread* th = new std::thread(&pollRun);

    if (res != 0) {
        return res;
    }

    videoSender->SetRTPParamsForSending();
    return res;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_sendRTP(JNIEnv *env, jobject obj,
                                                                    jbyteArray data,
                                                                    jint dataLength) {
    LOGI("start sendRTP");

    jint res = -2;
    if (videoSender == NULL)
        return res;

    char *m_h264Buf = NULL;

    m_h264Buf = (char *) (env->GetByteArrayElements(data, NULL));

    res = videoSender->SendRTP((unsigned char *) m_h264Buf, dataLength);

    //release
    env->ReleaseByteArrayElements(data, (jbyte *) m_h264Buf, 0);
    return res;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_releaseSender(JNIEnv *env,
                                                                          jobject obj) {

    LOGI("start releaseSender");

    jint res = 0;
    if (videoSender == NULL)
        return res;

    delete videoSender;
    videoSender = NULL;
    return res;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_createReceiver(JNIEnv *env, jobject obj,
                                                                           jstring ip,
                                                                           jint meetingId,
                                                                           jint isFec) {

    LOGI("start createReceiver");
    //	createRTPSession();
    jint res = -1;
    if (videoRecv != NULL)
        return res;

    setenv("LOGNAME", "myAndroid", 0);

    if (isFec > 0)
        videoRecv = new AndroidVideoFECRTPReceiverV2(meetingId);
    else
        videoRecv = new VideoRTPReceiver(meetingId);
    int m_nRecvPort = meetingId;

    const char *nativeIP = env->GetStringUTFChars(ip, JNI_FALSE);

    //Init_Single
    LOGD("recv %s, %d, %d", nativeIP, meetingId, isFec);
    res = videoRecv->Init_Single("192.168.10.150", 8000);

    env->ReleaseStringUTFChars(ip, nativeIP);

    if (res != 0) {
        LOGD("init errno %d", errno);
        return res;
    }

    return res;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_example_dmccdemo_TestActivity_recvPacket(JNIEnv *env, jobject obj) {
    LOGI("start recvPacket");

    if (videoRecv == NULL)
        return env->NewByteArray(0);

    PacketNode_t tempNode = videoRecv->GetPacketNode(1000);
    tempNode.Trim();
    //	tempNode.length = 4;
    //	tempNode.buf = (unsigned char*)new char[4];
    //	memcpy(tempNode.buf,"abcd",4);
    LOGD("recv Packeted %d", tempNode.length);
    jbyteArray data = env->NewByteArray(tempNode.length); //创建与buffer容量一样的byte[]
    env->SetByteArrayRegion(data, 0, tempNode.length,
                            (jbyte *) tempNode.buf); //数据拷贝到data中

    if (tempNode.length > 0)
        delete[] tempNode.buf;
    LOGD("recv end");
    return data;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_releaseRecv(JNIEnv *env, jobject obj) {
    LOGI("start releaseRecv");

    jint res = 0;
    if (videoRecv == NULL)
        return res;

    delete videoRecv;
    videoRecv = NULL;
    return res;
}

VideoDecoder *frdecoder = nullptr;
bool _run_videoDecode = false;
extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_InitffmpegRTP(JNIEnv *env,
                                                                          jobject obj) {
    LOGI("create ffmpeg video decoder.");

    if (frdecoder == nullptr) {
        frdecoder = VideoDecoder::createVideoDecoder();
        LOGD("frdecoder init");
    } else {
        LOGW("frdecoder already exist.");
        return -1;
    }
    _run_videoDecode = true;
    return 0;
}
ANativeWindow *nwin = nullptr;
AVPixelFormat g_format = AV_PIX_FMT_NONE;

extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_PlayffmpegRTPYUV(JNIEnv *env,
                                                                             jobject obj,
                                                                             jobject surface,
                                                                             jint screenWidth,
                                                                             jint screenHeight) {

    LOGI("init ffmpeg video decoder srcw:%d srch:%d", screenWidth, screenHeight);

    if (frdecoder == nullptr) {
        LOGE("PlayffmpegRTPYUV please run InitffmpegRTP first.");
        return -1;
    }
    int ret = frdecoder->initDecoder(screenWidth, screenHeight);
    if (ret < 0) {
        LOGD("InitDecoder error: %d", ret);
        return -1;
    }
    //1.获取原始窗口
    //be sure to use ANativeWindow_release()
    // * when done with it so that it doesn't leak.
    nwin = ANativeWindow_fromSurface(env, surface);
    //获取Display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOGD("egl display failed");
        return -1;
    }
    //2.初始化egl，后两个参数为主次版本号
    if (EGL_TRUE != eglInitialize(display, 0, 0)) {
        LOGD("eglInitialize failed");
        return -1;
    }

    //3.1 surface配置，可以理解为窗口
    EGLConfig eglConfig;
    EGLint configNum;
    EGLint configSpec[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };

    if (EGL_TRUE != eglChooseConfig(display, configSpec, &eglConfig, 1, &configNum)) {
        LOGD("eglChooseConfig failed");
        return -1;
    }

    //3.2创建surface(egl和NativeWindow进行关联。最后一个参数为属性信息，0表示默认版本)
    EGLSurface winSurface = eglCreateWindowSurface(display, eglConfig, nwin, 0);
    if (winSurface == EGL_NO_SURFACE) {
        LOGD("eglCreateWindowSurface failed");
        return -1;
    }

    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE
    };

    EGLContext context = eglCreateContext(display, eglConfig, EGL_NO_CONTEXT, ctxAttr);
    if (context == EGL_NO_CONTEXT) {
        LOGD("eglCreateContext failed");
        return -1;
    }

    if (EGL_TRUE != eglMakeCurrent(display, winSurface, winSurface, context)) {
        LOGD("eglMakeCurrent failed");
        return -1;
    }

    GLCoder glcoder(OUTPUT_VIEW_VERTEX_SHADER, YUV_FRAME_FRAGMENT_SHADER);
    glcoder.use();

    //加入三维顶点数据
    static float ver[] = {
            1.0f, -1.0f, 0.0f, // right bottom
            -1.0f, -1.0f, 0.0f, // left bottom
            1.0f, 1.0f, 0.0f, // right top
            -1.0f, 1.0f, 0.0f, // left top
    };
    GLuint apos = static_cast<GLuint>(glGetAttribLocation(glcoder.ID, "aPosition"));

    glEnableVertexAttribArray(apos);
    glVertexAttribPointer(apos, 3, GL_FLOAT, GL_FALSE, 0, ver);
    LOGD("run glVertexAttribPointer");

    //加入纹理坐标数据
    static float fragment[] = {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };
    GLuint aTex = static_cast<GLuint>(glGetAttribLocation(glcoder.ID, "aTextCoord"));
    glEnableVertexAttribArray(aTex);
    glVertexAttribPointer(aTex, 2, GL_FLOAT, GL_FALSE, 0, fragment);


    int width = frdecoder->getWidth();
    int height = frdecoder->getHeight();
    LOGD("frdecoder size:%d x %d", width, height);

    unsigned char *buf[3] = {0};

    int init = 0;
    GLuint texts[3] = {0};
    while (_run_videoDecode) {

        ret = frdecoder->decodeFrame();
        if (ret < 0) {
            LOGE("decode Frame error");
            //break;
            continue;
        }
//        else if (ret > 1) {
//            continue;
//        }

        if(init == 0 && !buf[0])
        {
            LOGD("begin init buffer");

            AVPixelFormat format = frdecoder->getPixelFormat();
            LOGI("get First AVPixelFormat:%d", format);
            if(format == -1)
                continue;
            g_format = format;

            buf[0] = new unsigned char[width * height];//y

            init = 1;
            glcoder.setInt("yTexture", 0);
            glcoder.setInt("uTexture", 1);
            glcoder.setInt("vTexture", 2);

            glGenTextures(3, texts);
            glBindTexture(GL_TEXTURE_2D, texts[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE,
                         GL_UNSIGNED_BYTE,
                         NULL);

            switch(format)
            {
                case AV_PIX_FMT_YUV420P:
                {
                    buf[1] = new unsigned char[width * height / 4];//u
                    buf[2] = new unsigned char[width * height / 4];//v

                    glBindTexture(GL_TEXTURE_2D, texts[1]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0, GL_LUMINANCE,
                                 GL_UNSIGNED_BYTE, NULL);

                    glBindTexture(GL_TEXTURE_2D, texts[2]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0, GL_LUMINANCE,
                                 GL_UNSIGNED_BYTE, NULL);
                }
                    break;
                case AV_PIX_FMT_YUVJ420P:
                {
                    buf[1] = new unsigned char[width * height / 4];//u
                    buf[2] = new unsigned char[width * height / 4];//v

                    glBindTexture(GL_TEXTURE_2D, texts[1]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0, GL_LUMINANCE,
                                 GL_UNSIGNED_BYTE, NULL);

                    glBindTexture(GL_TEXTURE_2D, texts[2]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width / 2, height / 2, 0, GL_LUMINANCE,
                                 GL_UNSIGNED_BYTE, NULL);
                }
                    break;
                case AV_PIX_FMT_YUVJ444P:
                {
                    buf[1] = new unsigned char[width * height];//u
                    buf[2] = new unsigned char[width * height];//v

                    glBindTexture(GL_TEXTURE_2D, texts[1]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE,
                                 GL_UNSIGNED_BYTE, NULL);

                    glBindTexture(GL_TEXTURE_2D, texts[2]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE,
                                 GL_UNSIGNED_BYTE, NULL);
                }
                    break;
            }
        }

        //读一帧yuv420p数据
        int ret = frdecoder->getYUVData(buf, width, height);
        if(ret < 0){
            continue;
        }

        //激活第一层纹理，绑定到创建的纹理
        //下面的width,height主要是显示尺寸？
        glActiveTexture(GL_TEXTURE0);
        //绑定y对应的纹理
        glBindTexture(GL_TEXTURE_2D, texts[0]);
        //替换纹理，比重新使用glTexImage2D性能高多
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0,//相对原来的纹理的offset
                        width, height,//加载的纹理宽度、高度。最好为2的次幂
                        GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        buf[0]);

        switch(g_format)
        {
            case AV_PIX_FMT_YUVJ420P:
            {
                //激活第二层纹理，绑定到创建的纹理
                glActiveTexture(GL_TEXTURE1);
                //绑定u对应的纹理
                glBindTexture(GL_TEXTURE_2D, texts[1]);
                //替换纹理，比重新使用glTexImage2D性能高
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                                GL_UNSIGNED_BYTE,
                                buf[1]);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, texts[2]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                                GL_UNSIGNED_BYTE,
                                buf[2]);
            }
                break;
            case AV_PIX_FMT_YUV420P:
            {
                //激活第二层纹理，绑定到创建的纹理
                glActiveTexture(GL_TEXTURE1);
                //绑定u对应的纹理
                glBindTexture(GL_TEXTURE_2D, texts[1]);
                //替换纹理，比重新使用glTexImage2D性能高
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                                GL_UNSIGNED_BYTE,
                                buf[1]);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, texts[2]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_LUMINANCE,
                                GL_UNSIGNED_BYTE,
                                buf[2]);
            }
                break;
            case AV_PIX_FMT_YUVJ444P:
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, texts[1]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE,
                                GL_UNSIGNED_BYTE,
                                buf[1]);

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, texts[2]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_LUMINANCE,
                                GL_UNSIGNED_BYTE,
                                buf[2]);
            }
                break;
        }

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        //窗口显示，交换双缓冲区
        eglSwapBuffers(display, winSurface);
    }

    delete[] buf[0];
    delete[] buf[1];
    delete[] buf[2];

    LOGD("delete[] buf ok;");

    _run_videoDecode = false;

    if (nwin) {
        ANativeWindow_release(nwin);
        nwin = nullptr;
        LOGD("Release ANativeWindow");
    }

    if (frdecoder) {
        frdecoder->releaseVideoDecoder();
        frdecoder = nullptr;
        LOGD("Release video decode");
    }

    LOGI("end video decoder.");
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_SetFFMPEGRTPData(JNIEnv *env,
                                                                             jobject obj,
                                                                             jbyteArray datas,
                                                                             jint dataLength) {

    if (frdecoder == nullptr) {
        LOGE("SetFFMPEGRTPData: please run InitffmpegRTP first.");
        return -1;
    }

    char *m_h264Buf = NULL;
    m_h264Buf = (char *) (env->GetByteArrayElements(datas, NULL));

    frdecoder->appendData(m_h264Buf, dataLength);
    env->ReleaseByteArrayElements(datas, (jbyte *)m_h264Buf, 0);

    return 0;
}


extern "C" JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_ReleaseFFMPEGRTP(JNIEnv *env,
                                                                             jobject obj) {

    LOGI("Release FFMPEG RTP");

    if (_run_videoDecode) {
        _run_videoDecode = false;
    } else {
        LOGW("Already stop FFMPEG RTP");
        return -1;
    }

    //防止线程没有退出，释放时崩溃
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    if (frdecoder) {
        //frdecoder->Stop();
        frdecoder->releaseVideoDecoder();
    }

    return 0;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_createRtspReceiver(JNIEnv *env,
                                                                               jclass clazz,
                                                                               jstring url) {
    // TODO: implement createRtspReceiver()
    std::lock_guard<std::mutex> locker(g_rtsprecvmetex);
    LOGI("create RtspReceiver");
    if(g_rtspModule == nullptr)
        g_rtspModule = new RtspModule();

    jint ret = 0;
    const char *nativeIP = env->GetStringUTFChars(url, JNI_FALSE);

    if(0 != g_rtspModule->openUrl(nativeIP, 1, 6000))
    {
        LOGI("failed to openURL, cannot connect to rtsp server\n");
        ret = -1;
    }

    env->ReleaseStringUTFChars(url, nativeIP);
    return ret;
}
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_dmccdemo_TestActivity_recvRtspAudioPacket(JNIEnv *env,
                                                                                jclass clazz,
                                                                                jint timeout) {
    // TODO: implement recvRtspAudioPacket()
    //LOGI("recv Rtsp Audio Packet");
    if(g_rtspModule == nullptr)
        return env->NewByteArray(0);

    DataNode tempNode = g_rtspModule->getAudioDataNode(timeout);

    jbyteArray data = env->NewByteArray(tempNode.len);
    env->SetByteArrayRegion(data, 0, tempNode.len,
                            (jbyte *) tempNode.buf); //数据拷贝到data中
    //LOGI("recv Rtsp Audio Packeted size: %d", tempNode.len);
    g_rtspModule->releaseNodeBuffer(tempNode.buf);
    return data;
}
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_dmccdemo_TestActivity_recvRtspVideoPacket(JNIEnv *env,
                                                                                jclass clazz,
                                                                                jint timeout) {
    // TODO: implement recvRtspVideoPacket()
    //LOGI("recv Rtsp Video Packet");
    if(g_rtspModule == nullptr)
        return env->NewByteArray(0);
    DataNode tempNode = g_rtspModule->getVideoDataNode(timeout);

    jbyteArray data = env->NewByteArray(tempNode.len);
    env->SetByteArrayRegion(data, 0, tempNode.len,
                            (jbyte *) tempNode.buf); //数据拷贝到data中

    //LOGI("recv Rtsp Video Packeted size: %d", tempNode.len);
    g_rtspModule->releaseNodeBuffer(tempNode.buf);
    return data;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dmccdemo_TestActivity_releaseRtspRecv(JNIEnv *env,
                                                                            jclass clazz) {
    // TODO: implement releaseRtspRecv()
    std::lock_guard<std::mutex> locker(g_rtsprecvmetex);
    LOGI("release Rtsp");
    if(g_rtspModule == nullptr)
        return;

    LOGI("begin release Rtspsender ");

    g_rtspModule->closeUrl();

    delete g_rtspModule;
    g_rtspModule = nullptr;

    LOGI("release Rtspsender ok");
}
extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_example_dmccdemo_TestActivity_recvRtspAudioAddHreadPacket(JNIEnv *env,
                                                                                        jclass clazz,
                                                                                        jint timeout) {
    // TODO: implement recvRtspAudioAddHreadPacket()
    //LOGI("release Rtsp");
    if(g_rtspModule == nullptr)
        return env->NewByteArray(0);

    DataNode tempNode = g_rtspModule->getAudioAddHreadNode(timeout);

    jbyteArray data = env->NewByteArray(tempNode.len);
    env->SetByteArrayRegion(data, 0, tempNode.len,
                            (jbyte *) tempNode.buf); //数据拷贝到data中
    //LOGI("recv Rtsp Audio Packeted size: %d", tempNode.len);
    g_rtspModule->releaseNodeBuffer(tempNode.buf);
    return data;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_example_dmccdemo_TestActivity_getRtspQueueSize(JNIEnv *env,
                                                                             jclass clazz,
                                                                             jint type) {
    // TODO: implement getRtspQueueSize()
    if(g_rtspModule == nullptr || type > 1)
        return 0;

    int size = g_rtspModule->getQueueSize((RTP_FRAME_TYPE)type);
    return size;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_dmccdemo_TestActivity_createRtspSender(JNIEnv *env,
                                                                             jclass clazz,
                                                                             jstring url,
                                                                             jint timeout,
                                                                             jint samplerate,
                                                                             jint channels,
                                                                             jboolean has_adts) {
    // TODO: implement createRtspSender()
    LOGI("create RtspSender");
    const char *nativeIP = env->GetStringUTFChars(url, JNI_FALSE);
    int urlLen = strlen(nativeIP);

    //LOGI("create RtspSender copy mem");

    char *tempUrl = new char[urlLen+1];
    memcpy(tempUrl, nativeIP, urlLen);
    tempUrl[urlLen]= 0;

    char *temp;
    std::list<char*> urls;

    temp = strtok(tempUrl, ",");
    while(temp)
    {
        LOGI("temp : %s",temp);
        int len = strlen(temp);
        char *tempBuffer = new char[len+1];
        memset(tempBuffer,0,len+1);
        memcpy(tempBuffer, temp, len);
        urls.push_back(tempBuffer);
        temp = strtok(NULL,",");
    }

    bool ret = false;

    LOGI("create RtspSender create");

    //调用创建函数
    ret = g_rtspSender->create(urls, timeout, samplerate, channels, has_adts);

    delete tempUrl;
    env->ReleaseStringUTFChars(url, nativeIP);
    std::list<char*>::iterator ite;
    for(ite = urls.begin(); ite != urls.end(); ite++)
    {
        delete (*ite);
        (*ite) = nullptr;
    }

    LOGI("create RtspSender create status %d", ret);

    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dmccdemo_TestActivity_rtspSenderClose(JNIEnv *env,
                                                                            jclass clazz) {
    // TODO: implement rtspSenderClose()
    LOGI("RtspSender rtspSenderClose begin");

    //调用关闭函数
    g_rtspSender->close();

    LOGI("RtspSender rtspSenderClose   end");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_dmccdemo_TestActivity_rtspSenderSetLog(JNIEnv *env,
                                                                             jclass clazz) {
    // TODO: implement rtspSenderSetLog()

}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_dmccdemo_TestActivity_rtspSenderPushFrameV2(JNIEnv *env,
                                                                                  jclass clazz,
                                                                                  jint type,
                                                                                  jbyteArray data,
                                                                                  jint data_length) {
    // TODO: implement rtspSenderPushFrameV2()
    //LOGI("rtspSenderPushFrameV2 .... begin ");

    if(data_length > 0)
    {
        jbyte* jbuffer = (env->GetByteArrayElements(data, NULL));
        char *buffer = (char *)jbuffer;

        //调用Push函数
        g_rtspSender->push(type, buffer, data_length);

        env->ReleaseByteArrayElements(data, jbuffer, NULL);
        return true;
    }
    else
    {
        LOGI("rtspSenderPushFrameV2 buf size < 0 ....");
    }

    return false;
}