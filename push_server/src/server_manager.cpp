#include "server_manager.h"
#include "logic_common.h"
#include "server_conn.h"
#include "push_server.h"
#include "list_watcher.h"
#include "gpush.pb.h"
#include "net/ef_server.h"
#include "net/ef_event_loop.h"
#include "net/ef_operator.h"


#include <sstream>


namespace gim{

using namespace std;
using namespace ef;

	int Svlist::addServer(int id, SvCon* con){
		m_svcons[id] = con;
		return 0;
	}

	int Svlist::delServer(int id){
		m_svcons.erase(id);
		return 0;
	}	

	Svlist::Svlist(PushServer* s, int kpms, ef::EventLoop* l, DtSrc* src):
		m_sv(s), m_keepalivems(kpms), m_evlp(l), m_loader(src){
	}


	Svlist::~Svlist(){
		//do not delete sesscache, because it will be delete on thread exit;
	}
	
	int Svlist::connectIPArray(SvCon* c, const vector<string>& a, 
		int port){
	
		int ret = 0;
	
		if(a.size() <= 0){
			return -1;
		}
	
		for(Json::UInt i = 0; i < a.size(); ++i){
			
			ret = c->connectTo(a[i], port);
			if(ret >= 0){
				c->setAddr(a[i], port);
				break;	
			}
		}
	
		return ret;
	}
	
	
	int Svlist::connectServer(const ServerStatus& s){
		int ret = 0;
		SvCon* c = new SvCon();
		MainConfig conf = m_sv->getConfig();

		c->setPushServer(m_sv);

		c->setConnSvID(s.ID);
		c->setReconnectSpan(0);
		c->setKeepAliveSpan(m_keepalivems);
		
		int port = s.Ports[0].Port;
		vector<string> localIPs;
		vector<string> publicIPs;

		for(size_t i = 0; i < s.IPs.size(); ++i){
			if(isLocalIP(s.IPs[i])){
				localIPs.push_back(s.IPs[i]);
			}else{
				publicIPs.push_back(s.IPs[i]);
			}
		}
	
		ret = connectIPArray(c, localIPs, port);

		if(ret < 0){
			ret = connectIPArray(c, publicIPs, port);
		}
		
		m_evlp->asynAddConnection(c);
		
		return 0;
	}

	SvCon* Svlist::getServer(int id){
		map<int, SvCon*>::iterator it = m_svcons.find(id);
		if(it != m_svcons.end()){
			return it->second;
		}

		return NULL;
	}


	int Svlist::checkServers(){
		bool l = m_loader.loadData();
		cout << "SvManager, this:" << this << ", l:" << m_evlp << endl;
		if(!l){
			return 0;
		}

		StatusMap smap = m_loader.getData();
		StatusMap::iterator itor = smap.begin();
		for(; itor != smap.end(); ++itor){
			if(getServer(itor->first)){
				continue;
			}
			connectServer(itor->second);
		}
		
		return 0;
	}

	class CheckConnectServerOp: public NetOperator{
	public:
		virtual int32 process(EventLoop *l){
			cout << "CheckConnectServerOp, l:" << l << endl;
			Svlist* d = (Svlist*)l->getObj();
			return d->checkServers();
		}
	private:
	};
	
	

	SvManager::SvManager(PushServer* lgsv, Server* sv):m_lgsv(lgsv), 
		m_sv(sv), m_keepalivems(30000), m_svlstob(NULL){
	}
	
	SvManager::~SvManager(){
		
		for(size_t i = 0; i < m_svlsts.size(); ++i){
			delete m_svlsts[i];
			m_svlsts[i] = NULL;
		}
		
		if(m_svlstob){
			delete m_svlstob;
		}
	}

	int SvManager::init(ZKClient* zkc, const string& svpath, int keepalivems){
		m_svlstob = new SvListWatcher<SvManager>(this, &SvManager::onSvListChange);
		int ret = m_svlstob->init(zkc, svpath);
		cout << "SvManager::init, path:" << svpath << ", ret:" << ret << std::endl;
		if(ret < 0){
			return -12;
		}
		
		int c = m_sv->getEventLoopCount();
		
		for(int i = 0; i< c; ++i){
			EventLoop& l = m_sv->getEventLoop(i);			
			Svlist* dsp = new Svlist(m_lgsv, m_keepalivems, &l, &m_datasrc);

			l.setObj(dsp, NULL, NULL);
			m_svlsts.push_back(dsp);			
		}

		SvStatusMap m;
		ret = m_svlstob->getChildren(m);
		if(ret < 0){
			return ret;
		}	

		ret = onSvListChange(m);

		return 0;
	}


	int SvManager::onSvListChange(const SvStatusMap& notify){
		const MainConfig& conf = m_lgsv->getConfig();
		SvStatusMap::const_iterator itor = notify.begin();
		StatusMap m;
		
		for(; itor != notify.end(); ++itor){
			const ServerStatus& s2 = itor->second;
			ServerStatus s1;
			
			s1 = s2;
			s1.Ports.clear();
			size_t i = 0;
			for(; i < s2.Ports.size(); ++i){
				if(s2.Ports[i].MinType <= conf.Type 
					&& s2.Ports[i].MaxType > conf.Type){
					s1.Ports.push_back(s2.Ports[i]);
				}
			}
			if(!s1.Ports.size()){
				continue;
			}
			m[atoi(itor->first.c_str())] = s1;
		}
		
		m_datasrc.setData(m);
		notifySvlistChange();
		
		return 0;
	}

	int SvManager::notifySvlistChange(){	
		int c = m_sv->getEventLoopCount();
		cout << "notifySvlistChange, svman:" << this << endl;	
		for(int i = 0; i< c; ++i){

			EventLoop& l = m_sv->getEventLoop(i);
			CheckConnectServerOp* op = new CheckConnectServerOp();
			l.addAsynOperator(op);
		}

		return  0;
	}

	
};

