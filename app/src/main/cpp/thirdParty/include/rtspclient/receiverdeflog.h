#ifndef RECEIVERDEFLOG_H
#define RECEIVERDEFLOG_H

#include "receiverlog.h"

class ReceiverDefLog : public ReceiverLog
{
public:
	virtual void LogDp(char const* const msg, ...) override;
	virtual void LogIp(char const* const msg, ...) override;
	virtual void LogWp(char const* const msg, ...) override;
	virtual void LogEp(char const* const msg, ...) override;
	virtual void LogFp(char const* const msg, ...) override;
};

#endif // !RECEIVERDEFLOG_H


