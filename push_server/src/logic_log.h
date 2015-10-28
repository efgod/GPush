#ifndef __LOGIC_LOG_H__
#define __LOGIC_LOG_H__

#include <string>
#include "base/ef_log.h"

namespace gim{

	int initLogicLog(const std::string& path, int level);
	int initLogicNetLog(const std::string& path, int level);
	int initLogicStatisLog(const std::string& path);

	ef::Logger getLogicLog(int level);
	ef::Logger getLogicStatisLog();

	#define logicDebug getLogicLog(ef::LOG_LEVEL_DEBUG) << "[PID=" \
		<< getpid() << "] [THREAD=" \
		<< pthread_self() <<  "] [" << __FILE__ << ":" \
		<< __LINE__ << "] [" << __FUNCTION__ << "] "

	#define logicTrace getLogicLog(ef::LOG_LEVEL_TRACE) << "[PID=" \
		<< getpid() << "] [THREAD=" \
		<< pthread_self() <<  "] [" << __FILE__ << ":" \
		<< __LINE__ << "] [" << __FUNCTION__ << "] "

	#define logicInfo getLogicLog(ef::LOG_LEVEL_INFO) << "[PID=" \
		<< getpid() << "] [THREAD=" \
		<< pthread_self() <<  "] [" << __FILE__ << ":" \
		<< __LINE__ << "] [" << __FUNCTION__ << "] "

	#define logicWarn getLogicLog(ef::LOG_LEVEL_WARN) << "[PID=" \
		<< getpid() << "] [THREAD=" \
		<< pthread_self() <<  "] [" << __FILE__ << ":" \
		<< __LINE__ << "] [" << __FUNCTION__ << "] "

	#define logicError getLogicLog(ef::LOG_LEVEL_ERROR) << "[PID=" \
		<< getpid() << "] [THREAD=" \
		<< pthread_self() <<  "] [" << __FILE__ << ":" \
		<< __LINE__ << "] [" << __FUNCTION__ << "] "
};

#endif
