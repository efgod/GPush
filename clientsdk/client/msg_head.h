#ifndef __MSG_HEAD_H__
#define __MSG_HEAD_H__

#include <base/ef_btype.h>

namespace gim{

using namespace ef;

enum{
	KEEPALIVE_REQ = 0,
	KEEPALIVE_RESP = KEEPALIVE_REQ + 1,
	LOGIN_REQ = 100,
	LOGIN_RESP = LOGIN_REQ + 1,
	PUSH_REQ = 200,
	PUSH_RESP = PUSH_REQ + 1,
	KICK_CLIENT = 400,
	JSON_PUSH_REQ = 600,
	JSON_PUSH_RESP = PUSH_REQ + 1,
	REDIRECT_RESP = 1001,	
	MAGIC_NUMBER = 0x20141228,
};

struct head{
	int32 magic;
	int32 len;//include head 
	//0: keepalive, 1: keepalive resp
	//100: login, 101: login resp
	//200: push req, 201: push resp
	//1001: redericet
	int32 cmd;			
};


};

#endif
