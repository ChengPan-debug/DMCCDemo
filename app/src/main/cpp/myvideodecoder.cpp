#include "myvideodecoder.h"

#include <jni.h>
#include <android/log.h>

#define TAG "video_software_decoder"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG,__VA_ARGS__)

#ifndef  PIXFMT
#define PIXFMT AV_PIX_FMT_YUVJ444P
#endif

VideoDecoder *VideoDecoder::_decoder = NULL;

VideoDecoder::VideoDecoder()
{
    m_init = false;
    m_firstDecode = true;
}

VideoDecoder::~VideoDecoder()
{
    
}

VideoDecoder * VideoDecoder::createVideoDecoder()
{
    if(VideoDecoder::_decoder == nullptr)
        VideoDecoder::_decoder = new VideoDecoder();
    return VideoDecoder::_decoder;
}

int VideoDecoder::initDecoder(int width, int height)
{
    if (!VideoDecoder::_decoder) {
        LOGE("use CreateDecoder first.");
        return -1;
    }

    AVCodec* pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!pCodec)
	{
		LOGE("Codec not found");
		return -2;
	}

    _ctx = avcodec_alloc_context3(pCodec);
	_ctx->codec_id = AV_CODEC_ID_H264;
	_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	_ctx->pix_fmt = AV_PIX_FMT_YUVJ444P;
	_ctx->width = width;
	_ctx->height = height;
	_ctx->time_base = av_make_q(1, 1);
	_ctx->framerate = av_make_q(1, 1);
	_ctx->bit_rate = 400000;
	_ctx->gop_size = 25;
	_ctx->qmin = 10;
	_ctx->qmax = 30;
	_ctx->max_b_frames = 0;

    // Open codec
	if (avcodec_open2(_ctx, pCodec, NULL) < 0)
	{
		LOGE("Could not open codec");
		return -3;
	}

    //_packet = av_packet_alloc();
    _rframe = av_frame_alloc();

    m_init = true;
    m_firstDecode = true;

    return 0;
}

void VideoDecoder::releaseVideoDecoder()
{
    mu.lock();

    m_init = false;
    m_firstDecode = true;

    while(m_packetList.size() > 0)
    {
        AVPacket* packet = m_packetList.front();
        m_packetList.pop_front();
        delete[] packet->data;
        packet->data = nullptr;
        av_packet_unref(packet);
        av_packet_free(&packet);
    }

    if (_rframe) {
        av_frame_free(&_rframe);
        _rframe = NULL;
        LOGD("av_frame_free");
    }
    if (_packet) {
        av_packet_free(&_packet);
        _packet = NULL;
        LOGD("av_packet_free ");
    }
    if (_ctx) {
        avcodec_free_context(&_ctx);
        _ctx = NULL;
        LOGD("avcodec_free_context ");
    }

    mu.unlock();

    if (VideoDecoder::_decoder) {
        delete VideoDecoder::_decoder;
        VideoDecoder::_decoder = NULL;
    }
}

int VideoDecoder::getWidth()
{
    return _ctx->width;
}

int VideoDecoder::getHeight()
{
    return _ctx->height;
}

//用该数据生成一个AVPacket，放到 m_packetList
void VideoDecoder::appendData(char *data, int size)
{
    if(!m_init)
        return;
//    LOGI("data: %x %x %x %x %x %x",
//         (unsigned char)data[0],(unsigned char)data[1],
//         (unsigned char)data[2],(unsigned char)data[3],
//         (uint8_t)data[4],(uint8_t)data[5]);
   // LOGI("appendData size: %d", size);

    std::unique_lock<std::mutex> lck(mu);

    if(m_packetList.size() > 10)
    {
        if(data[4] != 0x00 && data[5] != 0x00 &&
        data[6] != 0x00 && data[7] != 0x01)
        {
            return;
        }
    }

    if(data[4] == 0x00 && data[5] == 0x00 && data[6] == 0x00 &&
        data[7] == 0x01 && data[8] == 0x67 && m_packetList.size() > 20){

        LOGI("~ m_packetList size: %d", m_packetList.size());
        while (m_packetList.size()) {
            AVPacket* packetTemp = m_packetList.front();
            LOGI("delete packetTemp size: %d", packetTemp->size);
            m_packetList.pop_front();
            delete[] packetTemp->data;
            packetTemp->data = nullptr;
            av_packet_unref(packetTemp);
            av_packet_free(&packetTemp);


        }

        LOGI("~ m_packetList remainder size: %d", m_packetList.size());
    }

    LOGI("m_packetList remainder size: %d", m_packetList.size());


    AVPacket* ipacket = av_packet_alloc();
    ipacket->data = new uint8_t[size+1];
    memcpy(ipacket->data, data, size);
    ipacket->size = size;

    m_packetList.push_back(ipacket);
}

