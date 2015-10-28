#include "err_no.h"
#include "msg_head.h"
#include "listen_conn.h"
#include "push_server.h"
#include "gpush.pb.h"
#include "logic_log.h"
#include "sess_cache.h"
#include "server_conn.h"
#include "server_manager.h"
#include "base/ef_base64.h"
#include <iostream>

namespace gim{

using namespace std;


int genSessID(int svid, int conid,
	const std::string& n, std::string& dn){

	std::string buf;
	buf.resize(8);

	char* p = (char*)buf.data();

	*(int32*)p = svid;
	*(int32*)(p + sizeof(int32)) = conid;

	dn = ef::base64Encode(buf) + n;
	return 0;
}

ListenCon::~ListenCon(){
}


int ListenCon::getSvID() const{
	return m_serv->getConfig().ID;
}

int ListenCon::handlePacket(const string& req){

	std::cout << "ListenCon::handlePacket" << std::endl;
	int ret = 0;
	head h;
	h = *(head*)req.data();
	h.cmd = htonl(h.cmd);
	h.len = htonl(h.len);
	h.magic = htonl(h.magic);

	switch(h.cmd){
	case JSON_PUSH_REQ:
		ret = handlePushReqeust(req.substr(sizeof(h)));
		break;
	default:
		break;	
	}

	return ret;
}

int ListenCon::sendJsonResponse(const Json::Value& v){
	head resph;
	resph.cmd = JSON_PUSH_RESP;
	resph.magic = MAGIC_NUMBER;

	string msg;
	constructPacket(resph, v.toStyledString(), msg);


	return sendMessage(msg);
}

int ListenCon::handlePushReqeust(const string& req){

	Json::Value v;
	Json::Reader r;
	Json::Value respv;

	respv["status"] = 0;

	if(!r.parse(req, v)){
		respv["status"] = -1;
		respv["message"] = "parse req fail";
		sendJsonResponse(respv);
		logicInfo << "<action:handlePushReqeust> <req:"<< req 
			<< "> <resp:" << respv.toStyledString() << ">";
		return -1;
	}

	respv["sn"] = v["sn"];

	PushRequest preq;
	preq.set_type(v["type"].asInt());
	preq.set_payload(v["payload"].asString());
	preq.set_sn(v["sn"].asString());

	SessCache* s = SessCacheService::instance()->getSessCache();
	if (!s) {
		respv["status"] = -2;
		respv["message"] = "get sess cache fail";
		sendJsonResponse(respv);
		logicInfo << "<action:sendJsonResponse> <req:" << req 
			<< "> <resp:" << respv.toStyledString() << ">";
		return -2;
	}

	if(!v["to_id"].isString()){
		respv["status"] = -4;
		respv["message"] = "to_id is empty";
		sendJsonResponse(respv);
		logicInfo << "<action:sendJsonResponse> <req:" << req 
			<< "> <resp:" << respv.toStyledString() << ">";
		return -4;
	}

	std::string id = v["to_id"].asString();

	std::vector<Sess> sv;
	s->getSession(id, sv);
	if (sv.empty()) {
		respv["status"] = -3;
		respv["message"] = "user offline";
		sendJsonResponse(respv);
		logicInfo << "<action:sendJsonResponse> <req:" << req 
			<< "> <resp:" << respv.toStyledString() << ">";
		return -3;
	}

	int count = 0;
	for (unsigned int n = 0; n<sv.size(); n++){
		Sess&  ss = sv[n];
		ss.consvid();

		int ret = sendToServer(ss.consvid(), preq);
		if (ret >= 0){
			++count;
		}

		logicInfo << "<action:sendJ2C> <sn:" << preq.sn() 
			<< "> <id:" << id << "> <type:" << preq.type() << "> <ret:" << ret
			<< "> <req_payload:" << preq.payload() << ">";
	}

	respv["count"] = count;
	sendJsonResponse(respv);
	logicInfo << "<action:sendJsonResponse> <req:" << req << "> <resp:" 
		<< respv.toStyledString() << ">";
	return count;
}

int ListenCon::sendToServer(int svid, const PushRequest& preq){
	Svlist* m = (Svlist*)getEventLoop()->getObj();

	SvCon* s = m->getServer(svid);

	if(!s){
		return -1;
	}

	return s->sendPushRequest(preq);
}

int ListenCon::checkPackLen(){
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

int ListenCon::onCreate(ef::EventLoop* l){
	addNotify(l, READ_EVENT);
	return 0;
}

Connection* LConFactory::createConnection(EventLoop* l,
	int32 fd, const std::string& addr,int32 port){
	ListenCon* c = new ListenCon();
	return c;
}

}
