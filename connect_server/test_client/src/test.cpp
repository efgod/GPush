#include "net/ef_sock.h"
#include "proto/gpush.pb.h"
#include "base/ef_utility.h"
#include "base/ef_log.h"
#include "base/ef_aes.h"
#include "base/ef_md5.h"
#include "msg_head.h"
#include "ef_crypt.h"
#include <iostream>
#include <algorithm>
#include <sys/epoll.h>
#include <sstream>
#include <fstream>
#include <string>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <map>

using namespace ef;
using namespace std;
using namespace gim;


#define isBigEndian()   ((*(unsigned short *) ("KE") >> 8) == 'K')
#define rol2(x,n)( ((x) << (n)) | ((x) >> (64-(n))) )
#define swap8(b) ( (rol2((uint64_t)(b),8 )&0x000000FF000000FFULL) | \
		   (rol2((uint64_t)(b),24)&0x0000FF000000FF00ULL) | \
		   (rol2((uint64_t)(b),40)&0x00FF000000FF0000ULL) | \
		   (rol2((uint64_t)(b),56)&0xFF000000FF000000ULL) )
#ifndef htonll
#define htonll(b) ( isBigEndian()?(b):swap8(b) )
#endif
#ifndef ntohll
#define ntohll(b) ( isBigEndian()?(b):swap8(b) )
#endif

#define STATUS_OK (0)
#define STATUS_SOCKET_ERR (-1)
#define STATUS_OTHER_ERR  (-1000)
#define KEEPALIVE_SPAN	  (300 * 1000)

enum{
	CLIENT_STATUS_INIT = 0,
	CLIENT_STATUS_WAIT_RESP = 1,
};

#define LOG_OUT std::cout << ef::getStrTimeAndPid(time(NULL)) \
		<< "<sn:" << lastSn() << "> "

#define LOG_OUT_NO_SN std::cout << ef::getStrTimeAndPid(time(NULL))

class Client{
public:
	Client(int32 fd = -1):m_fd(fd), m_sn(0){
	}

	~Client(){
		if(m_fd >= 0){
			closesocket(m_fd);
		}
	}
	
	int32   const_req(int c, std::string &req, const std::string &body){

		int32   len = 4 + 4 + 4 + body.size();
		uint32  magic = htonl(0x20140417);
		uint32  cmd = htonl(c);
		uint32  nlen = htonl(len);

		req.append((char*)&magic, 4);
		req.append((char*)&nlen, 4);
		req.append((char*)&cmd, 4);
		req.append(body.data(), body.size());

		return  (int32)len;
	}

	int32	const_service_req(const PushRequest& srreq, 
			std::string& req){
		return srreq.SerializeToString(&req);
	}

	int32	parse_service_resp(const std::string& resp, 
			PushResponse& svresp){
		return svresp.ParseFromString(resp);
	}

	int32	login(const std::string& cid, const std::string& version, 
		const std::string& token){
		int32 ret = 0;
		LoginRequest lgr;

		string enctk;

		if(token.size()){
			ef::encrypt(token, enctk);	
		}	

		lgr.set_id(cid);		
		lgr.set_type(-1);
		lgr.set_version(version);	
		lgr.set_token(enctk);
	
		m_cid = cid;
		m_token = token;
		LoginResponse lgresp;
		std::string req;
		lgr.SerializeToString(&req);	
	
		std::string rsp;
		ret = send_req(100, req);

		if(ret < 0){
			LOG_OUT << "test_login send fail, cid:" << m_cid 
				<< std::endl;
			return  ret;
		}
		
		int32 cmd = 0;
		ret = recv_resp(cmd, rsp);	
		if(ret <= 0){
			LOG_OUT << "login recv_resp fail, cid:" << m_cid
				<< std::endl;
			return ret;
		}

		lgresp.ParseFromString(rsp);
		if(lgresp.status() == 0){
			LOG_OUT << "test_login success: cid:" << m_cid 
				<< ", sessid:" << lgresp.sessid() << std::endl;	
		}else{
			LOG_OUT << "test_login fail: cid:" << m_cid 
				<< ", status:" 
				<< lgresp.status() << std::endl;	
			return -1;
		}
		m_sessid = lgresp.sessid();
		++success_count;
		if(token.size()){
			m_key = token + m_sessid;
		}
		return	ret;
	}

