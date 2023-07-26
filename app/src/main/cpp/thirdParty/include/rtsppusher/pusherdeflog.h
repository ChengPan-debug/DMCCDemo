#ifndef PUSHERDEFLOG_H
#define PUSHERDEFLOG_H

#include "pusherlog.h"

class PusherDefLog : public PusherLog
{
public:
	virtual void LogDp(char const* const msg, ...) override;
	virtual void LogIp(char const* const msg, ...) override;
	virtual void LogWp(char const* const msg, ...) override;
	virtual void LogEp(char const* const msg, ...) override;
	virtual void LogFp(char const* const msg, ...) override;
};

#endif // !PUSHERDEFLOG_H


