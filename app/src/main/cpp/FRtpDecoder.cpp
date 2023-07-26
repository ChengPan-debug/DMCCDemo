//
// Created by DELL on 2020/10/29.
//

#include "FRtpDecoder.h"

FRTPDecoder::FRTPDecoder() {

}

static int _times = 0;

int FRTPDecoder::InitFRTPDecoder(int width, int height) {
    if (!FRTPDecoder::_frtpDecoder) {
        LOGE("use CreateFRTPDecoder first.");
        return -1;
    }

    AVIOContext *pb = NULL;
    AVInputFormat *piFmt = NULL;
    uint8_t *pIObuffer = NULL;
    int m_result;
    unsigned int video_stream_index;
    g_RTPRecvThreadRun = true;
    av_register_all();

    pIObuffer = (uint8_t *) av_malloc(4096);
    pb = avio_alloc_context(
            pIObuffer,
            4096,
            0,
            NULL,
            _fill_iobuffer,
            NULL,
            NULL);

//    int res = av_probe_input_buffer(pb, &piFmt, "", NULL, 0, 0);
//    if (res < 0)//探测从内存中获取到的媒体流的格式
//    {
//        LOGE("Error: probe format failed. %s", res);
//        return -1;
//    } else {
//        LOGW("input format:%s[%s]\n", piFmt->name, piFmt->long_name);
//
//    }
//    piFmt = av_find_input_format("h264");

    _fmtCtx = avformat_alloc_context();
    _fmtCtx->interrupt_callback.callback = FRTPDecoder::FFmpegCB;
    _fmtCtx->interrupt_callback.opaque = this;
    _fmtCtx->pb = pb;

    _fmtCtx->fps_probe_size = 5;
    _fmtCtx->probesize = 10 * 1024;
    _fmtCtx->max_analyze_duration = 5 * AV_TIME_BASE;;


    m_result = avformat_open_input(&_fmtCtx, "", piFmt, NULL);
    if (m_result < 0) {
        LOGE("Error: avformat_open_input failed----------------- ");
        //ASSERT(0);
        return -1;
    }

    _codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    _ctx = _fmtCtx->streams[0]->codec;

    _ctx->coded_width = width;
    _ctx->coded_height = height;

    _ctx->width = width;
    _ctx->height = height;
    _ctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    //_ctx->gop_size = 12;
    //_ctx->max_b_frames = 0;
    //_ctx->bit_rate = 0;
    //_ctx->rc_buffer_size = 0;

    AVCodecContext *ctxTmp = _ctx;
    memcpy(ctxTmp, _ctx, sizeof(_ctx));
    AVFrame *pFrame = av_frame_alloc();

    uint8_t *out_buffer = NULL;
    struct SwsContext *img_convert_ctx = NULL;

    avcodec_close(_ctx);
    _ctx = ctxTmp;

    int resCode = avcodec_open2(_ctx, _codec, NULL);
    if (resCode < 0) {
        printf("error %x in avcodec_open2\n", resCode);
        return 1;
    }
    _packet = av_packet_alloc();
    _rframe = av_frame_alloc();

    return 0;
}

int FRTPDecoder::DecodeFrame() {
    _times = 0;
    while (av_read_frame(_fmtCtx, _packet) == 0) {

        LOGI("read video steam index:%d", _packet->stream_index);

        if (_packet->stream_index == 0) {
            //讲原始数据发送到解码器
            int sendPacketState = avcodec_send_packet(_ctx, _packet);
            LOGI("avcodec_send_packet state:%d", sendPacketState);

            if (sendPacketState == 0) {
                int receiveFrameState = avcodec_receive_frame(_ctx, _rframe);
                LOGI("avcodec_receive_frame recv state:%d", receiveFrameState);
                if (receiveFrameState == 0) {
                    return 0;
                } else if (receiveFrameState == AVERROR(EAGAIN)) {
                    LOGD("从解码器-接收-数据失败：AVERROR(EAGAIN)");
                    return EAGAIN;
                } else if (receiveFrameState == AVERROR_EOF) {
                    LOGD("从解码器-接收-数据失败：AVERROR_EOF");
                    return AVERROR_EOF;
                } else if (receiveFrameState == AVERROR(EINVAL)) {
                    LOGD("从解码器-接收-数据失败：AVERROR(EINVAL)");
                    return EINVAL;
                } else {
                    LOGD("从解码器-接收-数据失败：未知");
                    return 1;
                }
            } else if (sendPacketState == AVERROR(EAGAIN)) {//发送数据被拒绝，必须尝试先读取数据
                LOGD("向解码器-发送-数据包失败：AVERROR(EAGAIN)");//解码器已经刷新数据但是没有新的数据包能发送给解码器
                return EAGAIN;
            } else if (sendPacketState == AVERROR_EOF) {
                LOGD("向解码器-发送-数据失败：AVERROR_EOF");
                return AVERROR_EOF;
            } else if (sendPacketState == AVERROR(EINVAL)) {//遍解码器没有打开，或者当前是编码器，也或者需要刷新数据
                LOGD("向解码器-发送-数据失败：AVERROR(EINVAL)");
                return EINVAL;
            } else if (sendPacketState == AVERROR(ENOMEM)) {//数据包无法压如解码器队列，也可能是解码器解码错误
                LOGD("向解码器-发送-数据失败：AVERROR(ENOMEM)");
                return ENOMEM;
            } else {
                LOGD("向解码器-发送-数据失败：未知");
                return 1;
            }
        }
        LOGI("read error.");
        return -1;
    }
    LOGI("end of DecodeFrame.");

    return -1;
}


