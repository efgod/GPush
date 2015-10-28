#include "type_map.h"
#include <algorithm>
#include "client_conn.h"
#include "net/ef_event_loop.h"
#include "msg_head.h"
#include "err_no.h"
#include "gpush.pb.h"

namespace gim{

using namespace ef;
using namespace std;

SvList::SvList():m_cnt(0){
	mutexInit(&m_cs);		
}

SvList::~SvList(){
	mutexDestroy(&m_cs);
}

int SvList::addServer(CliCon* c){
	AutoLock l(&m_cs);
	m_svs.push_back(c);
	return 0;	
}

int SvList::delServer(CliCon* c){
	AutoLock l(&m_cs);
	vector<CliCon*>::iterator it = std::find(m_svs.begin(), m_svs.end(), c);	
	if(it != m_svs.end()){
		m_svs.erase(it);
	}
	return 0;
}

int SvList::transRequest(const PushRequest& req){

	if(!m_svs.size()){
		return NO_SERVICE; 
	}

	AutoLock l(&m_cs);

	CliCon* c = m_svs[m_cnt % m_svs.size()];

	if(!c){
		return NO_SERVICE;
	}

	string newreq;

	req.SerializeToString(&newreq);
	SendMsgOp *op = new SendMsgOp(c->getId(), PUSH_REQ, newreq);
	int ret = c->getEventLoop()->addAsynOperator(op);
	if(ret < 0){
		return INNER_ERROR;
	}

	++m_cnt;
	if(m_cnt < 0){
		m_cnt = 0;
	}

	return 0;
}


SvList TypeMap::m_svlsts;


int TypeMap::addServer(CliCon* c){
	
	int ret = m_svlsts.addServer(c);

	return ret;
}

int TypeMap::delServer(CliCon* c){
	
	int ret = m_svlsts.delServer(c);

	return ret;
}


int TypeMap::transRequest(const PushRequest& req){

	int ret = m_svlsts.transRequest(req);
	
	return ret;
}
	
};