	int32	keepAlive(){
		int32 ret = 0;
		if(gettime_ms() - m_send_time < KEEPALIVE_SPAN){
		//	LOG_OUT << "cid:" << m_cid
		//		<< "keepAlive not send!" << std::endl;
			return	0;
		}
		ret = send_req(0, "");
		if(ret < 0){
			LOG_OUT << "cid:" << m_cid
				<< "keepAlive send fail!" << std::endl;
			return  ret;
		}else{
			LOG_OUT << "cid:" << m_cid
				<< " keepAlive!" << std::endl;
		}
		return  ret;
	}


	int32	get_messages(){
		int32 ret = 0;
		
	}


	int32   disconnect(){
		if(m_fd != EF_INVALID_SOCKET){
			closesocket(m_fd);
			m_fd = EF_INVALID_SOCKET;
		}
		return  0;
	}	

	int32   do_recv(std::string  &msg){

		uint32  totallen = 0;
		int     ret = 0;
		int     rcvcnt = 0;

		msg.resize(12);
		char* buf = (char*)msg.data();
		
		while(rcvcnt < 12){
			ret = recv(m_fd, buf + rcvcnt, 12 - rcvcnt, 0);
			if(ret <= 0){
				return -1;
			}
			rcvcnt += ret;
		}

		totallen = *(uint32*)(buf + 4);
		totallen = ntohl(totallen);
		rcvcnt = 12;

		msg.resize(totallen);
		buf = (char*)msg.data() + 12;

		while(rcvcnt < totallen){
			ret = recv(m_fd, buf, totallen - rcvcnt, 0);

			if(ret <= 0){
				LOG_OUT << "server addr:" << m_server_addr
					<< ", port:" << m_server_port 
					<< "recv body fail, errno:" << strerror(errno)
					<< std::endl;
				return  STATUS_SOCKET_ERR;
			}
			rcvcnt += ret;
			buf += ret;
		}
		return  totallen;

	}

	int32   handle_service_request(const std::string& resp){
		int32 ret = 0;
		PushRequest svreq;
		std::string outpayload;
		++total_recv_req;
		if(!svreq.ParseFromString(resp)){
			LOG_OUT << "cid:" << m_cid
				<< " handle_service_request ParseFromString fail\n";
			return -1;
		}
		if(svreq.to_sessid() != m_sessid){
			LOG_OUT << "cid:" << m_cid
				<< " handle_service_request,ret sessid:" 
				<< svreq.from_sessid()
				<< " != m_sessid:" << m_sessid << std::endl;
			ret = -200;				
		}		

		LOG_OUT << "cid:" << m_cid << ", handlePushRequest:" << svreq.payload()
			<< std::endl;
		PushResponse svresp;
		svresp.set_sn(svreq.sn());
		svresp.set_from_sessid(m_sessid);
		svresp.set_to_sessid(svreq.from_sessid());
		svresp.set_status(ret);
		std::string msg;
		std::string body;
		svresp.SerializeToString(&body);
		ret = send_req(201, body);
		if(ret == 0)
			++recv_req_success_count;
		return ret;	
	}

	std::string lastSn(){
		std::stringstream os;
		os << m_cid << "." << m_sn;
		return os.str();
	}


	int64	lastSendTime(){
		return m_send_time;
	}

	int32	handle_resp(){
		int32 cmd = 0;
		int32 ret = 0;
		std::string resp_body;
		int64 n = gettime_ms();
		if(sockGetPending(m_fd) < 12){
			LOG_OUT << "handle_resp fail, server close\n";
			return -1;
		}

		while(sockGetPending(m_fd) >= 12){
			ret = recv_resp(cmd, resp_body);
			if(ret < 0){
				LOG_OUT << "handle_resp recv_resp fail\n";
				return ret;
			}
			switch(cmd){
			case PUSH_REQ:
				total_spend_time -= (n - m_send_time);
				ret = handle_service_request(resp_body);
				break;
			}
			if(ret < 0)
				LOG_OUT << "cid:" << m_cid 
					<< " handle event fail, delete\n";
		}
		return ret;	
	}

