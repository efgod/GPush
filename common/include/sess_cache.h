#ifndef __SESS_CACHE_H__
#define __SESS_CACHE_H__

#include <string>
#include <vector>
#include "json/json.h"
#include "session.pb.h"
#include "base/ef_tsd_ptr.h"
#include "base/ef_loader.h"

namespace gim{
	class ZKClient;
	class RedisCli;
	class RedisGroup;
	class JsonWatcherBase;

	class SessCache{
	public:
		virtual ~SessCache(){}
		virtual int getSession(const std::string &key, std::vector<Sess>& m) = 0;
		virtual int setSession(const Sess &s) = 0;
		virtual int delSession(const Sess &s) = 0;
	};

	class RedisSessCache:public SessCache{
	public:
		enum{
			SESSION_DEFAULT_EXPIRE_SECOND = 500,
		};

		RedisSessCache(ef::DataSrc<Json::Value>* config);
		virtual ~RedisSessCache();
		virtual int getSession(const std::string& key, std::vector<Sess>& m);
		virtual int setSession(const Sess &s);
		virtual int delSession(const Sess &s);
	private:
		int init(const Json::Value& config);

		RedisCli* getHandle(const std::string &key);

		RedisGroup* m_dbg;
		int m_expire;
		ef::Loader<Json::Value> m_loader;
	};

	typedef ef::DataSrc<Json::Value> SCCDataSrc;
	typedef ef::Loader<Json::Value> SCCLoader;
	class SessCacheWatcher{
	public:
		SessCacheWatcher();
		~SessCacheWatcher();
		int init(ZKClient* c, const std::string& zkpath);
		SCCDataSrc* getDataSrc();
	private:
		int watchCallBack(int version, const Json::Value& notify);
	private:
		JsonWatcherBase* m_zkcfg;
		SCCDataSrc m_cfg;
	};

	class SessCacheGrp:public SessCache{
	public:
		SessCacheGrp();
		virtual ~SessCacheGrp();
		int init(const std::vector<SessCacheWatcher*>& watches);
		virtual int getSession(const std::string &key, std::vector<Sess>& m);
		virtual int setSession(const Sess &s);
		virtual int delSession(const Sess &s);
	private:
		std::vector<RedisSessCache*> m_caches;
	};
	
	class SessCacheService{
	public:
		static SessCacheService* s_ins;
		static SessCacheService* instance();  
		static void destroy();
	public:
		SessCacheService();
		~SessCacheService();
		int init(ZKClient* c, const Json::Value& config);
		SessCache* getSessCache();
	private:
		std::vector<SessCacheWatcher*> m_watches;
		typedef ef::TSDPtr<SessCache> SessCachePtr;
		SessCachePtr m_pss;
	};
}
#endif
