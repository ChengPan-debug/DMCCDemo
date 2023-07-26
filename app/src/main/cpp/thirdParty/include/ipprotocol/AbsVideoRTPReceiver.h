/**
 * \file AbsVideoRTPReceiver.h
 */

#ifndef ABSVIDEORTPRECEIVER_H
#define ABSVIDEORTPRECEIVER_H

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
#include "IRTPReceiver.h"
#include<mutex>
#include "ipprotocollog.h"

#include <condition_variable>

using namespace jrtplib;


class IPPROTOCOL_API AbsVideoRTPReceiver : public IRTPReceiver , public RTPSession
{
public:
	AbsVideoRTPReceiver(uint32_t ssrc);
	~AbsVideoRTPReceiver();

protected:
    virtual void OnPollThreadStep();
	virtual void ProcessRTPPacket(const RTPSourceData &srcdat, RTPPacket *rtppack);
	virtual uint32_t GetPayloadType() = 0;

	//android poll thread
	void run_android();

public:
	//  IRTPReceiver 
	virtual int Init(const char * ipStr, int port, const char* localIpStr = "0.0.0.0") override;
	virtual int Init_Single(const char * ipStr, int port, const char* localIpStr="0.0.0.0") override;
	virtual void Dispose() override;
	virtual PacketNode_t GetPacketNode(int timeout_msc) = 0;
	virtual void ReleasePacketNode(unsigned char* buf) override;
	
protected:
	uint32_t m_ssrc;
	uint32_t m_payloadType;
	std::mutex mtx;

	std::queue<RTPPacket *> qrtp;
	std::mutex m_rtpPack;
//	CCriticalSection  m_rtpPack;
	RTPIPv4Address *gaddr = nullptr;

	bool joinGroup;
};

#endif //ABSVIDEORTPRECEIVER_H