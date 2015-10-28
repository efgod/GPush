#include "server_status.h"
#include <sstream>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "net/ef_sock.h"
#include "zk_client.h"

namespace gim{

using namespace std;
using namespace ef;

int ServerStatus::parseFromJson(const  Json::Value & v){

	if (v["ID"].isInt()) {
		ID = v["ID"].asInt();
	}
 
	if (v["IPs"].isArray()) {
		const Json::Value &jv = v["IPs"];
		for (int i = 0; i < (int)jv.size(); i++) {
			IPs.push_back(jv[i].asString());
		}
	}

	if (v["CliConfigs"].isArray()) {

		const Json::Value &jv = v["CliConfigs"];
		PortConfig pc;

		for (int i = 0; i < (int)jv.size(); i++) {
			const Json::Value &cv = jv[i];
			if (cv["ListenPort"].isInt()) {
				pc.Port = cv["ListenPort"].asInt();
			}
 
			if (cv["MaxType"].isInt()) {
				pc.MaxType = cv["MaxType"].asInt();
			} 

			if (cv["MinType"].isInt()) {
				pc.MinType = cv["MinType"].asInt();
			}

			if (!cv["Others"].isNull()) {
				pc.Others = cv["Others"];
			}
		}

		Ports.push_back(pc);
	}

	if (!v["Properties"].isNull()){
		Properties = v["Properties"];		
	}


	return 0;
}

int ServerStatus::serializeToJson(Json::Value& v) const
{
	v["ID"] = ID;

	for (int i = 0; i < (int)IPs.size(); i++) {
		v["IPs"][i] = IPs[i];
	}

	v["Properties"] = Properties;

	vector<PortConfig>::const_iterator pit = Ports.begin();
	for (int i = 0; pit != Ports.end(); pit++, i++) {
		Json::Value &servcfgs = v["CliConfigs"][i];
		servcfgs["ListenPort"] = pit->Port;
		servcfgs["MaxType"] = pit->MaxType;
		servcfgs["MinType"] = pit->MinType;
		servcfgs["Others"] = pit->Others;
	}

	return 0;
}


static int getPublicIPFromShell(std::string& ip){
	FILE* f = NULL;
	char buf[1024] = {0};

	f = popen("curl ipinfo.io", "r");

	if(!f){
		return -1;
	}

	fread(buf, sizeof(buf), 1, f);
	pclose(f);

	Json::Reader reader;
	Json::Value root;
	if(reader.parse(buf, root)){
		ip = root["ip"].asString();
	}else{
		return -2;
	}

	return 0;
}

int ServerStatus::autoSetIPs(bool need_public_ip){

	int ret = 0;

	string pubip;

	ret = getPublicIPFromShell(pubip);

	if(ret < 0){
		return ret;
	}
	
	vector<string> ips;
	getIPs(ips);	

	for(size_t i = 0; i < ips.size(); ++i){
		if(ips[i] == pubip){
			continue;
		}
		IPs.push_back(ips[i]);
	}

	if(need_public_ip){
		IPs.push_back(pubip);
	}

	return ret;
}


int ServerStatus::parseFromString(const std::string& s){
	Json::Value v;
	Json::Reader r;
	
	if(r.parse(s, v)){
		return parseFromJson(v);
	}


	return -1;
}

int ServerStatus::serializeToString(std::string& s) const{
	Json::Value v;
	serializeToJson(v);
	s = Json::FastWriter().write(v);
	return 0;
}
ServerNode::ServerNode(ZKClient* c, const std::string& typepath, const std::string& id)
:m_cli(c)
{
	m_path = zkPath(typepath, id);
	if (c)
		c->addSrvNode(this);
}

ServerNode::~ServerNode()
{
	if (m_cli){
		m_cli->deleteNode(m_path);
		m_cli->delSrvNode(this);
	}
}
int ServerNode::init(const ServerStatus& s)
{
	if (m_cli){
		Json::Value v;
		if (s.serializeToJson(v) >= 0){
			m_data = Json::FastWriter().write(v);
			return m_cli->createEphemeralNode(m_path, m_data);
		}
	}

	return -1;
}

int ServerNode::setStatus(const ServerStatus& s){
	Json::Value v;
	s.serializeToJson(v);
	std::string data = Json::FastWriter().write(v);

	if (m_data != data){
		m_data = data;
		return setData();
	}
	return 0;
}
int ServerNode::setData()
{
	int ret = -1;
	if (m_cli){
		ret = m_cli->setNodeData(m_path, m_data);
		if (ZNONODE == ret){
			ret = m_cli->createEphemeralNode(m_path, m_data);
		}
	}
	return ret;
}
};
