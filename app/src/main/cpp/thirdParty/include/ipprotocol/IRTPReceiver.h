/**
 * \file IRTPReceiver.h
 */

#ifndef IRTPRECEIVER_H
#define IRTPRECEIVER_H

#include "ipprotocol.h"
#include "usefulStruct.h"

class IPPROTOCOL_API IRTPReceiver
{
public:
	virtual ~IRTPReceiver() {}
	virtual int Init(const char* ipStr, int port, const char* localIpStr = "0.0.0.0") = 0;
	virtual int Init_Single(const char* ipStr, int port, const char* localIpStr= "0.0.0.0") = 0;
	virtual PacketNode_t GetPacketNode(int timeout_msc = 3000) = 0;
	virtual void ReleasePacketNode(unsigned char *buf) = 0;
	virtual void Dispose() = 0;
};


#endif //IRTPRECEIVER_H