#ifndef MYVIDEODECODER_H
#define MYVIDEODECODER_H

extern "C" {
#include "libswresample/swresample.h"
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

#include "unistd.h"
#include <functional>
#include <thread>
#include <list>
#include <mutex>

class VideoDecoder
{
public:
    static VideoDecoder * createVideoDecoder();
	void releaseVideoDecoder();

    int initDecoder(int width, int height);

    int getWidth();
    int getHeight();

    //用该数据生成一个AVPacket，放到 m_packetList
    void appendData(char *data, int size);  
    //从packetList中取数据，解码放到 _rframe
    int decodeFrame();
    //_rframe转换成YUV，放入dst中
    int getYUVData(uint8_t ** dst, int &width, int &height);

    //获取像素格式
    AVPixelFormat getPixelFormat();

private:
	VideoDecoder();
	~VideoDecoder();

	static VideoDecoder *_decoder;
	int readNextPacket();

private:
    AVCodecContext* _ctx = NULL;
    AVPacket* _packet = NULL;
    AVFrame* _rframe = NULL;
    AVPixelFormat m_pixelFormat = AV_PIX_FMT_NONE;

	std::mutex mu;
	std::list<AVPacket*>  m_packetList;

    bool m_init;
    bool m_firstDecode;
};

#endif //MYVIDEODECODER_H