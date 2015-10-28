#ifndef __TYPE_MAP_H__
#define __TYPE_MAP_H__

#include <vector>
#include "base/ef_thread.h"

namespace gim{

class PushRequest;
class CliCon;


class SvList{
public:
	SvList();
	~SvList();

	int addServer(CliCon* c);
	int delServer(CliCon* c);
	

	int transRequest(const PushRequest& req);
private:
	ef::MUTEX m_cs;
	std::vector<CliCon*> m_svs;
	int m_cnt;
}; 

class TypeMap{
public:

	static int addServer(CliCon* c);
	static int delServer(CliCon* c);	

	static int transRequest(const PushRequest& req);
private:
	static SvList m_svlsts;
};

};

#endif/*__TYPE_MAP_H__*/
