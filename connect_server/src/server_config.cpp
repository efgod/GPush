#include "server_config.h"
#include "base/ef_log.h"
#include <fstream>

namespace gim{


using namespace std;
using namespace ef;

int ServerConfig::init(const std::string& f){

	int ret = 0;
	fstream ifs;

	ifs.open(f.data(), ios::in);

	if(!ifs.is_open()){
		return -1;
	}

	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(ifs, root, false)) {
		return -2;
	}

	ret = parseFromJson(root);

	return ret;
}


int ServerConfig::parseFromJson(const Json::Value& root){

	int ret = 0;

	if (root["ID"].isInt()) {
		ID = root["ID"].asInt();
	}

	if (root["ThreadCount"].isInt()) {
		ThreadCount = root["ThreadCount"].asInt();
	}

	if (root["ZkUrl"].isString()) {
		ZkUrl = root["ZkUrl"].asString();
	}

	if (root["StatusZkPath"].isString()) {
		StatusZkPath = root["StatusZkPath"].asString();
	}

	if (root["NetLogLevel"].isString()) {
		NetLogLevel = getStrLevel(root["NetLogLevel"].asString().data());
	}

	if (root["NetLogPath"].isString()) {
		NetLogPath = root["NetLogPath"].asString();
	}

	if (root["LogConfig"].isString()) {
		LogConfig = root["LogConfig"].asString();
	}

	if (root["SessCacheConfig"].isArray()) {
		SessCacheConfig = root["SessCacheConfig"];
	}

	ret = loadPortsConfigs(root["CliConfigs"]);

	return ret;
}

	
int ServerConfig::loadPortsConfigs(const Json::Value& v){


	for(Json::Value::const_iterator itr = v.begin(); 
		itr != v.end(); ++itr){

		const Json::Value& cf = *itr;	
		
		const Json::Value& Encv = cf["Enc"];

		if(!Encv.isInt()){
			return -1;
		}
		
		const Json::Value& AliveMsv = cf["AliveMs"];
		if(!AliveMsv.isInt()){
			return -1;
		}

		const Json::Value& MinTypev = cf["MinType"];		
		if(!MinTypev.isInt()){
			return -1;
		}

		const Json::Value& MaxTypev = cf["MaxType"];
		if(!MaxTypev.isInt()){
			return -1;
		}
		
		const Json::Value& ListenPortv = cf["ListenPort"];
		if(!ListenPortv.isInt()){
			return -1;
		}

		const Json::Value& MaxReqQueSizev = cf["MaxReqQueSize"];
		if(!MaxReqQueSizev.isInt()){
			return -1;
		}

		const Json::Value& MaxPackCntPerMinv = cf["MaxPackCntPerMin"];
		if(!MaxPackCntPerMinv.isInt()){
			return -1;
		}

		const Json::Value& StartThreadIdxv = cf["StartThreadIdx"];
		if(!StartThreadIdxv.isInt()){
			return -1;
		}

		const Json::Value& ThreadCntv = cf["ThreadCnt"];		
		if(!ThreadCntv.isInt()){
			return -1;
		}

		CliConfig c;
		c.Enc = Encv.asInt();
		c.AliveMs = AliveMsv.asInt();
		c.MinType = MinTypev.asInt();
		c.MaxType = MaxTypev.asInt();
		c.ListenPort = ListenPortv.asInt();
		c.MaxReqQueSize = MaxReqQueSizev.asInt();
		c.MaxPackCntPerMin = MaxPackCntPerMinv.asInt();
		c.StartThreadIdx = StartThreadIdxv.asInt();
		c.ThreadCnt = ThreadCntv.asInt();
	
		CliConfigs.push_back(c);	
			
	}

	return 0;
}


int ServerConfig::serializeToJson(Json::Value& v) const{
	v["ID"] = ID;
	v["ThreadCount"] = ThreadCount;

	v["ZkUrl"] = ZkUrl;
	v["NetLogLevel"] = NetLogLevel;
	v["NetLogPath"] = NetLogPath;
	v["LogConfig "] = LogConfig;
	v["SessCacheConfig"] = SessCacheConfig;
	v["TokenKeyPath"] = TokenKeyPath;

	return 0;
}

string ServerConfig::toStyledString() const{
	Json::Value v;
	serializeToJson(v);
	return v.toStyledString();
}

}
