#ifndef RECEIVERLOGLOG_H
#define RECEIVERLOGLOG_H

class ReceiverLog
{
public:
	virtual void LogDp(char const* const msg, ...) = 0;
	virtual void LogIp(char const* const msg, ...) = 0;
	virtual void LogWp(char const* const msg, ...) = 0;
	virtual void LogEp(char const* const msg, ...) = 0;
	virtual void LogFp(char const* const msg, ...) = 0;
};

#endif // !RECEIVERLOGLOG_H