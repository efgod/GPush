#ifndef __SERVER_MANAGER_H__
#define __SERVER_MANAGER_H__

#include <map>
#include <vector>
#include "base/ef_loader.h"
#include "server_status.h"

namespace ef{
	class Server;
	class EventLoop;
}

namespace gim{

typedef std::map<int, ServerStatus> StatusMap;
typedef ef::DataSrc<StatusMap> DtSrc;
typedef ef::Loader<StatusMap> Loader;

class SvCon;
class PushServer;
class ZKClient;
class SvListWatcherBase;

class Svlist{
	public:
		
		Svlist(PushServer* s, int kpms, ef::EventLoop* l, DtSrc* src);
		~Svlist();
	
		int addServer(int svid, SvCon* con);
		int delServer(int svid);
		SvCon* getServer(int id);
		int checkServers();
	
	private:
		int connectServer(const ServerStatus& s);
		int connectIPArray(SvCon* c, const std::vector<std::string>& a, int port);
	
		
		PushServer* m_sv;
		int m_keepalivems;
		ef::EventLoop* m_evlp;
		Loader m_loader;
		std::map<int, SvCon*> m_svcons; 

};

class SvManager{
public:
	
	SvManager(PushServer* lgsv, ef::Server* sv);
	~SvManager();

	int init(ZKClient* zkc, const std::string& svpath, int keepalivems = 30000);
	

private:
	
	int onSvListChange(const SvStatusMap& sss); 
	int notifySvlistChange();

	PushServer* m_lgsv;
	ef::Server* m_sv;
	std::string m_svpath;
	int m_keepalivems;
	DtSrc m_datasrc;
	SvListWatcherBase* m_svlstob;
	std::vector<Svlist*> m_svlsts; 
};



};

#endif
