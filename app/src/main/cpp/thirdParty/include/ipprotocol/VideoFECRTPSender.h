/**
 * \file VideoFECRTPSender.h
 */

#ifndef VIDEOFECRTPSENDER_H
#define VIDEOFECRTPSENDER_H

#include "AbsVideoRTPSender.h"
#include "FecEncoder.h"
#include <math.h>


class IPPROTOCOL_API VideoFECRTPSender : public AbsVideoRTPSender
{
public:
	VideoFECRTPSender(uint32_t ssrc, double rate = 0.8);
	~VideoFECRTPSender();

private:
	void TestDecode(fec_oti_t	fec_oti, void **array);

public:
	// ͨ�� IRTPSender �̳�
	virtual int SendRTP(unsigned char* m_h264Buf, int buflen) override;
	virtual void SetRTPParamsForSending() override;

private:
	double m_rate;
};

#endif