#ifndef _CLIENT_H_
#define  _CLIENT_H_

#include <string>

namespace gim
{
	class EventLoop;
	class Client
	{
	public:
		Client();
		~Client();

		int init();
		void setKeepaliveTimeout(int second);
		int login(const std::string& srvip, int srvport, const std::string& cid, const std::string& version);
		int stop();
		int disconnect(const std::string& cid);
	public:
		virtual int handleMessage(const std::string& msg);
	private:
		static int eventLoopMsgRoutine(void* cli, const std::string& msg);
		EventLoop* m_evlp;
		int m_keepalive_timeout;
	};
}

#endif //_CLIENT_H_
