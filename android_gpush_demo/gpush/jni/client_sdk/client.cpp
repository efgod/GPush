#include "client.h"
#include "ops.h"
#include "base/ef_utility.h"
namespace gim
{
	Client::Client()
	:m_keepalive_timeout(KEEPALIVE_TIMEOUT),
	 m_sn(0){

	}
	void Client::setKeepaliveTimeout(int second){
		m_keepalive_timeout = second;
	}
	std::string Client::getSN()
	{
		return itostr(m_sn++);
	}
	int Client::init()
	{
		m_sn = gettime_ms();
		m_evlp.setMsgCb(eventLoopMsgRoutine, (void*)this);
		m_evlp.startLoop();
		return 0;
	}

	int Client::login(const std::string& srvip, int32 srvport, const std::string& cid, const std::string& version)
	{
		LoginOp* op = new LoginOp(cid);
		op->init(srvip, srvport, version, m_keepalive_timeout);
		return m_evlp.asynAddOp((Op*)op);
	}
	int Client::stop()
	{
		m_evlp.asynStop();
		return 0;
	}
	int Client::disconnect(const std::string& cid)
	{
		DisconnectOp* op = new DisconnectOp(cid);
		return m_evlp.asynAddOp((Op*)op);
	}

	int Client::eventLoopMsgRoutine(void* cli, const std::string& msg)
	{
		return cli ? ((Client*)cli)->handleMessage(msg) : -1;
	}
	int Client::handleMessage(const std::string& msg)
	{
		return 0;
	}
}
