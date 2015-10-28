#include "logic_log.h"
#include "net/ef_net_log.h"

namespace gim{
	using namespace ef;

	static ef::SimpleLog g_logciLog;
	static ef::SimpleLog g_statisLog;

	int initLogicLog(const std::string& path, int level){
		g_logciLog.setName("PushServer");
		g_logciLog.setPath(path);
		g_logciLog.setLevel(level);
		return 0;
	}

	int initLogicNetLog(const std::string& path, int level){
		setNetLogName("LogicNet");
		setNetLogPath(path);
		setNetLogLevel(level);
		return 0;
	}


	int initLogicStatisLog(const std::string& path){
		g_statisLog.setName("LogicStatis");
		g_statisLog.setPath(path);
		g_statisLog.setLevel(LOG_LEVEL_ALL);
		
		return 0;
	}

	ef::Logger getLogicLog(int level){
		return g_logciLog(level);
	}

	ef::Logger getLogicStatisLog(){
		return g_statisLog(LOG_LEVEL_ERROR);
	}	

	
}
