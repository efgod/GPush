#ifndef _CLIENT_H_
#define  _CLIENT_H_

#include "eventloop.h"

namespace gim
{
	class Client
	{
	public:
		Client();
		~Client(){};

		std::string getSN();

		int init();
		void setKeepaliveTimeout(int second);
		int login(const std::string& srvip, int32 srvport, const std::string& cid, const std::string& version);
		int stop();
		int disconnect(const std::string& cid);
	public:
		virtual int handleMessage(const std::string& msg);
	private:
		static int eventLoopMsgRoutine(void* cli, const std::string& msg);
		EventLoop m_evlp;
		int64 m_sn;
		int m_keepalive_timeout;
	};
}

#endif //_CLIENT_H_
