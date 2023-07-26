/**
 * \file VideoFECRTPReceiver.h
 */

#ifndef VIDEOFECRTPRECEIVER_H
#define VIDEOFECRTPRECEIVER_H

#include "AbsVideoRTPReceiver.h"

#include "FecEncoder.h"

class IPPROTOCOL_API VideoFECRTPReceiver : public AbsVideoRTPReceiver
{
public:
	VideoFECRTPReceiver(uint32_t ssrc);
	~VideoFECRTPReceiver();
	virtual PacketNode_t GetPacketNode(int timeout_msc);

protected:
	virtual uint32_t GetPayloadType();


	PacketNode_t InitFec(fec_oti_t* s, int timeout_msc);
	void PopPacket();
	RTPPacket * GetNextPacket(int timeout_msc);
	fec_oti_t oldFec;
};

#endif //VIDEOFECRTPRECEIVER_H