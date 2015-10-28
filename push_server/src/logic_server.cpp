#include "push_server.h"
#include "server_manager.h"
#include "listen_conn.h"
#include "logic_log.h"
#include "zk_client.h"
#include "list_watcher.h"
#include "sess_cache.h"
#include "net/ef_operator.h"
#include "net/ef_net_log.h"
#include "base/ef_log.h"
#include "base/ef_utility.h"
#include "base/ef_statistic.h"

namespace gim{

using namespace std;
using namespace ef;

static int outputStatistic(void* par, const string& l){
	PushServer* s = (PushServer*)par;
	getLogicStatisLog() << "<svid:" << s->getConfig().ID
		<< "> " << l;
	return 0;
}

int PushServer::initLog(){
	initLogicLog(m_conf.LogPath, m_conf.LogLevel);
	initLogicNetLog(m_conf.NetLogPath, m_conf.NetLogLevel);
	initLogicStatisLog(m_conf.StatisLogPath);
	initStatistic(outputStatistic, this);

	return 0;
}

int PushServer::startListen(int port){	
	int ret = 0;

	if(port <= 0){
		return 0;
	}

	static LConFactory cfac(this);
	ret = m_serv.startListen(port, &cfac);
	if(ret < 0){
		cout << "listen at port:" << port << " fail!\n";
		return ret;
	}

	return 0;
}

int PushServer::stopListen(int port){	
	int ret = 0;

	if(port <= 0){
		return 0;
	}

	ret = m_serv.stopListen(port);
	if(ret < 0){
		cout << "listen at port:" << port << " fail!\n";
		return ret;
	}

	return 0;
}

PushServer::PushServer()
	:m_zkc(NULL),
	m_svman(NULL){
}


PushServer::~PushServer(){
	
	clean();
}

int PushServer::start(){
	return m_serv.run();
}

int PushServer::stop(){
	stopListen(m_conf.ListenPort);
	m_serv.stop();
	return 0;
}

int PushServer::init(const std::string& conf_file){

	MainConfig conf;
	int ret = conf.init(conf_file);

	if(ret < 0){
		std::cout << "config init fail, ret:" << ret << std::endl;
		return ret;
	}

	ret = init(conf);

	return ret;
}

int PushServer::initServerManager(){

	m_svman = new SvManager(this, &m_serv);

	std::string path = m_conf.ConnSvZkPath;
	int kpms = m_conf.KeepAliveSpanMS;

	return m_svman->init(m_zkc, path, kpms);
}

int PushServer::intSessCache(){
	
	if (!m_conf.SessCacheConfig.isArray()){
		return -1;
	}
	
	int ret = SessCacheService::instance()->init(m_zkc, m_conf.SessCacheConfig);

	return ret;
}

int PushServer::init(const MainConfig& conf){
	int ret = 0;

	m_conf = conf;

	initLog();

	ret = initZk(m_conf.ZkUrl);
	if (ret < 0){
		std::cout << "initZk fail!" << std::endl;
		return ret;
	}

	ret = intSessCache();
	if (ret < 0){
		std::cout << "intSessCache fail" << std::endl;
		return ret;
	}


	ret = initEventLoop();
	if (ret < 0){
		std::cout << "initEventLoop fail!" << std::endl;
		return ret;
	}

	ret = startListen(m_conf.ListenPort);
	if (ret < 0){
		std::cout << "startListen fail!" << std::endl;
		return ret;
	}

	ret = initServerManager();
	if (ret < 0){
		std::cout << "initServerManger fail!" << std::endl;
		return ret;
	}

	return ret;
	
}

int PushServer::clean(){
	
	if(m_svman){
		delete m_svman;
		m_svman = NULL;
	}
	
	if(m_zkc){
		delete m_zkc;
		m_zkc = NULL;
	}

	return 0;
}

int PushServer::initZk(const std::string& zkUrl){
	m_zkc = new ZKClient();
	if(m_zkc->init(zkUrl) < 0){
		delete m_zkc;
		return -5;
	}

	return 0;
}


int PushServer::initEventLoop(){
	m_serv.setEventLoopCount(m_conf.ThreadCount);
	initServerManager();
	int ret = m_serv.init();

	return ret;
}

};
