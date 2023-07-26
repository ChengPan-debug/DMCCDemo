/**
 * \file FecEncoder.h
 */

#ifndef FECENCODER_H
#define FECENCODER_H

#ifdef WIN32
	#ifdef _DEBUG
	#pragma comment(lib,"openfec_d.lib")
	#else
	#pragma comment(lib,"openfec.lib")
	#endif
#endif

#define RTP_FEC                    127
#define CODE_RATE (0.8)

extern "C" {
#include "of_openfec_api.h"

};

#include "of_types.h"

typedef struct {
	UINT32		codec_id;	/* identifies the code/codec being used. In practice, the "FEC encoding ID" that identifies the FEC Scheme should
					 * be used instead (see [RFC5052]). In our example, we are not compliant with the RFCs anyway, so keep it simple. */
	UINT32		k;
	UINT32		n;
} fec_oti_t;

void randomize_array(UINT32	**array, UINT32	arrayLen);

#endif // !FECENCODER_H
