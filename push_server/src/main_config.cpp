#include "main_config.h"
#include "base/ef_log.h"
#include <fstream>

namespace gim{


using namespace std;
using namespace ef;

int MainConfig::init(const std::string& f){

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


int MainConfig::parseFromJson(const Json::Value& root){

	int ret = 0;

	if (root["ID"].isInt()) {
		ID = root["ID"].asInt();
	}

	if (root["Type"].isInt()) {
		Type = root["Type"].asInt();
	}

	if (root["ThreadCount"].isInt()) {
		ThreadCount = root["ThreadCount"].asInt();
	}

	if (root["ListenPort"].isInt()) {
		ListenPort = root["ListenPort"].asInt();
	} else {
		ListenPort = -1;
	}

	if (root["ZkUrl"].isString()) {
		ZkUrl = root["ZkUrl"].asString();
	}

	if (root["ConnSvZkPath"].isString()) {
		ConnSvZkPath = root["ConnSvZkPath"].asString();
	}

	if (root["KeepAliveSpanMS"].isInt()) {
		KeepAliveSpanMS = root["KeepAliveSpanMS"].asInt();
	}

	if (root["NetLogLevel"].isString()) {
		NetLogLevel = getStrLevel(root["NetLogLevel"].asString().data());
	}

	if (root["NetLogPath"].isString()) {
		NetLogPath = root["NetLogPath"].asString();
	}

	if (root["LogLevel"].isString()) {
		LogLevel = getStrLevel(root["LogLevel"].asString().data());
	}

	if (root["LogPath"].isString()) {
		LogPath = root["LogPath"].asString();
	}

	if (root["StatisLogPath"].isString()) {
		StatisLogPath = root["StatisLogPath"].asString();
	}

	if (root["SessCacheConfig"].isArray()) {
		SessCacheConfig = root["SessCacheConfig"];
	}

	return ret;
}

int MainConfig::serializeToJson(Json::Value& v) const{
	v["ID"] = ID;
	v["Type"] = Type;
	v["ThreadCount"] = ThreadCount;
	v["ListenPort"] = ListenPort;

	v["ZkUrl"] = ZkUrl;
	v["ConnSvZkPath"] = ConnSvZkPath;
	v["SessCacheConfig"] = SessCacheConfig;
	v["KeepAliveSpanMS"] = KeepAliveSpanMS;
	v["NetLogLevel"] = NetLogLevel;
	v["NetLogPath"] = NetLogPath;
	v["LogLevel"] = LogLevel;
	v["LogPath"] = LogPath;
	v["StatisLogPath"] = StatisLogPath;
	return 0;
}

string MainConfig::toStyledString() const{
	Json::Value v;
	serializeToJson(v);
	return v.toStyledString();
}

};
