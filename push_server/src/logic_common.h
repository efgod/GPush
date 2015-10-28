#ifndef __LOGIC_COMMON_H__
#define __LOGIC_COMMON_H__

#include <string>
#include "base/ef_btype.h"
#include "base/ef_log.h"
#include "base/ef_utility.h"

namespace ef{
	class EventLoop;
	class Connection;
};

namespace gim{
	class head;
	using namespace ef;

	int32 constructPacket(const head& h, 
		const std::string& body, std::string& pack);

	int32 decorationName(int32 svid, int32 conid,
		const std::string& n, std::string& dn);

	int32 getDecorationInfo(const std::string& dn, int32& svid,
		int32& conid, std::string& n);	

	int32 getSvTypeFromSessid(const std::string& ssid, int32& svtype);


};

#endif/*LOGIC_COMMON_H*/
