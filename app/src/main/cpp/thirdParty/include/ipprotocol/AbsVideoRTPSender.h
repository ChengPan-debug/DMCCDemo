/**
 * \file AbsVideoRTPSender.h
 */

#ifndef ABSVIDEORTPSENDER_H
#define ABSVIDEORTPSENDER_H


#include "jrtplib3/rtpsession.h"
#include "jrtplib3/rtpsessionparams.h"
#include "jrtplib3/rtpudpv4transmitter.h"
#include "jrtplib3/rtpipv4address.h"
#include "jrtplib3/rtptimeutilities.h"
#include "jrtplib3/rtppacket.h"
#include "jrtplib3/rtcpapppacket.h"
#include "jrtplib3/rtpsourcedata.h"
#include "mpeg2rtp.h"
#include "IRTPSender.h"
#include <list>

#pragma warning(disable : 4996)

using namespace jrtplib;

typedef void(*CallBack_RTCP_Control)(int state);

class IPPROTOCOL_API AbsVideoRTPSender : public IRTPSender, public RTPSession
{
public:
	AbsVideoRTPSender(uint32_t ssrc);
	virtual ~AbsVideoRTPSender();

	// ͨ�� IRTPSender �̳�
	virtual int SetRTPParams(const char * destip, uint16_t destport, uint16_t baseport) override;
	virtual int SendRTP(unsigned char * m_h264Buf, int buflen) = 0;


	void OnNewSource(RTPSourceData *srcdat);

	void OnRemoveSource(RTPSourceData *srcdat);

	void AddRecvDestination(const RTPIPv4Address &addr, uint32_t ssrc);
	void DeleteRecvDestination(uint32_t ssrc);

	virtual void SetRTPParamsForSending() = 0;

	CallBack_RTCP_Control m_lp;

protected:

	virtual void OnAPPPacket(RTCPAPPPacket *apppacket, const RTPTime &receivetime,
		const RTPAddress *senderaddress);

protected:
	typedef struct {
		RTPIPv4Address *dest_addr;
		std::list<uint32_t> list_ssrc;
	}RecvDestination;

	uint32_t m_ssrc;
	std::list<RecvDestination*> list_recvdest;
};

#endif // !ABSVIDEORTPSENDER_H