#include "client_conn.h"
#include "client_log.h"
#include "proto/gpush.pb.h"
#include <json/json.h>
#include <errno.h>
#include <base/ef_md5.h>
#include <base/ef_base64.h>
#include "ops.h"
#include "err_no.h"
#include "eventloop.h"
#include <json/json.h>

namespace gim
{
	CliConn::CliConn(EventLoop* lp)
		:m_fd(INVALID_SOCKET),
		m_devicetype(0),
		 m_keepalive_timeout(KEEPALIVE_TIMEOUT),
		m_loginStatus(STATUS_DISCONNECT),
		m_login_time(0),
		m_evlp(lp)
	{
		assert(m_evlp);
		m_active_time = ef::gettime_ms();
	}
	CliConn::~CliConn()
	{
	}
	void CliConn::setCliver(const std::string& cliver)
	{
		m_cli_ver = cliver;
	}
	void CliConn::setCid(const std::string& cid)
	{
		m_cid = cid;
	}
	const std::string& CliConn::getCid()
	{
		return m_cid;
	}
	int32 CliConn::setSrvAddr(const std::string& ip, int32 port)
	{
		return addSrvaddr(ip, port);
	}
	int32 CliConn::addSrvaddr(const std::string& ip, int32 port)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "CliConn::addSrvaddr %s:%d", ip.c_str(), port);
		m_svrlist.push_back(AddrItem(ip, port));
		return 0;
	}
	void CliConn::closefd()
	{
		SDK_LOG(LOG_LEVEL_TRACE, "cid=%s, CliConn::closefd", m_cid.c_str());
		if (m_fd != INVALID_SOCKET)
		{
			closesocket(m_fd);
			m_fd = INVALID_SOCKET;
		}
	}
	void CliConn::asynDestroy()
	{
		DelConnOp* op = new DelConnOp(getCid());
		m_evlp->asynAddOp((Op*)op);
	}
	int32 CliConn::connectServer()
	{
		while (!m_svrlist.empty())
		{
			AddrItem item = m_svrlist.back();
			SDK_LOG(LOG_LEVEL_TRACE, "connect %s:%d", item.ip.c_str(), item.port);
			m_fd = tcp_connect(item.ip.c_str(), item.port, "", 0);
			if (m_fd == INVALID_SOCKET)
			{
				SDK_LOG(LOG_LEVEL_ERROR, "connect fail");
				m_svrlist.pop_back();
			}
			else
			{
				break;
			}
		}

		if (m_fd == INVALID_SOCKET)
		{
			OnLoginFail(GIM_NETWORK_ERROR);
			return -1;
		}
		return 0;
	}
	int32 CliConn::onDisconnect(bool notify, int code)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "cid=%s, CliConn::onDisconnect", m_cid.c_str());
		for (TimerList::reverse_iterator rit = m_timers.rbegin(); rit != m_timers.rend(); ++rit)
		{
			if (rit->second.get())
			{
				rit->second->OnCancel(this);
			}
		}
		closefd();
		setStatus(STATUS_DISCONNECT, code,true);
		asynDestroy();
		return 0;
	}
	int32 CliConn::OnLoginFail(int code)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "cid=%s, CliConn::OnLoginFail", m_cid.c_str());
		closefd();
		setStatus(STATUS_LOGIN_FAIL, code, true);
		asynDestroy();
		return 0;
	}
	int32 CliConn::publish(const std::string& json)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "[cid=%s] publish: %s", m_cid.c_str(), json.c_str());
		m_evlp->publish(json);
		return 0;
	}

	void CliConn::setStatus(int32 status, int32 code, bool notify)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "[cid=%s] setStatus before=%d, now=%d, code=%d, notify=%d", m_cid.c_str(), m_loginStatus, status, code, int32(notify));
		if (status != m_loginStatus)
		{
			m_loginStatus = status;
			m_loginStatusCode = code;

			while (notify)
			{
				Json::FastWriter w;
				Json::Value jev;
				jev["cid"] = getCid();
				if (m_loginStatus == STATUS_LOGIN){
					jev["evtype"] = GIM_EVTYPE_LOGIN_OK;
				}
				else if (m_loginStatus == STATUS_LOGIN_FAIL){
					jev["evtype"] = GIM_EVTYPE_LOGIN_FAIL;
					jev["code"] = m_loginStatusCode;
				}
				else if (m_loginStatus == STATUS_DISCONNECT){
					jev["evtype"] = GIM_EVTYPE_LOGOUT;
					jev["code"] = m_loginStatusCode;
				}
				else{
					break;
				}
				publish(w.write(jev).c_str());
				break;
			}
		}
	}
	int32 CliConn::connectAndLogin()
	{
		if ((m_loginStatus != STATUS_DO_LOGIN) 
			&& (m_loginStatus != STATUS_LOGIN) 
			&& connectServer() == 0)
		{
			return login();
		}
		return -1;
	}
	int32 CliConn::login()
	{
		SDK_LOG(LOG_LEVEL_TRACE, "CliConn::login");
		setStatus(STATUS_DO_LOGIN);

		int32 ret = 0;
		m_login_time = gettime_ms();
		LoginRequest lgr;
		lgr.set_id(getCid());
		lgr.set_version(m_cli_ver);
		lgr.set_type(-1);

		std::string body;
		lgr.SerializeToString(&body);

		ret = sendPacket(LOGIN_REQ, body);
		return ret >= 0 ? 0 : -1;
	}
	int32 CliConn::handleRead()
	{
		SDK_LOG(LOG_LEVEL_TRACE, "cid=%s, CliConn::handleRead", m_cid.c_str());
		if(m_fd == INVALID_SOCKET){
			return -1;
		}
		int32	ret = 0;
		char*	tmpbuf = NULL;
		int32	wantlen = 0;
		int32	actrcv = 0;
		int32	totalrcv = 0;
		m_active_time = ef::gettime_ms();
		while (true)
		{
			actrcv = 0;
			ret = 0;
			wantlen = m_buf.next_part_len();
			if (wantlen <= 0)
			{
				m_buf.resize(m_buf.capacity() + 16 * 1024);
				wantlen = m_buf.next_part_len();
			}

			tmpbuf = (char*)m_buf.next_part();

			ret = tcp_nb_receive(m_fd, tmpbuf, wantlen, &actrcv);
			if (ret < 0)
			{
				SDK_LOG(LOG_LEVEL_ERROR, "cid=%s, recvLoop recv  error=%d, %d", m_cid.c_str(), ret, sock_errno);
				return -1;
			}
			if (actrcv == 0)
			{
				break;
			}
			totalrcv += actrcv;
			m_buf.write(NULL, actrcv);
		};

		while (true)
		{
			head h;
			if (m_buf.peek((uint8*)&h, sizeof(h)) < sizeof(h))
			{
				break;
			}
			h.magic = ntohl(h.magic);	
			h.len = ntohl(h.len);
			h.cmd = ntohl(h.cmd);

			if (m_buf.size() < h.len)
			{
				break;
			}

			SDK_LOG(LOG_LEVEL_TRACE, "cid=%s, recvLoop cmd=%d, len=%d", m_cid.c_str(), h.cmd, h.len);
			m_buf.read(NULL, sizeof(h));
			std::string body;
			body.resize(h.len - sizeof(h));
			m_buf.read((uint8*)body.data(), h.len - sizeof(h));
			handlePacket(h, body);
		}
		return 0;
	}
	int32 CliConn::send_(const std::string& m)
	{
		if (m.empty() || getfd() == INVALID_SOCKET)
		{
			return -1;
		}
		int32 ack = 0;
		int32 ret = tcp_send(getfd(), m.data(), m.size(), &ack);
		if (ret <= 0)
		{
			SDK_LOG(LOG_LEVEL_ERROR, "cid=%s, CliConn::send fail",m_cid.c_str());
			closefd();
			return -1;
		}
		return ret;
	}

	int32 CliConn::handlePacket(const head& h, const std::string& body)
	{
		int32 ret;
		switch (h.cmd)
		{
		case LOGIN_RESP:
			ret = handleLoginResponse(body);
			break;
		case PUSH_REQ:
			ret = handleServiceRequest(body);
			break;
		}
		return ret;
	}
	int32 CliConn::handleLoginResponse(const std::string& resp)
	{
		SmartOp op(NULL);
		popTimer(LOGIN_OP_SN, op);

		LoginResponse lgresp;
		if (!lgresp.ParseFromArray(resp.data(), resp.size()))
		{
			SDK_LOG(LOG_LEVEL_ERROR, "cid=%s, handleLoginResponse parse probuf error", m_cid.c_str());
			return GIM_PROBUF_FORMAT_ERROR;
		}
		int32 status = lgresp.status();
		if (status >= 0)
		{
			m_sessid = lgresp.sessid();
			setStatus(STATUS_LOGIN, status, true);

			//keepalive op
			addKeepaliveTimer();

			return 0;
		}
		else
		{
			m_svrlist.pop_back();
			if (m_svrlist.empty())
			{
				SDK_LOG(LOG_LEVEL_ERROR, "cid=%s, handleLoginResponse fail ret=%d", m_cid.c_str(), status);
				OnLoginFail(status);
			}
			else
			{
				setStatus(STATUS_DISCONNECT, 0, false);
				connectAndLogin();
			}
		}
		return 0;
	}
	int32 CliConn::handleServiceRequest(const std::string& reqbody)
	{
		static int g_req_count = 0;
		g_req_count++;
		PushRequest svreq;
		if (!svreq.ParseFromArray(reqbody.data(), reqbody.size()))
		{
			SDK_LOG(LOG_LEVEL_ERROR, "cid=%s, handleServiceRequest parse fail", m_cid.c_str());
			return GIM_PROBUF_FORMAT_ERROR;
		}
		SDK_LOG(LOG_LEVEL_TRACE, "cid=%s, CliConn::handleServiceRequest ,count=%d, size=%d, payload=%s",
			m_cid.c_str(), g_req_count, svreq.payload().size(), svreq.payload().c_str());

		Json::Value jev;

		jev["cid"] = getCid();
		jev["evtype"] = GIM_EVENT_PUSH;
		jev["sn"] = svreq.sn();
		jev["payload"] = svreq.payload();
		publish(Json::FastWriter().write(jev));

		PushResponse resp;
		resp.set_sn(svreq.sn());
		resp.set_status(0);
		resp.set_to_sessid(svreq.from_sessid());
		resp.set_from_sessid(svreq.to_sessid());

		std::string respbody;
		resp.SerializeToString(&respbody);
		sendPacket(PUSH_RESP, respbody);
		return 0;
	}

	int32 CliConn::addKeepaliveTimer()
	{
		SmartOp op(new KeepaliveOp(getCid(), m_keepalive_timeout));
		addTimer(op->getSN(), op.get());
		return 0;
	}
	int32 CliConn::addTimer(const std::string& id, Op* sp)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "CliConn::addTimer op=%s", id.c_str());
		m_timers.insert(TimerList::value_type(TimerKey(id, sp->getTimeout()), SmartOp(sp)));
		return 0;
	}
	int32 CliConn::popTimer(const std::string& id, SmartOp& sp)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "cid=%s, CliConn::popTimer sp=%s", m_cid.c_str(), id.c_str());
		for (TimerList::reverse_iterator rit = m_timers.rbegin(); rit != m_timers.rend(); ++rit)
		{
			if (rit->first.id == id)
			{
				sp.reset(rit->second.get());
				m_timers.erase((++rit).base());
				return 0;
			}
		}
		return -1;
	}
	int32 CliConn::processTimers(const struct timeval& tnow, struct timeval& tv)
	{
		//SDK_LOG(LOG_LEVEL_TRACE, "cid=%s, CliConn::processTimers timer num=%d", m_cid.c_str(), m_timers.size());
		std::vector<SmartOp> ops;
		for (TimerList::iterator it = m_timers.begin(); it != m_timers.end();)
		{
			TimerKey key = it->first;
			if (tv_cmp(key.deadline, tnow) > 0)
			{
				timeval tvtemp = tv_diff(key.deadline, tnow);
				if (tv_cmp(tv, tvtemp) > 0)
				{
					tv = tvtemp;
				}
				break;
			}
			else
			{
				//SDK_LOG(LOG_LEVEL_TRACE, "op %s time out", key.id.c_str());
				ops.push_back(it->second);
				m_timers.erase(it++);
			}
		}

		for (std::vector<SmartOp>::iterator it = ops.begin(); it != ops.end(); it++)
		{
			if (it->get())
			{
				it->get()->OnTimeout(this);
			}
		}


		return 0;
	}
	int32 CliConn::keepalive()
	{
		SDK_LOG(LOG_LEVEL_TRACE, "cid=%s keepalive, %d", m_cid.c_str(), m_keepalive_timeout);
		if(ef::gettime_ms() - m_active_time > m_keepalive_timeout * 2 * 1000){
			SDK_LOG(LOG_LEVEL_ERROR, "cid=%s, keepalive timeout", m_cid.c_str());
			closefd();
			setStatus(STATUS_DISCONNECT, GIM_TIMEOUT,true);
			asynDestroy();
			return 0;
		}
		addKeepaliveTimer();
		return sendPacket(KEEPALIVE_REQ, "");
	}
	int32 CliConn::sendPacket(int32 cmd, const std::string& body)
	{
		SDK_LOG(LOG_LEVEL_TRACE, "sendPacket cmd=%d", cmd);
		int32 ret = 0;
		std::string packet;
		head h;
		h.magic = 0x20140417;
		h.cmd = cmd;
		constructPacket(cmd, body, packet);
		ret = send_(packet);
		return ret;
	}
	int32 CliConn::constructPacket(int32 srvcmd, const std::string& body, std::string& respbuf)
	{
		head rh;
		respbuf.reserve(sizeof(head)+body.size());
		rh.cmd = htonl(srvcmd);
		rh.magic = htonl(MAGIC_NUMBER);
		rh.len = htonl(sizeof(rh)+body.size());
		respbuf.append((char*)&rh, sizeof(rh));
		respbuf.append(body);
		return 0;
	}
}