//outBuffer.bits , outBuffer.stride * 4
int FRTPDecoder::FrameYUVSwap(uint8_t **dst, int &width, int &height) {

    int ret = 0;
    int ly = _rframe->linesize[0];
    int lu = _rframe->linesize[1];
    int lv = _rframe->linesize[2];

    LOGW("frame data:%d, %d, %d,width:%d", ly, lu, lv, width);
    if(ly == 0){
        return -1;
    }

//    if (ly != width) {
//        ret = 1;
//        width = ly;
//        delete[] dst[0];
//        delete[] dst[1];
//        delete[] dst[2];
//        dst[0] = new unsigned char[width * height];//y
//        dst[1] = new unsigned char[width * height / 4];//u
//        dst[2] = new unsigned char[width * height / 4];//v
//    }

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
    av_packet_unref(_packet);
    return ret;
}

int FRTPDecoder::GetWidth() {
    return _ctx->width;
}

int FRTPDecoder::GetHeight() {
    return _ctx->height;
}

FRTPDecoder::~FRTPDecoder() {
    g_RTPRecvThreadRun = false;
    {
        std::unique_lock<std::mutex> lck(mu);
        LOGI("~ m_packetList size: %d", m_packetList.size());
        while (!m_packetList.empty()) {
//            LOGI("~ m_packetList size: %d", m_packetList.size());
            PacketNode_t data = m_packetList.front();
            m_packetList.pop_front();
            delete[] data.buf;
        }
    }
    Release();
}

static int times = 0;

void FRTPDecoder::SetData(char *buf, int length) {
    if (!g_RTPRecvThreadRun) {
        LOGW("FRTPDecoder is stop.");
        return;
    }
    std::unique_lock<std::mutex> lck(mu);
    if (m_packetList.size() > 50)
        while (m_packetList.size() > 1) {
            PacketNode_t data = m_packetList.front();
            m_packetList.pop_front();
            delete[] data.buf;
        }

    PacketNode_t tmp;
    tmp.buf = new unsigned char[length];
    tmp.length = length;
    memcpy(tmp.buf, buf, length);
    m_packetList.push_back(tmp);
    LOGD("m_packetList size: %d ", m_packetList.size());

}

int FRTPDecoder::_fill_iobuffer(void *opaque, uint8_t *buf, int bufSize) {
    LOGD("run _fill_iobuffer...");
    int nBytes = _frtpDecoder->ReadBuf((char *) buf, bufSize, nullptr);
    return (nBytes > 0) ? bufSize : -1;
}

int FRTPDecoder::ReadBuf(char *data, int len, void *pContext) {
    int data_to_read = len;
    char *pReadPtr = data;

    while (g_RTPRecvThreadRun) {
        int nRead = ReadNetPacket((uint8_t *) pReadPtr, data_to_read);
        if (nRead < 0) {
            usleep(10);
            continue;
        }
        pReadPtr += nRead;
        data_to_read -= nRead;
        if (data_to_read > 0) {
            usleep(10);
            continue;
        }
        break;
    }

    return (data_to_read > 0) ? -1 : len;
}

std::list<PacketNode_t>  FRTPDecoder::m_packetList;

int FRTPDecoder::ReadNetPacket(uint8_t *buf, int buf_size) {
    int nsize = 0;
//    LOGD("ReadNetPacket start run size: %d", m_packetList.size());
    if (!m_packetList.empty()) {
        std::list<PacketNode_t>::iterator itr = m_packetList.begin();

        for (; itr != m_packetList.end();) {
            if (nsize < buf_size) {
                int nToReadSize = min(buf_size - nsize, itr->length);

                if (nToReadSize < itr->length) {
                    memcpy(buf + nsize, itr->buf, nToReadSize);
                    nsize += nToReadSize;

                    ::memmove(itr->buf, itr->buf + nToReadSize, itr->length - nToReadSize);
                    itr->length -= nToReadSize;
                    break;
                } else {
                    memcpy(buf + nsize, itr->buf, itr->length);
                    nsize += itr->length;
                }
            } else {
                break;
            }

            delete[] itr->buf; //�ͷ��ڴ�
            std::unique_lock<std::mutex> lck(mu);
            m_packetList.erase(itr++);   //listɾ��item
        }
    } else {
        nsize = -1;  //��ʾû�����ݿɶ�
    }

    //	m_cs.Unlock();

    return nsize;
}

FRTPDecoder *FRTPDecoder::_frtpDecoder = NULL;

FRTPDecoder *FRTPDecoder::CreateFRTPDecoder() {
    FRTPDecoder::_frtpDecoder = new FRTPDecoder();
    return FRTPDecoder::_frtpDecoder;
}

void FRTPDecoder::ReleaseFRTPDecoder() {
    if (FRTPDecoder::_frtpDecoder) {
        delete FRTPDecoder::_frtpDecoder;
        FRTPDecoder::_frtpDecoder = NULL;
    }

}

void FRTPDecoder::Stop() {
    g_RTPRecvThreadRun = false;
}

void FRTPDecoder::Release() {

    if (_rframe) {
        av_frame_free(&_rframe);
        LOGD("av_frame_free ");
    }
    if (_packet) {
        av_packet_free(&_packet);
        LOGD("av_packet_free ");
    }
    if (_ctx) {
        avcodec_free_context(&_ctx);
        LOGD("avcodec_free_context ");
    }
    if (_fmtCtx) {
//        avformat_close_input(&_fmtCtx);
//        avformat_free_context(_fmtCtx);
        LOGD("avformat_free_context null");
    }
}

int FRTPDecoder::FFmpegCB(void *param) {

    _times++;
    if (_times > 5) {
        //通知FFMpeg可以从阻塞工作线程中释放操作
        LOGW("FFmpegCB err: %d", _times);
        return 1;
    } else {
        //通知FFMpeg继续阻塞工作
        LOGW("FFmpegCB ");
        return 0;
    }

}

