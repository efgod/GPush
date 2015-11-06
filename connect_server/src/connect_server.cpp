#include "connect_server.h"
#include <fstream>
#include "log_init.h"
#include "client_conn.h"
#include "sess_cache.h"
#include "type_map.h"
#include "zk_client.h"
#include "data_watcher.h"
#include "net/ef_server.h"
#include "net/ef_net_log.h"
#include "base/ef_statistic.h"
#include "base/ef_utility.h"


namespace gim{

using namespace std;
using namespace ef;

static int output_statistic(void* par, const string& l){
	ConnectServer* s = (ConnectServer*)par;
	logError("ConnectStatistic") << "<svid:" << s->getConfig().ID
		<< "> " << l;
	return 0;
}


static void output_zklog(void* par, const string& l){
	ConnectServer* s = (ConnectServer*)par;
	logError("ConnectZookeeper") << "<svid:" << s->getConfig().ID
		<< "> " << l;
}


int ConnectServer::initZk(const std::string& zkUrl){
	m_zkc = new ZKClient();
	m_zkc->setLogFn(this, output_zklog);
	if(m_zkc->init(zkUrl) < 0){
		delete m_zkc;
		return -5;
	}

	return 0;
}



int ConnectServer::initLog(){
	logInit(m_conf.LogConfig);
	initStatistic(output_statistic, this);
	setNetLogName("ConnectNet");
	setNetLogPath(m_conf.NetLogPath);
	setNetLogLevel(m_conf.NetLogLevel);
	return 0;
}



int ConnectServer::initCompent(){
	int ret = 0;

	ret = SessCacheService::instance()->init(m_zkc, m_conf.SessCacheConfig);

	if(ret < 0){
		return -3;
	}

	return ret;
}

int ConnectServer::freeCompent(){
	SessCacheService::destroy();

	if(m_zksvnd){
		delete m_zksvnd;
		m_zksvnd = NULL;
	}

	if(m_zkc){
		delete m_zkc;
		m_zkc = NULL;
	}

	return 0;
}

int ConnectServer::startListen(){	
	int ret = 0;

	m_serv.setEventLoopCount(m_conf.ThreadCount);
	ret = m_serv.init();

	if(ret < 0){
		return ret;
	}


	std::vector<CliConfig>::iterator itor = m_conf.CliConfigs.begin();

	for(; itor != m_conf.CliConfigs.end(); ++itor){

		CliConfig& cliconf = *itor;

		CliConFactory* cfac = new CliConFactory(this, &cliconf);
		CDispatcher* cdisp = new CDispatcher(this, &cliconf);
		ret = m_serv.startListen(cliconf.ListenPort, cfac, cdisp);
		if(ret < 0){
			cout << "listen at port:" << cliconf.ListenPort << " fail!\n";
			return ret;
		}
		m_cfacs.push_back(cfac);
		m_cdisps.push_back(cdisp);
	}

	return 0;
}

int ConnectServer::stopListen(){	
	int ret = 0;


	std::vector<CliConfig>::iterator itor = m_conf.CliConfigs.begin();

	for(; itor != m_conf.CliConfigs.end(); ++itor){

		CliConfig& cliconf = *itor;

		ret = m_serv.stopListen(cliconf.ListenPort);
		if(ret < 0){
			cout << "stop listen port:" << cliconf.ListenPort << " fail!\n";
			return ret;
		}
	}

	for(size_t i = 0; i < m_cfacs.size(); ++i){
		delete m_cfacs[i];
		delete m_cdisps[i];
	}

	return 0;
}

int ConnectServer::start(){
	m_serv.run();
	m_run = true;
	while(m_run){
		reportStatus();
		sleep_ms(10000);
	}
	return 0;
}

int ConnectServer::stop(){
	m_run = false;
	stopListen();
	m_serv.stop();
	return 0;
}


int ConnectServer::init(const std::string& conf_file){

	ServerConfig conf;
	int ret = conf.init(conf_file);

	if(ret < 0){
		std::cout << "config init fail, ret:" << ret << std::endl;
		return ret;
	}

	ret = init(conf);

	return ret;
}


int ConnectServer::init(const ServerConfig& conf){
	int ret = 0;

	m_conf = conf;

	initLog();

	ret = initZk(m_conf.ZkUrl);
	if (ret < 0){
		std::cout << "initZk fail!" << std::endl;
		return ret;
	}

	ret = initCompent();
	
	if(ret < 0){
		cout << "initCompent fail!" << endl;
		return ret;
	}


	ret = startListen();
	if (ret < 0){
		std::cout << "startListen fail!" << std::endl;
		return ret;
	}
	

	ret = initStatusNode(m_conf.StatusZkPath, m_conf.ID);
	if(ret < 0){
		cout << "initStatusNode fail!" << endl;
		return ret;
	}

	return ret;
	
}

int ConnectServer::free(){
	return freeCompent();
}

EventLoop& ConnectServer::getEventLoop(int idx){
	return m_serv.getEventLoop(idx);
}

const ServerConfig& ConnectServer::getConfig() const{
	return m_conf;
}

int ConnectServer::initStatusNode(const std::string& statuspath, int id){

	m_status.ID = id;

	m_status.autoSetIPs(true);

	std::vector<CliConfig>::iterator itor = m_conf.CliConfigs.begin();

	for(; itor != m_conf.CliConfigs.end(); ++itor){
		PortConfig p;
		p.Port = itor->ListenPort;
		p.MaxType = itor->MaxType;
		p.MinType = itor->MinType;	
		m_status.Ports.push_back(p);
	}

	m_zksvnd = new ServerNode(m_zkc, statuspath, itostr(id));

	int ret = m_zksvnd->init(m_status);
	
	if(ret < 0){
		return -12;
	}

	return 0;
}



int ConnectServer::reportStatus(){
	if(!m_zksvnd){
		return 0;
	}
	m_status.Properties["ConnectionCount"] = CliCon::connectionCount();
	m_status.Properties["ReportTimestamp"] = (int)time(NULL);


	int ret = m_zksvnd->setStatus(m_status); 

	if(ret < 0){
		logError("ConnectZookeeper") << "<svid:" << getConfig().ID
			<< "> <action:reportStatus> <status:" << ret << ">";
	}

	return ret;
}



};
