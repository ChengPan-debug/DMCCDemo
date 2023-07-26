/**
 * \file VideoRTPSender.h
 */

#ifndef VIDEORTPSENDER_H
#define VIDEORTPSENDER_H


#include <iostream>
#include "AbsVideoRTPSender.h"

/** VideoRTP sender, inherits AbsVideoRTPSender
 */
class IPPROTOCOL_API VideoRTPSender : public AbsVideoRTPSender
{
public:
	VideoRTPSender(uint32_t ssrc);
	~VideoRTPSender();

public:
	virtual int SendRTP(unsigned char* m_h264Buf, int buflen);
	virtual void SetRTPParamsForSending();
protected:
};

#endif // !VIDEORTPSENDER_H

