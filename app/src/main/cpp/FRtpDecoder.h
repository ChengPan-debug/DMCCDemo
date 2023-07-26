//
// Created by DELL on 2020/10/29.
//

#ifndef MFFMPEG_RTPDECODER_H
#define MFFMPEG_RTPDECODER_H

extern "C" {
#include "ffmpeg/libswresample/swresample.h"
#include "ffmpeg/libavformat/avformat.h"
#include "ffmpeg/libavformat/avio.h"
#include "ffmpeg/libavfilter/avfilter.h"
#include "ffmpeg/libavfilter/buffersrc.h"
#include "ffmpeg/libavfilter/buffersink.h"
#include "ffmpeg/libavutil/channel_layout.h"
#include "ffmpeg/libavutil/intreadwrite.h"
#include "ffmpeg/libavutil/mathematics.h"
#include "ffmpeg/libavutil/parseutils.h"
#include "ffmpeg/libavutil/samplefmt.h"
#include "ffmpeg/libavutil/fifo.h"
#include "ffmpeg/libavutil/opt.h"
#include "ffmpeg/libavutil/dict.h"
#include "ffmpeg/libavutil/pixdesc.h"
#include "ffmpeg/libavutil/avstring.h"
#include "ffmpeg/libavutil/imgutils.h"
#include "ffmpeg/libavcodec/avcodec.h"
#include "ffmpeg/libavutil/time.h"
#include "ffmpeg/libswscale/swscale.h"
}

#include "ipprotocol/IRTPReceiver.h"
#include "ipprotocol/ipprotocoldeflog.h"
#include "unistd.h"
#include <functional>
#include <thread>
#include <list>
#include<mutex>

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef  PIXFMT
#define PIXFMT AV_PIX_FMT_YUVJ420P
#endif

class FRTPDecoder {

public:
	static FRTPDecoder * CreateFRTPDecoder();
	static void ReleaseFRTPDecoder();
	void Release();

    int InitFRTPDecoder(int width, int height);
    void SetData(char* buf,int length);
    int DecodeFrame();
    int FrameYUVSwap(uint8_t ** dst,int &width, int &height);
    void Stop();

    int GetWidth();
    int GetHeight();
private:
	static int FFmpegCB(void* param);

	FRTPDecoder();
	~FRTPDecoder();

	static int _fill_iobuffer(void* opaque, uint8_t* buf, int bufSize);
	static FRTPDecoder *_frtpDecoder;
	int ReadBuf(char* data, int len, void* pContext);
	int ReadNetPacket(uint8_t *buf, int buf_size);

	bool g_RTPRecvThreadRun;

private:
    AVFormatContext* _fmtCtx = NULL;
    AVCodecContext* _ctx = NULL;
    AVPacket* _packet = NULL;
    AVFrame* _rframe = NULL;
    AVCodec * _codec = NULL;

	std::mutex mu;
	static std::list<PacketNode_t>  m_packetList;

};


#endif //MFFMPEG_HYDECODER_H
