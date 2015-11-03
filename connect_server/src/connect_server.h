#ifndef __CONNECT_SERVER_H__
#define __CONNECT_SERVER_H__

#include <map>
#include <vector>
#include <string>
#include "server_config.h"
#include "net/ef_server.h"
#include "json/json.h"
#include "server_status.h"

namespace ef{
class EventLoop;
};

namespace gim{

class ZKClient;
class CDispatcher;
class JsonWatcherBase;
class CliConFactory;

class ConnectServer{
public:

	ConnectServer():m_run(false), m_zkc(NULL),
		m_zksvnd(NULL){
		}

	int init(const std::string& path);
	int init(const ServerConfig& conf);
	int free();
	int start();
	int stop();

	ef::EventLoop& getEventLoop(int idx);

	const ServerConfig& getConfig() const;

	ZKClient* getZKClient(){ 
		return m_zkc;
	}

private:
	int initStatusNode(const std::string& statuspath, int id);
	int initLog();
	int initZk(const std::string& zkUrl);
	int reportStatus();
	int initCompent();
	int freeCompent();
	int startListen();
	int stopListen();


	ServerConfig m_conf;
	Json::Value m_jsonconf;
	ef::Server m_serv;

	std::vector<CliConFactory*> m_cfacs;
	std::vector<CDispatcher*> m_cdisps;

	ServerStatus m_status;


	bool m_run;

	ZKClient* m_zkc;
	ServerNode* m_zksvnd;

};

};



#endif/*__CONNECT_SERVER_H__*/
