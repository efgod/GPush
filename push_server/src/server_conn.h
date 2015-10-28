#ifndef __SERVER_CONN_H__
#define __SERVER_CONN_H__

#include "logic_common.h"
#include "net/ef_client.h"
#include "gpush.pb.h"

namespace gim{

class PushServer;
class PushRequest;

class SvCon:public Client
{
public:
	enum{
		STATUS_INIT = 0,
		STATUS_LOGIN = 1,
		CHECK_TIMER = 1,
	};

	SvCon():m_status(STATUS_INIT),
		m_serv(NULL){
	}

	virtual	~SvCon();

	void setConnSvID(int consvid){
		m_con_serv_id = consvid;	
	}

	int getConnSvID() const{
		return m_con_serv_id;
	}

	int getSvType() const;

	int getSvID() const;

	void setPushServer(PushServer* s){
		m_serv = s;
	}
	
	PushServer* getPushServer(){
		return m_serv;
	}

	const std::string& getSessID() const{
		return m_sessid;
	}

	virtual int onCreate(ef::EventLoop* l);
	virtual int onConnected();
	virtual int onDisconnected();
	virtual int keepAlive();
	virtual int handlePacket(const std::string& req);

	// < 0, error, = 0, wait more, > 0, recv whole pack
	virtual int checkPackLen();


	int sendPushRequest(const PushRequest& req);


protected:

private:

	int doRegister();
	int handlePushResponse(const std::string& resp);
	int handleRegisterResponse(const std::string& resp);

	int m_status;
	int m_con_serv_id;
	std::string m_sessid;
	PushServer* m_serv;
};


};//end of namespace ef

#endif
