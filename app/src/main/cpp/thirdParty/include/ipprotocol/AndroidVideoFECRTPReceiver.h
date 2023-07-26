#ifndef ANDROIDVIDEOFECRTPRECEIVER_H
#define ANDROIDVIDEOFECRTPRECEIVER_H

#ifdef _WIN32

#include <winsock2.h>
#include <ws2ipdef.h>
#pragma comment(lib,"Ws2_32.lib")
#else

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif // WIN32


#include "usefulStruct.h"
#include <queue>
#include "mpeg2rtp.h"
#include "IRTPReceiver.h"
#include <mutex>
#include "ipprotocollog.h"
#include <condition_variable>
#include "FecEncoder.h"
#include <functional>
#include <thread>
#include <string.h>

using namespace jrtplib;


class IPPROTOCOL_API AndroidVideoFECRTPReceiver : public IRTPReceiver
{
public:
	AndroidVideoFECRTPReceiver(uint32_t ssrc);
	~AndroidVideoFECRTPReceiver();
	virtual PacketNode_t GetPacketNode(int timeout_msc);
	virtual void ReleasePacketNode(unsigned char *buf) override;

public:
	//  IRTPReceiver 
	/*
	* localIpStr not use in Android
	*/
	virtual int Init(const char * ipStr, int port, const char* localIpStr = "0.0.0.0") override;
	virtual int Init_Single(const char * ipStr, int port, const char* localIpStr = "0.0.0.0") override;
	virtual void Dispose() override;

protected:

	virtual void OnPollThreadStep();

	virtual uint32_t GetPayloadType();

	PacketNode_t InitFec(fec_oti_t* s, int timeout_msc);
	void PopPacket();
	void DeletePacket(RTP_FIXED_HEADER * pack);

	RTP_FIXED_HEADER * GetNextPacket(int timeout_msc);

protected:
	bool m_run = false;
	bool joinGroup = false;
	fec_oti_t oldFec;
	uint32_t m_ssrc;
	uint32_t m_payloadType;
	std::condition_variable cv;
	std::mutex mtx;

	std::queue<RTP_FIXED_HEADER *> qrtp;
	std::mutex mu_rtpPack;
	std::thread* m_poll = nullptr;
	int len_addr;

	ip_mreq *gaddr = nullptr;
	sockaddr *laddr = nullptr;
#ifdef _WIN32
	SOCKET  m_socket;
#else
	int m_socket;
#endif // _WIN32

};

#endif //VIDEOANDROIDFECRTPRECEIVER_H
