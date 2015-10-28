#ifndef __LISTEN_CONN_H__
#define __LISTEN_CONN_H__


#include "logic_common.h"
#include "net/ef_connection.h"
#include "net/ef_acceptor.h"
#include <json/json.h>

namespace gim{

class PushServer;
class PushRequest;

class ListenCon:public Connection 
{
public:

	ListenCon():m_serv(NULL){
	}

	virtual	~ListenCon();

	int getSvID() const;

	void setPushServer(PushServer* s){
		m_serv = s;
	}
	
	PushServer* getPushServer(){
		return m_serv;
	}

	virtual int handlePacket(const std::string& req);

	// < 0, error, = 0, wait more, > 0, recv whole pack
	virtual int checkPackLen();

	virtual int onCreate(ef::EventLoop* l);
protected:

private:

	int handlePushReqeust(const std::string& req);
	int sendJsonResponse(const Json::Value& v);
	int sendToServer(int svid, const PushRequest& req);

	PushServer* m_serv;
};



class LConFactory:public ConnectionFactory{
public:
	LConFactory(PushServer* s = NULL):m_s(s){
	}

	virtual ~LConFactory(){};
	virtual Connection* createConnection(EventLoop* l,
		int32 fd, const std::string& addr,int32 port);

private:
	PushServer* m_s;	
};

}

#endif/*__LISTEN_CONN_H__*/
