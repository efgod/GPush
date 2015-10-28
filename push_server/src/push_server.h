#ifndef __PUSH_SERVER_H__
#define __PUSH_SERVER_H__

#include <map>
#include "main_config.h"
#include "server_status.h"
#include "net/ef_server.h"
#include "base/ef_loader.h"

namespace gim{

class ZKClient;
class SvManager;


//you can overwrite the virtual function
class PushServer{
public:
	PushServer();
	virtual ~PushServer();

	int init(const std::string& conf_file);
	int init(const MainConfig& conf);


	int start();
	int stop();

	const MainConfig& getConfig() const{
		return m_conf;
	}


	ZKClient* getZKClient(){
		return m_zkc;
	}
	
private:


	int initLog();
	int initZk(const std::string& zkUrl);
	int initEventLoop();
	int intSessCache();
	int initServerManager();
	int startListen(int port);
	int stopListen(int port);
	int clean();

	MainConfig m_conf;

	bool m_run;

	ZKClient* m_zkc;

	SvManager* m_svman;
		
	ef::Server m_serv;

};


}

#endif
