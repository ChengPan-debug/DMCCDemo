/**
 * \file IRTPSender.h
 */

#ifndef IRTPSENDER_H
#define IRTPSENDER_H

#include "ipprotocol.h"
#include <stdint.h>

/**	RTPSender Interface
 *	RTPSender Interface, how to use
 *	IRTPSender *sender = new xxRTPSender();
 *  sender->SetRTPParams();
 *  sender->SetRTPParamsForSending();
 *  SendRTP->SendRTP();
 * 
 *  delete SendRTP;
 * 
*/
class IPPROTOCOL_API IRTPSender
{
public:
	/** virtual delete ctor */
	virtual ~IRTPSender() {}
	/** set RTPParams
	 * set RTPParams， \c destip, \c destport, \c baseport
	*/
	virtual int SetRTPParams(const char * destip, uint16_t destport, uint16_t baseport) = 0;
	virtual int SendRTP(unsigned char* m_h264Buf, int buflen) = 0;
	virtual void SetRTPParamsForSending() = 0;
};

#endif // !IRTPSender_H
