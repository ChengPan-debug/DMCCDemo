/**

\htmlonly
<style type="text/css">
body {
    counter-reset: section;
}

h2 {
    counter-increment: section;
    counter-reset: subsection;
}

h3 {
    counter-increment: subsection;
    counter-reset: subsubsection;
}

h4 {
    counter-increment: subsubsection;
}

h2:before {
    content: counter(section) ". ";
}

h3:before {
    content: counter(section) "." counter(subsection) " ";
}

h4:before {
    content: counter(section) "." counter(subsection) "." counter(subsubsection) " ";
}
</style>
\endhtmlonly

\mainpage ipprotocol

\author chown qiang
\author Developed at the the huisheng ltd. company url is [http://www.toppa.biz].

## Introduction ## 

This document describes libs by ipprotocol, and object-oriented library written in C++.
how to use

~~~{.cpp}

    IRTPSender *sender = new xxRTPSender();

    sender->SetRTPParams("224.0.2.10",1234,0);

    sender->SetRTPParamsForSending();
    
    SendRTP->SendRTP(data, 20360);

    delete SendRTP;

~~~
*/
#pragma once

#include <iostream>
#include "ipprotocoldeflog.h"

#if defined(_WIN32)
	#ifdef IPPROTOCOL_EXPORTS
		#define IPPROTOCOL_API __declspec(dllexport)
	#else
		#define IPPROTOCOL_API __declspec(dllimport)
	#endif //IPPROTOCOL_EXPORTS 
#else
	#define IPPROTOCOL_API 
#endif

namespace ipprotocol{
	void IPPROTOCOL_API initLog(IpprotocolLog *log = nullptr);
	void IPPROTOCOL_API releaseLog();
}

// TODO: 在此处引用程序需要的其他标头。
