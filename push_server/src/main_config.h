#ifndef __MAIN_CONFIG_H__
#define __MAIN_CONFIG_H__

#include "json/json.h"

namespace gim{

struct MainConfig{
	int ID;
	int Type;
	int ThreadCount;


	int ListenPort;

	std::string ZkUrl;
	std::string ConnSvZkPath;
	int KeepAliveSpanMS;

	std::string LogPath;
	int LogLevel;
	
	std::string NetLogPath;
	int NetLogLevel;

	std::string StatisLogPath;

	Json::Value SessCacheConfig;

	MainConfig():ID(0), Type(0), ThreadCount(12){
	}


	int init(const std::string& f);

	int serializeToJson(Json::Value& v) const;
	int parseFromJson(const Json::Value& v);

	std::string toStyledString() const;
private:
	
	
};


}


#endif/*__LOGIC_CONFIG_H__*/
