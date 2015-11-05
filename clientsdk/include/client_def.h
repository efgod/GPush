#ifndef CLIENT_DEF_H
#define CLIENT_DEF_H

namespace gim
{
	typedef enum _LoginStatus
	{
		STATUS_DO_LOGIN = 1,
		STATUS_LOGIN = 2,
		STATUS_LOGIN_FAIL = 3,
		STATUS_DISCONNECT = 4
	}LoginStatus;

	typedef enum _GResult
	{
		GIM_OK = 0,
		GIM_ERROR = -1,
		GIM_NETWORK_ERROR = -9999,
		GIM_PROBUF_FORMAT_ERROR = -9998,
		GIM_TOO_LONG_PACKET = -9997,
		GIM_JNI_ERROR = -9996,
		GIM_JSON_ERROR = -9995,
		GIM_UNDEFINED_CMD = -9994,
		GIM_NOT_LOGGED = -9993,
		GIM_TIMEOUT	= -9992
	}GResult;

	typedef enum _GEventType
	{
		GIM_EVTYPE_LOGIN_OK = 0,
		GIM_EVTYPE_LOGIN_FAIL = 1,
		GIM_EVTYPE_LOGOUT = 2,
		GIM_EVENT_PUSH = 10000
	}GEventType;

};

#endif