	int32   recv_resp(int32& cmd, std::string &resp){

		int32	ret = 0;
		uint32  totallen = 0;
		std::string str;
		std::string encresp;
		int64 n = gettime_ms();
		ret = do_recv(str);

		if(ret < 0){
			LOG_OUT << " server addr:" << m_server_addr
				<< ", port:" << m_server_port 
				<< " recv resp fail\n";
			return  ret;
		}
		++total_resp;	
		total_spend_time += n - m_send_time;
		head h = *(head*)&str[0];
		cmd = htonl(h.cmd);
					
		resp = str.substr(12);

		return  ret;
	}

	int32   send_req(int cmd, const std::string& req){
		std::string msg;
		std::string encreq;
		const_req(cmd, msg, req);
		
		int ret = send(m_fd, msg.data(), msg.size(), 0);
		if(ret = 0)
			return -1;
		m_send_time = gettime_ms();
		++total_req;
		return  ret;
	}	

	int32	bind_connect(const  std::string& serverip, int32 serverport){
		
		m_fd = tcpConnect(const_cast<char*>(serverip.data()), serverport, 
				NULL, 0);
		if(m_fd < 0){
			LOG_OUT << "bind_connect fail, serverip:" << serverip
				<< ", serverport:" << serverport << std::endl;
			return -1;
		}
		m_server_addr = serverip;
		m_server_port = serverport;
		return m_fd;
	}

	int32	getlocalport(){
		struct sockaddr_in a;
		memset(&a, 0, sizeof(a));
		socklen_t len = sizeof(a);
		getsockname(m_fd, (sockaddr*)&a, &len);
		return	a.sin_port;
	}

	SOCKET getFd(){
		return m_fd;
	}

	int32 localPort(){
		return 	getlocalport();
	}

private:
	friend	class CliSet;
	std::string	m_server_addr;
	int32		m_server_port;
	std::string	m_cid;
	std::string	m_token;
	int32		m_sn;
	int64 		m_send_time;
	std::string	m_sessid;
	std::string	m_key;
	SOCKET	m_fd;
	static	int32	total_req;
	static  int32	total_resp;
	static	int32	last_total_req;
	static	int32	last_total_resp;
	static	int32	success_count;
	static	int32	total_recv_req;
	static	int32	recv_req_success_count;
	static	int64	total_spend_time;
	static	int64	last_total_spend_time;
}; 


int32	Client::total_req = 0;
int32	Client::total_resp = 0;
int32	Client::last_total_req = 0;
int32	Client::last_total_resp = 0;
int32	Client::success_count = 0;
int32	Client::total_recv_req = 0;
int32	Client::recv_req_success_count = 0;
int64	Client::total_spend_time = 0;
int64	Client::last_total_spend_time = 0;

class CliSet{
public:
	CliSet():m_run(false){
	}

	int32 init(const std::string& serverip, int32 serverport, 
		int32 max_con, const std::string& cid, 
		const std::string& token, const std::string& version){
		m_server_addr = serverip;
		m_server_port = serverport;
		m_epoll_fd = epoll_create(65535);
		m_max_cnt = max_con;
		m_cid = cid;
		m_token = token;
		m_version = version;
		return 0;
	}

