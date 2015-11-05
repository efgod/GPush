#include "ops.h"
#include <proto/gpush.pb.h>
#include "msg_head.h"
#include "eventloop.h"
#include "client_conn.h"
#include "client_log.h"
#include <json/json.h>
#include "err_no.h"
#include <base/ef_base64.h>

namespace gim
{
	// class LoginOp
	int32 LoginOp::init(const std::string& srvip, int32 srvport, const std::string& cliver, int32 keepalive_timeout)
	{
		m_srvip = srvip;
		m_srvport = srvport;
		m_cliver = cliver;
		m_keepalive_timeout = keepalive_timeout;
		return 0;
	}
	
	int32 LoginOp::process(EventLoop* el)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "LoginImsOp::process");
		if (!el)
		{
			return -1;
		}
		CliConn* conn = el->findConn(getCid());
		if (!conn)
		{
			conn = el->addConn(getCid());
			conn->setCid(getCid());
			conn->setCliver(m_cliver);
			conn->setSrvAddr(m_srvip, m_srvport);
			conn->setKeepaliveTimeout(m_keepalive_timeout);
		}
		else
		{
			SDK_LOG(LOG_LEVEL_ERROR, "LoginImsOp::process, connection %s exists", getCid().c_str());
			return -1;
		}

		int32 ret = conn->connectAndLogin();
		if (ret == 0)
		{
			increase_();
		}
		return ret;
	}
	int32 LoginOp::OnTimeout(CliConn* conn)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "LoginImsOp::OnTimeout");
		if (conn)
		{
			conn->OnLoginFail(GIM_TIMEOUT);
		}

		return 0;
	}
	//KeepaliveOp
	int32 KeepaliveOp::process(CliConn* conn)
	{
		if (conn)
		{
			increase_();
			return 0;
		}
		return -1;
	}
	int32 KeepaliveOp::OnTimeout(CliConn* conn)
	{
		if (conn)
		{
			return conn->keepalive();
		}
		return -1;
	}
	// class DelConnOp
	int32 DelConnOp::process(EventLoop* el)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "DelConnOp::process");
		if (el)
		{
			return el->delConn(getCid());
		}
		return -1;
	}
	// class DisconnectOp
	int32 DisconnectOp::process(EventLoop* el)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "DisconnectOp::process");
		CliConn* conn = el ? el->findConn(getCid()) : NULL;
		if (conn)
		{
			conn->onDisconnect(true, 0);
			return 0;
		}
		return -1;
	}
}//end of namespace

