#ifndef _OPERATION_H_
#define  _OPERATION_H_

#include "opbase.h"
#include <string>
#include <base/ef_btype.h>
#include <client_def.h>


using namespace ef;
#define KEEPALIVE_TIMEOUT 300

#define LOGIN_OP_SN "Login"
namespace gim
{
	class EventLoop;
	class CliConn;

	class LoginOp
		:public Op
	{
	public:
		LoginOp(const std::string& cid)
			:Op(LOGIN_OP_SN, cid),
			m_srvport(0)
		{
		}
		~LoginOp(){};
		int32 init(const std::string& srvip, int32 srvport, const std::string& cliver, int keepalive_timeout);
		virtual int32 process(EventLoop* el);
		virtual int32 OnTimeout(CliConn* conn);
	private:
		std::string m_srvip;
		int32 m_srvport;
		std::string m_cliver;
		int32 m_keepalive_timeout;
	};

	class KeepaliveOp
		:public Op
	{
	public:
		KeepaliveOp(const std::string& cid, int time_out=KEEPALIVE_TIMEOUT)
			:Op(std::string("keepalive"), cid)
		{
			setTimeout(time_out);
		}
		~KeepaliveOp(){};
		virtual int32 process(CliConn* conn);
		virtual int32 OnTimeout(CliConn* conn);
	};

	class DelConnOp
		:public Op
	{
	public:
		DelConnOp(const std::string& cid)
			:Op(std::string("delconnect"), cid)
		{
		}
		~DelConnOp(){};
		virtual int32 process(EventLoop* el);
	};
	class DisconnectOp
		:public Op
	{
	public:
		DisconnectOp(const std::string& cid)
			:Op(std::string("disconnect"), cid)
		{
		}
		~DisconnectOp(){};
		virtual int32 process(EventLoop* el);
	};
}
#endif
