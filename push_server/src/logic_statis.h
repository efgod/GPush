#ifndef __LOGIC_STATISTIC_H__
#define __LOGIC_STATISTIC_H__

#include "base/ef_common.h"

namespace gim{

struct LogicStatistic{
	int StartTimestamp;
	int RunningTime;
	int RequestCount;
	int ResponseCount;	 
	int SuccessCount;
	int FailCount;
};

};


#endif/*__LOGIC_STATISTIC_H__*/
