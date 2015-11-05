#include "client.h"
#include <string>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include "client_log.h"


int g_run = 1;

int system_shutdown( void )
{
	g_run = false;
	return 0;
}


static void signal_handler(int sig)
{

	switch(sig) {
	case SIGHUP:
	case SIGTERM:
		system_shutdown();
		break;
	}

}
namespace gim{
	
	  void logprint(LogLevel level, const char* logbuf){
        return;
    }
	class myclient:public Client{
	public:
		virtual int handleMessage(const std::string& msg){
			std::cout << msg << std::endl;
			return 0;
		}
	};
}

int main(int argc, char* const* argv){
	if(argc < 4){
		std::cout << "args [ip] [port] [uid]!" << std::endl;
		return -1;
	}
	
	std::string ip = argv[1];
	int port = atoi(argv[2]);
	std::string uid = argv[3];
	
	
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTSTP, SIG_IGN); 
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT,  SIG_IGN);
	signal(SIGURG,  SIG_IGN);
	signal(SIGTERM, signal_handler);
	gim::myclient c;
	if(c.init() < 0){
		std::cout << "client init error" << std::endl;
	}
	
	c.setKeepaliveTimeout(30); //30ms
	
	c.login(ip, port, uid, "1.0.0");
	while(g_run){
		std::cout << "running!\n";
		sleep(1);
	}
	c.stop();
	return 0;
}