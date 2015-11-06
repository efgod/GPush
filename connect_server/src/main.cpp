#include <stdlib.h>
#include <iostream>
#include <signal.h>
#include <getopt.h>
#include "connect_server.h"
#include "base/ef_deamonize.h"

gim::ConnectServer* g_pdb = NULL;

int system_shutdown( void )
{
	g_pdb->stop();

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

static void printHelpInfo(){
	std::cout << 	"-h				help\n"
			"-d				daemon\n"
			"-c	<config>		config\n"
			"-s	<so path>		so path\n"
	
		<< std::endl;

}


int main(int argc, char* const* argv){


	const char* short_options = "hdc:s:";

	const struct option long_options[] = {
		{  "help",      0,   NULL,   'h'  },
		{  "daemon",      0,   NULL,   'd'  },
		{  "config",    1,   NULL,   'c'  },
		{  NULL,      0,    NULL,   0  }
	};

	int c;

	std::string type;
	std::string config;
	bool dm = false;

	while((c = getopt_long (argc, argv, short_options, long_options, NULL)) != -1){
		switch(c){
		case 'h':
			printHelpInfo();
			return 0;

		case 'd':
			dm = true;
			break;

		case 'c':
			config = optarg;
			break;		
		
		}			
	}

	if(!config.size()){
		printHelpInfo();
		return -1;
	}
	
	
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGTSTP, SIG_IGN); 
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGQUIT,  SIG_IGN);
	signal(SIGURG,  SIG_IGN);
	signal(SIGTERM, signal_handler);

	if(dm){
		ef::daemonize();
	}

	int ret = 0;
	gim::ConnectServer s;
	g_pdb = &s;

	ret = s.init(config);

	if(ret < 0){
		std::cout << "ConnectServer init fail\n";
		return ret;
	}	

	ret = s.start();
	
	if(ret < 0){
		std::cout << "ConnectServer start fail\n";
	}

	s.free();

	return 0;
}
