#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <vector>
#include <json/json.h> 

namespace gim{

class CliConfig{
public:
	int Enc;
	int AliveMs;
	int MinType;
	int MaxType;
	int ListenPort;
	int MaxReqQueSize;
	int MaxPackCntPerMin;
	int StartThreadIdx;
	int ThreadCnt;
};


class ServerConfig{
public:
	int ID; 
	int ThreadCount;

	std::string ZkUrl;
	std::string StatusZkPath;

	int NetLogLevel;
	std::string NetLogPath;

	std::string LogConfig;

	Json::Value SessCacheConfig;

	std::string TokenKeyPath;

	std::vector<CliConfig> CliConfigs;

	ServerConfig():ID(0), ThreadCount(0){
	}

	int init(const std::string& f);

	int serializeToJson(Json::Value& v) const;
	int parseFromJson(const Json::Value& v);

	std::string toStyledString() const;

private:
	int loadPortsConfigs(const Json::Value& v);
	
};

}

#endif
