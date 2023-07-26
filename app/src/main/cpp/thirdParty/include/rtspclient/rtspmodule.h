#pragma once

#include "RTSPClient.h"
#include "RTSPCommonEnv.h"

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
using namespace std;

#if defined(_MSC_VER)
	#ifdef RTSPCLIENTDLL_EXPORTS
	#define RTSPCLIENTDLL_API __declspec(dllexport)
	#else
	#define RTSPCLIENTDLL_API __declspec(dllimport)
	#endif
#else
	#define RTSPCLIENTDLL_API 
#endif

void adts_header(char *szAdtsHeader, int dataLen) {

	int audio_object_type = 2;
	int sampling_frequency_index = 3;
	int channel_config = 2;

	int adtsLen = dataLen + 7;

	szAdtsHeader[0] = 0xff;         //syncword:0xfff                          ��8bits
	szAdtsHeader[1] = 0xf0;         //syncword:0xfff                          ��4bits
	szAdtsHeader[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
	szAdtsHeader[1] |= (0 << 1);    //Layer:0                                 2bits 
	szAdtsHeader[1] |= 1;           //protection absent:1                     1bit

	szAdtsHeader[2] = (audio_object_type - 1) << 6;            //profile:audio_object_type - 1                      2bits
	szAdtsHeader[2] |= (sampling_frequency_index & 0x0f) << 2; //sampling frequency index:sampling_frequency_index  4bits 
	szAdtsHeader[2] |= (0 << 1);                             //private bit:0                                      1bit
	szAdtsHeader[2] |= (channel_config & 0x04) >> 2;           //channel configuration:channel_config               ��1bit

	szAdtsHeader[3] = (channel_config & 0x03) << 6;     //channel configuration:channel_config      ��2bits
	szAdtsHeader[3] |= (0 << 5);                      //original��0                               1bit
	szAdtsHeader[3] |= (0 << 4);                      //home��0                                   1bit
	szAdtsHeader[3] |= (0 << 3);                      //copyright id bit��0                       1bit  
	szAdtsHeader[3] |= (0 << 2);                      //copyright id start��0                     1bit
	szAdtsHeader[3] |= ((adtsLen & 0x1800) >> 11);           //frame length��value   ��2bits

	szAdtsHeader[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    �м�8bits
	szAdtsHeader[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    ��3bits
	szAdtsHeader[5] |= 0x1f;                                 //buffer fullness:0x7ff ��5bits
	szAdtsHeader[6] = 0xfc;
}

struct DataNode
{
	RTP_FRAME_TYPE frame_type; 
	int64_t timestamp; 
	unsigned char *buf; 
	int len;

	DataNode()
	{
		frame_type = FRAME_TYPE_VIDEO;
		timestamp = 0;
		buf = nullptr;
		len = 0;
	}
};

enum RECV_DATA_TYPE {VIDEO_AUDIO, VIDEO, AUDIO};

class RTSPCLIENTDLL_API RtspModule
{
public:
	RtspModule();
	~RtspModule();

	void setRecvFlag(RECV_DATA_TYPE flag);
	void setCachePacketMax(int num);

	int openUrl(const char *url, int streamType, int timeout, bool rtpOnly = false);
	void closeUrl();

	DataNode getVideoDataNode(int timeout = 300);
	DataNode getAudioDataNode(int timeout = 300);
	DataNode getAudioAddHreadNode(int timeout = 300);
	int getQueueSize(RTP_FRAME_TYPE frame_type);

	void releaseNodeBuffer(uint8_t *buffer);

	void push(RTP_FRAME_TYPE frame_type, int64_t timestamp, unsigned char *buf, int len);
	
private:
	void run();
	DataNode popNode(RTP_FRAME_TYPE frame_type);			//old
	void releaseAllDataNode(RTP_FRAME_TYPE frame_type);

	DataNode popVideoNode(int timeout);
	DataNode popAudioNode(int timeout);

private:
	RTSPClient *m_rtspClient;
	bool m_isStop;
	bool m_isRuning;
	bool m_isReleaseAllAudio;

	RECV_DATA_TYPE m_recvType;
	int m_cacheMaxNum;

	std::thread *m_thread;

	std::mutex m_mutex;

	std::queue<DataNode> m_videoNodeQueue;
	std::queue<DataNode> m_audioNodeQueue;

	std::string m_url;
	int m_streamType;
	int m_timeout;
	bool m_rtpOnly;
};

