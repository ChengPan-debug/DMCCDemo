#ifndef UTILS_H
#define UTILS_H

#include "ipprotocol.h"

namespace ipprotocol
{

class IPPROTOCOL_API Utils
{
public:
	long long GetSystemSec(); 
	long long GetSystemMs();
	long long GetSystemMicrosecond();
	long long GetSystemNs();
};

}

#endif // !UTILS_H