//从packetList中取数据，解码放到 _rframe
int VideoDecoder::decodeFrame()
{
    if(!m_init)
        return -1;

    if(readNextPacket() != 0)
        return 2;

    int fret = 0;

    if(avcodec_send_packet(_ctx, _packet) >= 0) {
        while((fret = avcodec_receive_frame(_ctx, _rframe)) >= 0) {
            if (fret == AVERROR(EAGAIN) || fret == AVERROR_EOF) {
                break;
            }
            else if (fret < 0) {
                LOGE("Error during decoding!");
                break;
            }

            if(m_firstDecode)
            {
                m_pixelFormat = (AVPixelFormat)_rframe->format;
                if(m_pixelFormat != -1)
                    m_firstDecode = false;
                LOGD("First AVPixelFormat :%d",m_pixelFormat);
            }

            fret == 0;

            break;
        }
    }

    delete _packet->data;
    _packet->data = nullptr;

    av_packet_unref(_packet);
    av_packet_free(&_packet);

    _packet = NULL;

    return fret;
}

//_rframe转换成YUV，放入dst中
int VideoDecoder::getYUVData(uint8_t ** dst, int &width, int &height)
{
    if(!m_init)
        return -1;

    int ret = 0;
    int ly = _rframe->linesize[0];
    int lu = _rframe->linesize[1];
    int lv = _rframe->linesize[2];

    if(ly == 0 || lu == 0 || lv == 0)
        return -2;

    //LOGD("frame data:%d, %d, %d,width:%d", ly, lu, lv, width);

    switch (m_pixelFormat)
    {
    case AV_PIX_FMT_YUVJ444P:
    {
        memcpy(dst[0], (const void *) (_rframe->data[0]), width * height);
        memcpy(dst[1], (const void *) (_rframe->data[1]), width * height);
        memcpy(dst[2], (const void *) (_rframe->data[2]), width * height);
    }
        break;
    case AV_PIX_FMT_YUVJ420P:
    {
        if (ly == width) {
            memcpy(dst[0], (const void *) (_rframe->data[0]), ly * height);
            memcpy(dst[1], (const void *) (_rframe->data[1]), lu * height / 2);
            memcpy(dst[2], (const void *) (_rframe->data[2]), lv * height / 2);
        }
        else{
            for (int i = 0; i < height; ++i) {
                memcpy(dst[0]+i*width, (const void *) (_rframe->data[0]+i*ly), width);
            }
            for (int i = 0; i < height/2; ++i) {
                memcpy(dst[1]+i*(width/2), (const void *) (_rframe->data[1]+i*lu), width/2);
                memcpy(dst[2]+i*(width/2), (const void *) (_rframe->data[2]+i*lv), width/2);
            }
        }
    }
        break;
    case AV_PIX_FMT_YUV420P:
    {
        if (ly == width) {
            memcpy(dst[0], (const void *) (_rframe->data[0]), ly * height);
            memcpy(dst[1], (const void *) (_rframe->data[1]), lu * height / 2);
            memcpy(dst[2], (const void *) (_rframe->data[2]), lv * height / 2);
        }
        else{
            for (int i = 0; i < height; ++i) {
                memcpy(dst[0]+i*width, (const void *) (_rframe->data[0]+i*ly), width);
            }
            for (int i = 0; i < height/2; ++i) {
                memcpy(dst[1]+i*(width/2), (const void *) (_rframe->data[1]+i*lu), width/2);
                memcpy(dst[2]+i*(width/2), (const void *) (_rframe->data[2]+i*lv), width/2);
            }
        }
    }
        break;
    case AV_PIX_FMT_NV12:
        break;
    }

    return 1;
}

//获取像素格式
AVPixelFormat VideoDecoder::getPixelFormat()
{
    return m_pixelFormat;
}

int VideoDecoder::readNextPacket() {
    std::unique_lock<std::mutex> lck(mu);
    if (m_packetList.size() > 0) {
        _packet = m_packetList.front();
        m_packetList.pop_front();

        return 0;
    }
    return -1;
}

