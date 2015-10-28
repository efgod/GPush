#include <set>
#include <iostream>
#include "sess_cache.h"
#include "redis_group.h"
#include "data_watcher.h"
#include "base/ef_base64.h"

namespace gim{

	RedisSessCache::RedisSessCache(ef::DataSrc<Json::Value>* config) :m_dbg(NULL),
		m_expire(SESSION_DEFAULT_EXPIRE_SECOND),
		m_loader(config){
		m_loader.loadData();
		init(m_loader.getData());
	};
	RedisSessCache::~RedisSessCache(){
		if (m_dbg)
			delete m_dbg;
	}

	int RedisSessCache::init(const Json::Value& config){
		const Json::Value& dbconf = config["DBGroup"];

		if (!dbconf.isObject()){
			return -2001;
		}
		if (m_dbg)
			delete m_dbg;

		m_dbg = new RedisGroup(dbconf);

		const Json::Value& exp = config["Expire"];

		if (exp.isInt()){
			m_expire = exp.asInt();
		}

		return 0;
	}

	inline string sessKey(const std::string& key){
		return "SESS_" + key;
	}

	DBHandle RedisSessCache::getHandle(const std::string &key){
		if (!m_dbg){
			return NULL;
		}

		if (m_loader.loadData()){
			init(m_loader.getData());
		}
		DBHandle h = m_dbg->getHndl(sessKey(key));

		return h;
	}


	int RedisSessCache::getSession(const string &key, vector<Sess> &m){

		DBHandle h = getHandle(key);

		if (!h){
			return -1002;
		}

		time_t n = time(NULL);
		map<string, string> sm;

		int ret = h->hashGetAll(sessKey(key), sm);
		vector<string> oldsessids;
		map<string, string>::iterator it = sm.begin();

		for (; it != sm.end(); ++it){
			Sess s;
			string str = base64Decode(it->second);
			s.ParseFromString(str);

			if (s.lasttime() + m_expire < n){
				oldsessids.push_back(s.sessid());
			}
			else{
				m.push_back(s);
			}
		}

		if (oldsessids.size()){
			h->hashMDel(sessKey(key), oldsessids);
		}

		if (ret == CACHE_NOT_EXIST){
			ret = 0;
		}

		return ret;

	}

	int RedisSessCache::setSession(const Sess &s){

		DBHandle h = getHandle(s.id());

		if (!h){
			return -1002;
		}

		string str;
		const_cast<Sess&>(s).set_lasttime(time(NULL));
		s.SerializeToString(&str);

		int ret = h->hashSet(sessKey(s.id()), s.sessid(), base64Encode(str));
		h->keyExpire(sessKey(s.id()), m_expire);

		return ret;
	}

	int RedisSessCache::delSession(const Sess &s){

		DBHandle h = getHandle(s.id());

		if (!h){
			return -1002;
		}

		int ret = h->hashDel(sessKey(s.id()), s.sessid());

		return ret;
	}

	SessCacheWatcher::SessCacheWatcher()
		:m_zkcfg(NULL)
	{
	}

	SessCacheWatcher::~SessCacheWatcher(){
		if (m_zkcfg)
			delete m_zkcfg;
	}

	int SessCacheWatcher::init(ZKClient* c, const std::string& zkpath){
		m_zkcfg = new JsonWatcher<SessCacheWatcher>(this, &SessCacheWatcher::watchCallBack);
		m_zkcfg->init(c, zkpath);
		m_cfg.setData(m_zkcfg->getJsonData());
		return 0;
	}

	SCCDataSrc* SessCacheWatcher::getDataSrc(){
		return &m_cfg;
	}

	int SessCacheWatcher::watchCallBack(int version, const Json::Value& notify){
		std::cout << "SessCacheWatch::watchCallBack!" << std::endl;
		m_cfg.setData(notify);
		return 0;
	}

	SessCacheGrp::SessCacheGrp(){

	}

	SessCacheGrp::~SessCacheGrp(){
		for (unsigned int n = 0; n < m_caches.size(); n++){
			delete m_caches[n];
		}
	}

	int SessCacheGrp::init(const std::vector<SessCacheWatcher*>& watches){
		for (unsigned int n = 0; n < watches.size(); n++){
			SessCacheWatcher* w = watches[n];
			RedisSessCache* s = new RedisSessCache(w->getDataSrc());
			m_caches.push_back(s);
		}
		return 0;
	}

	int SessCacheGrp::getSession(const std::string &key, std::vector<Sess> &m){
		std::vector<Sess> tm;
		for (unsigned int n = 0; n < m_caches.size(); ++n){
			SessCache* p = m_caches[n];
			if (p)
				p->getSession(key, tm);
		}

		std::set<std::string> sessids;
		for (unsigned int n = 0; n < tm.size(); ++n){
			const Sess& ss = tm[n];
			if (sessids.find(ss.sessid()) != sessids.end())
				continue;
			sessids.insert(ss.sessid());
			m.push_back(ss);
		}

		return 0;
	}

	int SessCacheGrp::setSession(const Sess &s){
		for (unsigned int n = 0; n < m_caches.size(); ++n){
			SessCache* p = m_caches[n];
			if (p)
				p->setSession(s);
		}
		return 0;
	}

	int SessCacheGrp::delSession(const Sess &s){
		for (unsigned int n = 0; n < m_caches.size(); ++n){
			SessCache* p = m_caches[n];
			if (p)
				p->delSession(s);
		}
		return 0;
	}

	SessCacheService* SessCacheService::s_ins = NULL;
	SessCacheService* SessCacheService::instance(){
		if (!s_ins){
			s_ins = new SessCacheService();
		}
		return s_ins;
	}

	void SessCacheService::destroy(){
		if (s_ins){
			delete s_ins;
			s_ins = NULL;
		}
	}

	SessCacheService::SessCacheService(){

	}

	SessCacheService::~SessCacheService(){
		for (unsigned int n = 0; n < m_watches.size(); n++){
			delete  m_watches[n];
		}
	}

	int SessCacheService::init(ZKClient* c, const Json::Value& config){
		if (!config.isArray()){
			std::cerr << "SessCacheService init config error!" << std::endl;
			return -1;
		}

		for (unsigned int n = 0; n < config.size(); n++){
			SessCacheWatcher* w = new SessCacheWatcher();
			if (!config[n].isString()){
				std::cerr << "SessCacheService init config error!" << std::endl;
				return -1;
			}
			if (0 > w->init(c, config[n].asString())){
				std::cerr << "SessCacheWatcher init fail!" << std::endl;
				return -1;
			}
			m_watches.push_back(w);
		}
		return 0;
	}

	SessCache* SessCacheService::getSessCache(){
		if (m_pss.get())
			return m_pss.get();

		SessCacheGrp* g = new SessCacheGrp();
		g->init(m_watches);
		m_pss.set(g);
		return g;
	}

}
