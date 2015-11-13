#include "client_conn.h"
#include "err_no.h"
#include "msg_head.h"
#include "common.h"
#include "type_map.h"
#include "sess_cache.h"
#include "ef_crypt.h"
#include "connect_server.h"
#include "base/ef_base64.h"
#include "base/ef_aes.h"
#include "base/ef_md5.h"
#include "base/ef_hex.h"
#include "base/ef_utility.h"
#include "base/ef_statistic.h"
#include "net/ef_event_loop.h"
#include "gpush.pb.h"

namespace gim{

using namespace ef;
using namespace std;

#define SVID m_serv->getConfig().ID

#define ALogTrace(a) logTrace(a) << "<svid:" << (SVID) \
	<< "> <addr:" << getIp() << ":" \
	<< getPort() << "> <timestamp:" << gettime_ms() << "> " 
#define ALogError(a) logError(a) << "<svid:" << (SVID) \
	<< "> <addr:" << getIp() << ":" \
	<< getPort() << "> <timestamp:" << gettime_ms() << "> " 



int CDispatcher::dispatchConnection(EventLoop*l, Connection* c){
	int id = c->getId();
	int idx = 0;	

	idx = m_conf->StartThreadIdx + id % m_conf->ThreadCnt;	
		
	EventLoop& lp = m_s->getEventLoop(idx);
	id = getConnectionId(idx, id);
	c->setId(id);
	lp.asynAddConnection(c);
	return 0;		
}


CliCon::~CliCon(){
	delSession();
	atomicDecrement32(&cnt);
}


volatile int CliCon::cnt = 0;
volatile int64 CliCon::req_cnt = 0;
volatile int64 CliCon::resp_cnt = 0;

int CliCon::delSession(){
	TimeRecorder t(__FUNCTION__);

	time_t n = time(NULL);
	SessCache* c = NULL;

	if(m_sess.type() == 0){
		goto exit;
	}

	if(STATUS_LOGIN != m_status){
		goto exit;
	}

	if(m_sess.type() > 0){
		TypeMap::delServer(this);
		goto exit;
	}

	c = SessCacheService::instance()->getSessCache();		

	if(c){
		c->delSession(m_sess);
	}
exit:
	ALogError("ConnectServer") << "<action:client_logout> <id:" 
		<< m_sess.id() << "> <sessid:" << m_sess.sessid()
		<< "> <login_timestamp:" << m_login_time
		<< "> <online_time:" << n - m_login_time 
		<< ">";
	return 0;
}


int CliCon::onCreate(EventLoop* l){
	addNotify(l, READ_EVENT);	
	if(m_conf->AliveMs){
		startTimer(CHECK_TIMER, m_conf->AliveMs);
	}
	return 0;
}

int CliCon::handleTimer(EventLoop* l, int id){
	time_t n = time(NULL);
	switch(id){
	case CHECK_TIMER:
		if(n - m_sess.lasttime() > m_conf->AliveMs/1000){
			ALogError("ConnectServer") << "<id:" << m_sess.id() 
				<< "> <sessid:" << m_sess.sessid()
				<< "> <status:timeout>";
			safeClose();
			return 0;
		}
	
		startTimer(CHECK_TIMER, m_conf->AliveMs);
		break;
	}

	return 0;
}


int CliCon::updateSession(){
	TimeRecorder t(__FUNCTION__);

	time_t n = time(NULL);
	m_sess.set_lasttime(n);	

	//type is 0 or > 0, return
	if(m_sess.type() >= 0){
		return 0;
	}

	SessCache* c = SessCacheService::instance()->getSessCache();		
	if(!c){
		return -1;
	}

	return c->setSession(m_sess);
}



int CliCon::checkType(int type){
	//do not check rpc_client;
	if(!type)
		return 0;
	if(type < m_conf->MinType || type >= m_conf->MaxType){
		return -1;
	}
	return 0;
}

int CliCon::handleLoginRequest(const head& h, const string& req, 
	string& resp){
	TimeRecorder t("CliCon::handleLoginRequest");
	int ret = 0;
	int i = 0;
	LoginRequest lgreq;
	LoginResponse lgresp;	
	string tk;
	string sessid;
	string enctk;

	if(!lgreq.ParseFromArray(req.data(), req.size())){
		ret = INPUT_FORMAT_ERROR;
		goto exit;
	}
	//if login, return the sessid
	if(m_status == STATUS_LOGIN){
		goto exit;
	}

	m_login_time = time(NULL);	
	m_sess.set_id(lgreq.id());
	m_sess.set_type(lgreq.type());
	m_sess.set_consvid(m_serv->getConfig().ID);
	m_sess.set_version(lgreq.version());

	ret = checkType(lgreq.type());

	if(ret < 0){
		ret = INVALID_TYPE;
		goto exit;
	}

	decorationName(m_serv->getConfig().ID, getId(), lgreq.id(), sessid);
	m_sess.set_sessid(sessid);


	for(; i < lgreq.kvs_size(); ++i){
		*(m_sess.add_kvs()) = lgreq.kvs(i);
	}

	if(lgreq.type() > 0){
		TypeMap::addServer(this);
	}
	
	ret = updateSession();

	if(ret < 0){
		ret = CREATE_SESSION_FAIL;
	}
exit:
	if(ret < 0){
		lgresp.set_status(ret);
	}else{
		m_status = STATUS_LOGIN;
		lgresp.set_status(0);
	}

	lgresp.set_sessid(m_sess.sessid());

	lgresp.SerializeToString(&resp);
	ALogError("ConnectServer") << "<action:client_login> <id:" 
		<< m_sess.id() << "> <version:" << lgreq.version() 
		<< "> <type:" << lgreq.type()
		<< "> <token:" << lgreq.token()  
		<< "> <sessid:" << m_sess.sessid() << "> <status:"
		<< ret << "> <errstr:" << getErrStr(ret) << ">";
	return ret;
}


int CliCon::checkReqQue(int reqcnt, int respcnt){
	m_req_cnt += reqcnt;
	m_resp_cnt += respcnt;
	if(m_sess.type() <= 0 || m_conf->MaxReqQueSize <= 0){
		return 0;
	}
	if(m_req_cnt - m_resp_cnt >= m_conf->MaxReqQueSize){
		TypeMap::delServer(this);
		m_busy = true;
	}

	if(m_req_cnt - m_resp_cnt < m_conf->MaxReqQueSize / 2 && m_busy){
		TypeMap::addServer(this);
		m_busy = false;
	}

	return 0;
}

int CliCon::getConFromSessid(const string& sessid, 
	EventLoop*& l, int& conid, string& orgdata){
	int ret = 0;
	int svid = 0;
	int evlpid = 0;

	ret = getDecorationInfo(sessid, svid, conid, orgdata);

	if(ret < 0 || svid != m_serv->getConfig().ID){
		ret = INVALID_SESSION_ID;
		goto exit;	
	}

	evlpid = getEventLoopId(conid);
	
	if(evlpid < 0 || evlpid >= m_serv->getConfig().ThreadCount){
		ret = INVALID_SESSION_ID;
		goto exit;
	}

	l = &(m_serv->getEventLoop(evlpid));

exit:
	return ret;
}

int CliCon::sendCmd(int cmd, const string& body){
	string req_enc;
	string msg;
	head h;
	h.cmd = cmd;
	h.magic = MAGIC_NUMBER;
	constructPacket(h, body, msg);
	return sendMessage(msg);
}

int CliCon::handlePushRequest(const head& h, const string& req, 
	string& resp){
	TimeRecorder t("CliCon::handlePushRequest");

	int ret = 0;
	int conid = 0;

	EventLoop* l = NULL;
	NetOperator* op = NULL;

	PushResponse svresp;
	PushRequest svreq;
	string id;

	svresp.set_from_sessid("NULL");
	svresp.set_to_sessid("NULL");
	svresp.set_sn("null");

	if(!svreq.ParseFromArray(req.data(), req.size())){
		ret = INPUT_FORMAT_ERROR;
		goto exit;
	}

	svresp.set_sn(svreq.sn());
	svresp.set_status(0);

	if(type() < 0){
		ret = INVALID_TYPE;	
		goto exit;
	}

	svresp.set_to_sessid(svreq.from_sessid());

	ret = getConFromSessid(svreq.to_sessid(), l, conid, id);
	if(ret < 0){
		ret = INVALID_SESSION_ID;
		goto exit;
	}

	op = new SendMsgOp(conid, PUSH_REQ, req);	
	ret = l->addAsynOperator(op);
	if(ret < 0){
		svresp.set_status(ret);
		ret = 0;
		goto exit;
	}

	ef::atomicIncrement64(&req_cnt);

exit:
	if(ret < 0){
		svresp.set_status(ret);
	}

	if(svresp.status()){
		svresp.SerializeToString(&resp);
	}

	ALogError("ConnectServer") << "<action:service_request> <id:" 
		<< m_sess.id() << "> <version:" << m_sess.version() 
		<< "> <sessid:" << m_sess.sessid() << "> <from_sessid:"
		<< svreq.from_sessid() << "> <sn:" << svresp.sn()
		<< "> <to_sessid:" << svreq.to_sessid()
		<< "> <conid:" << conid << "> <status:" 
		<< svresp.status() << "> <errstr:"
		<< getErrStr(svresp.status()) << ">";
	return ret;
}

int32 CliCon::handlePushResponse(const head& h, 
		const string& req){
	TimeRecorder t("SrvCon::handlePushResponse");
	int32 ret = 0;
	int32 conid = 0;
	string id; 
	PushResponse svresp;
	EventLoop* l = NULL;
	SendMsgOp* op = NULL;

	if(!svresp.ParseFromArray(req.data(), req.size())){
		ret = INPUT_FORMAT_ERROR;	
		goto exit;
	}


	if(!svresp.has_to_sessid() || !svresp.to_sessid().size()){
		ret = MISS_TO_SESSION_ID;
		goto exit;
	}

	//cout << "RESP_TO_SESSID:" << svresp.to_sessid() << endl;

	ret = getConFromSessid(svresp.to_sessid(), l, conid, id);
	if(ret < 0){
		goto exit;
	}
	//???
	op = new SendMsgOp(conid, PUSH_RESP, req);
	ret = l->addAsynOperator(op);
	if(ret < 0){
		svresp.set_status(ret);
		ret = 0;
		goto exit;
	}

	ef::atomicIncrement64(&req_cnt);
exit:
	ALogError("ConnectServer")  
		<< "<action:service_response> <sessid:"
		<< m_sess.sessid() << "> <from_sessid:" 
		<< svresp.from_sessid() << "> <to_sessid:"
		<< svresp.to_sessid() << "> <sn:"
		<< svresp.sn() << "> <status:" << svresp.status() 
		<< "> <errstr:" << getErrStr(ret) << ">";
	if(ret != INPUT_FORMAT_ERROR)
		ret = 0;
	return ret;
}

Connection* CliConFactory::createConnection(EventLoop* l,
		int fd, const string& addr, int port){
	Connection* c = new CliCon(m_serv, m_conf);
	return c;
}


int CliCon::handlePacket(const string& req){
	int ret = 0;
	head h = *(head*)req.data();
	h.cmd = htonl(h.cmd);
	h.len = htonl(h.len);

	string resp;

	try{
		switch(h.cmd){
		case LOGIN_REQ:
			ret = handleLoginRequest(h, req.substr(sizeof(h)), resp);		
			break;
		case PUSH_REQ:
			updateSession();
			ret = handlePushRequest(h, req.substr(sizeof(h)), resp);
			break;
		case PUSH_RESP:
			updateSession();
			ret = handlePushResponse(h, req.substr(sizeof(h)));
			break;
		case KEEPALIVE_REQ:
			updateSession();
			break;	
		default:
			ALogError("ConnectServer") << "<id:" << m_sess.id()
				<< "> <sessid:" << m_sess.sessid() 
				<< "> <status:unknow_cmd>";
			ret = -1;
		}
		if(resp.size() || h.cmd == KEEPALIVE_REQ){	
			ret = sendCmd(h.cmd + 1, resp);
		}
	}catch(exception& e){
		ALogError("ConnectServer") << "<action:client_cmd:" << h.cmd << "> <id:" 
			<< m_sess.id() << "> <version:" << m_sess.version() 
			<< "> <sessid:" << m_sess.sessid() << "> <conid:" 
			<< getId() << "> <status:exception> <what:" << e.what()
			<< ">";
		ret = -1;
	}catch(...){
		ALogError("ConnectServer") << "<action:client_cmd>" << h.cmd << " <id:" 
			<< m_sess.id() << "> <version:" << m_sess.version() 
			<< "> <sessid:" << m_sess.sessid() << "> <conid:" 
			<< getId() << "> <status:exception>";
		ret = -1;
	}
	return ret;
}


int CliCon::checkPackLen(){
	int ret = 0;
	ret = checkLen(*this);
	if(ret <= 0 && bufLen() > 0){
		ALogError("ConnectServer") << "<action:client_check_pack> <id:" 
			<< m_sess.id() << "> <sessid:" << m_sess.sessid() 
			<< "> <conid:" << getId() << "> <buflen:" << bufLen() 
			<< "> <status:packet_not_full>";

	}
	return ret;
}


int SendMsgOp::process(ef::EventLoop *l){
	CliCon* clic = (CliCon*)l->getConnection(m_conid);
	if(!clic){
		return -1;
	}
	return clic->sendCmd(m_cmd, m_body);
}

}
