#ifndef ANDROIDVIDEOFECRTPRECEIVERV2_H
#define ANDROIDVIDEOFECRTPRECEIVERV2_H
#include "AbsVideoRTPReceiver.h"

#include "FecEncoder.h"


#include <condition_variable>
#include <functional>
#include <thread>
#include <string.h>

using namespace jrtplib;


class IPPROTOCOL_API AndroidVideoFECRTPReceiverV2 : public AbsVideoRTPReceiver
{
public:
    AndroidVideoFECRTPReceiverV2(uint32_t ssrc);
	~AndroidVideoFECRTPReceiverV2();
	virtual PacketNode_t GetPacketNode(int timeout_msc);
	virtual int Init_Single(const char * ipStr, int port, const char* localIpStr="0.0.0.0") override;
	virtual void Dispose() override;
	void PollThreadStep();

protected:
	virtual uint32_t GetPayloadType();

	PacketNode_t InitFec(fec_oti_t* s, int timeout_msc);
	void PopPacket();
	RTPPacket * GetNextPacket_V2(int timeout_msc);
	fec_oti_t oldFec;

	bool m_run = false;
	std::thread* m_poll = nullptr;
};

#endif //ANDROIDVIDEOFECRTPRECEIVERV2_H
