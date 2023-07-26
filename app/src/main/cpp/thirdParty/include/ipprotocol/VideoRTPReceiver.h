/**
 * \file VideoRTPReceiver.h
 */

#ifndef VIDEORTPRECEIVER_H
#define VIDEORTPRECEIVER_H

#include "jrtplib3/rtpsession.h"
#include "jrtplib3/rtpsessionparams.h"
#include "jrtplib3/rtpudpv4transmitter.h"
#include "jrtplib3/rtpipv4address.h"
#include "jrtplib3/rtptimeutilities.h"
#include "jrtplib3/rtpsourcedata.h"
#include "jrtplib3/rtppacket.h"
#include "usefulStruct.h"
#include <queue>
#include "mpeg2rtp.h"
#include "AbsVideoRTPReceiver.h"

using namespace jrtplib;


class IPPROTOCOL_API VideoRTPReceiver : public AbsVideoRTPReceiver
{
public:
	VideoRTPReceiver(uint32_t ssrc);
	~VideoRTPReceiver();
	virtual PacketNode_t GetPacketNode(int timeout_msc=0);

private:
	int rtp_unpackage_H264(unsigned char *payload, int len, int & marker, unsigned char & cFrameType, unsigned char * outbuf, int & total_bytes);

protected:
	virtual uint32_t GetPayloadType();
	RTPPacket* GetNextPacket();

protected:
	unsigned char *frame_buffer;
	int frame_size;
	bool start;

	unsigned char cFrameType;
	unsigned char temp_buffer[1500];
	int temp_size;
};

#endif //VIDEORTPRECEIVER_H