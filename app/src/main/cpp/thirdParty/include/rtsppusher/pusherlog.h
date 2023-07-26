#ifndef PUSHERLOG_H
#define PUSHERLOG_H

class PusherLog
{
public:
	virtual void LogDp(char const* const msg, ...) = 0;
	virtual void LogIp(char const* const msg, ...) = 0;
	virtual void LogWp(char const* const msg, ...) = 0;
	virtual void LogEp(char const* const msg, ...) = 0;
	virtual void LogFp(char const* const msg, ...) = 0;
};

#endif // !PUSHERLOG_H