#include "err_no.h"
#include "msg_head.h"
#include "logic_log.h"
#include "server_conn.h"
#include "server_manager.h"
#include "push_server.h"
#include "base/ef_utility.h"
#include "base/ef_statistic.h"

namespace gim{

using namespace ef;
using namespace std;

int SvCon::onConnected(){

	return doRegister();	
}

SvCon::~SvCon(){
	Svlist* d = (Svlist*)getEventLoop()->getObj();
	d->delServer(m_con_serv_id);
}

int SvCon::onCreate(ef::EventLoop* l){
	Client::onCreate(l);
	return 0;
}

int SvCon::onDisconnected(){
	Svlist* d = (Svlist*)getEventLoop()->getObj();
	d->delServer(m_con_serv_id);

	logicError << "<action:server_disconnect> "
		"<event_loop:" << getEventLoop() << "> <conid:"
		<< getId() << "> <con_serv_id:" << m_con_serv_id
		<< "> <sessid:" << m_sessid << ">"; 	
	return 0;
}

int SvCon::keepAlive(){
	int ret = 0;
	head h;
	h.magic = MAGIC_NUMBER;
	h.cmd = KEEPALIVE_REQ;	
	string req;	
	constructPacket(h, "", req);
	logicError << "<action:keep_alive> "
		"<event_loop:" << getEventLoop() << "> <conid:"
		<< getId() << "> <con_serv_id:" << m_con_serv_id
		<< "> <sessid:" << m_sessid << ">";
	ret = sendMessage(req);
	return ret;
}

int SvCon::getSvType() const{
	return m_serv->getConfig().Type;
}

int SvCon::getSvID() const{
	return m_serv->getConfig().ID;
}



int SvCon::doRegister(){
	int ret = 0;
	LoginRequest lgr;
	lgr.set_type(getSvType());
	lgr.set_id(ef::itostr(getSvID()));
	string reqbody;
	lgr.SerializeToString(&reqbody);	
	head h;
	h.cmd = LOGIN_REQ;
	h.magic = MAGIC_NUMBER;
	string req;
	constructPacket(h, reqbody, req);	
	logicError << "<action:server_register> "
		"<event_loop:" << getEventLoop() << "> <conid:"
		<< getId() << "> <con_serv_id:" << m_con_serv_id
		<< ">";
	ret = sendMessage(req);
	return ret;
}

int SvCon::handleRegisterResponse(const string& resp){
	int ret = 0;
	ef::TimeRecorder t("SvCon::handleRegisterResponse");
	LoginResponse lgresp;	
	lgresp.ParseFromString(resp);
	if(lgresp.status() < 0){
		//reg fail
		disconnect();

		logicError << "<action:server_register_resp> " 
			"<event_loop:" << getEventLoop() << "> <conid:"
			<< getId() << "> <con_serv_id:" << m_con_serv_id
			<< "> <status:" << lgresp.status() << ">";
		return ret;
	}

	m_status = STATUS_LOGIN; 
	m_sessid = lgresp.sessid();	

	Svlist* d = (Svlist*)getEventLoop()->getObj();
	d->addServer(getConnSvID(), this);	

	logicError << "<action:server_register_resp> " 
		"<event_loop:" << getEventLoop() << "> <conid:"
		<< getId() << "> <con_serv_id:" << m_con_serv_id
		<< "> <status:0> <sessid:" << m_sessid << ">";  

	return ret;
}
int SvCon::handlePacket(const string& req){
	int ret = 0;
	head h;
	h = *(head*)req.data();
	h.cmd = htonl(h.cmd);
	h.len = htonl(h.len);
	h.magic = htonl(h.magic);

	switch(h.cmd){
	case LOGIN_RESP:
		ret = handleRegisterResponse(req.substr(sizeof(h)));
		break;		
	case PUSH_RESP:
		ret = handlePushResponse(req.substr(sizeof(h)));
		break;	
	default:
		logicError << "<action:handlePacket> "
			"<event_loop:" << getEventLoop() << "> <conid:"
			<< getId() << "> <con_serv_id:" << m_con_serv_id
			<< "> <cmd:" << h.cmd << ">";
	}

	return ret;
}

int SvCon::handlePushResponse(const string& resp){
	PushResponse presp;
	if(!presp.ParseFromString(resp)){
		logicError << "<action:handlePushResponse> "
			"<event_loop:" << getEventLoop() << "> <conid:"
			<< getId() << "> <con_serv_id:" << m_con_serv_id
			<< "> <status:ParsingFail>";
		return -1;
	}
	logicInfo << "<action:handlePushResponse> <from_sessid:"
		<< presp.from_sessid() << "> <to_sessid:"
		<< presp.to_sessid() << "> <sn:" << presp.sn() 
		<< "> <status:" << presp.status() << ">";
	return 0;
}

int SvCon::sendPushRequest(const PushRequest& req){
	int ret = 0;
	string body;

	PushRequest req1 = req;

	req1.set_from_sessid(m_sessid);

	req1.SerializeToString(&body);

	head resph;
	resph.cmd = PUSH_REQ;
	resph.magic = MAGIC_NUMBER;

	string msg;
	constructPacket(resph, body, msg);

	ret = sendMessage(msg);

	return ret;
}

int SvCon::checkPackLen(){
	int ret = 0;

	head h;

	if(bufLen() < (int)sizeof(head)){
		return 0;
	}

	peekBuf((char*)&h, sizeof(head));
	h.magic = htonl(h.magic);
	h.len = htonl(h.len);

	if(h.len < (int)sizeof(h) 
		|| h.len > 1024 * 1024){
		ret = -1;
	} else if(h.len <= bufLen()){
		ret = h.len;
	}

	if(ret <= 0 && bufLen() > 0){
		logicError << "<action:client_check_pack> " 
			"<event_loop:" << getEventLoop() << "> <conid:" 
			<< getId() << "> <buflen:" << bufLen() 
			<< "> <status:packet_not_full>";
	}

	return ret;
}

}