	int32 connect_server(){
		Client* c = new Client();
		int i = 0;
		int32 ret = 0;
		while(c->bind_connect(m_server_addr, 
			m_server_port) < 0 && i < 20){
			++i;
		}
		if(i >= 20){
			delete c;
		}else{
			std::stringstream os;
			if(m_max_cnt > 1){
				os << m_cid << "." << c->localPort();
			}else{
				os << m_cid;
			}
			ret = c->login(os.str(), m_version, m_token); 
			if(ret < 0){
				LOG_OUT_NO_SN << "cid:" << os.str() << " login fail!"
					<< std::endl;
				delete c;
				return 0;
			}
			struct epoll_event ev;
			ev.events=EPOLLIN;
			ev.data.ptr=c;
			ret = epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, c->getFd(), &ev);
			if(ret < 0){
				LOG_OUT_NO_SN << "cid:" << os.str() << " epoll_ctl fail!"
					<< std::endl;
				delete c;
				return 0;
			}
			m_con_map.push_back(c);
		}
		return 0;
	}

	int32 run(int32 argc, const char** argv){
		m_run = true;
		const int32 events_on_loop = 128;
		const int32 slp_ms = 20;
		int32 kplvidx = 0;
		struct epoll_event events[events_on_loop];
		int64 lastms = gettime_ms();
		while(m_run){	
			if(m_con_map.size() < m_max_cnt)
				connect_server();
			int32 nfds = epoll_wait(m_epoll_fd, events, events_on_loop, slp_ms);
			if(nfds < 0){
				LOG_OUT_NO_SN << "epoll_wait fail!\n";
			}
			for(int i = 0; i < nfds; ++i){
				Client* c = (Client*)events[i].data.ptr;
				int32 ret = c->handle_resp();
				if(ret < 0){
					struct epoll_event ev;
					ev.events=EPOLLIN|EPOLLET;
					ev.data.ptr=c;
					epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, c->getFd(), &ev);
					m_con_map.erase(std::find(m_con_map.begin(), m_con_map.end(),c));
					delete c;
				}
			}
			int64 n = gettime_ms();
			int32 sz = m_con_map.size();
			for(int l = 0; l < 20 && l < sz; ++l){
				++kplvidx;
				Client* tc = NULL;
				int32 key = kplvidx % sz;
				tc = m_con_map[key];
				tc->keepAlive();
			}

			if(n - lastms > 1000){
				int32 req_tps = Client::total_req - Client::last_total_req;
				int32 resp_tps = Client::total_resp - Client::last_total_resp;
				int64 total_ms = Client::total_spend_time - Client::last_total_spend_time;
				Client::last_total_req = Client::total_req ;
				Client::last_total_resp = Client::total_resp;
				Client::last_total_spend_time = Client::total_spend_time;
				LOG_OUT_NO_SN << "total req:" << Client::total_req
					<< ", total resp:" << Client::total_resp
					<< ", success count:" << Client::success_count
					<< ", req tps:" << req_tps
					<< ", resp tps:" << resp_tps
					<< ", avg time:" << (resp_tps ? (total_ms / resp_tps) : 0)
					<< ", total_recv_req:" << Client::total_recv_req
					<< ", recv_req_success_count:" << Client::recv_req_success_count
					<< std::endl;
				lastms = n;
			}	
		}
	}

private:
	std::string     m_server_addr;
	int32	   m_server_port;
	std::string     m_local_ip;
	std::string	m_cid;
	std::string	m_token;
	int32		m_epoll_fd;
	int32		m_run;
	int32		m_max_cnt;
	int32		m_enc;
	std::string	m_version;
	std::vector<Client*> m_con_map;
};


int main(int argc, const char** argv){
	if(argc < 7){
		LOG_OUT_NO_SN << "test <addr> <port> <count> "
			"<cid> <token> <version> \n"
			<< std::endl;
		return	0;
	}
	int ret = 0;
	int l = 0;
	CliSet clst;
	std::string addr = argv[1];
	int port = atoi(argv[2]);
	int cnt = atoi(argv[3]);
	std::string cid = argv[4];
	std::string token = argv[5];
	std::string ver = argv[6];
		
	std::cout << "addr:" << addr << ", port:" << port << ", cid:" 
		<< cid << ", token:" << token << ", ver:" << ver
		<< std::endl;

	clst.init(addr, port, cnt, cid, token, ver);
	clst.run(argc - 7, argv + 7);
	return	0;
}
