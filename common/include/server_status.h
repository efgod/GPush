#ifndef __SERVER_STATUS_H__
#define __SERVER_STATUS_H__

#include <vector>
#include <map>
#include <string>
#include <json/json.h>


namespace gim{

struct PortConfig{
	int Port;
	int MaxType;
	int MinType;
	Json::Value Others;

	PortConfig():Port(0),MaxType(0),MinType(0){
	}
};

struct ServerStatus{
	int ID;
	std::vector<std::string> IPs;
	std::vector<PortConfig> Ports;
	Json::Value Properties;

	int autoSetIPs(bool need_public_ip = true);  
	int parseFromJson(const Json::Value& v);
	int serializeToJson(Json::Value& v) const ;
	int parseFromString(const std::string& v);
	int serializeToString(std::string& v) const ;	
};

typedef std::map<std::string/*id*/, ServerStatus>  SvStatusMap;


class ZKClient;

class ServerNode
{
public:
	friend class ZKClient;
	ServerNode(ZKClient* c, const std::string& typepath, const std::string& id);
	~ServerNode();
	int init(const ServerStatus& s);
	int setStatus(const ServerStatus& s);
private:
	int setData();
private:
	ZKClient* m_cli;
	std::string m_path;
	std::string m_data;
};

}

#endif/*__SERVER_STATUS_H__*/
