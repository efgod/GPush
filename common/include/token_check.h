#ifndef _TOKEN_CHECKER_H_
#define _TOKEN_CHECKER_H_
#include <string>
#include <map>
#include <time.h>

namespace gim{
	class ToekenCheckerImpl;
	class TokenKeyBox;
	class ZKClient;

	class TokenChecker{
	public:
		static TokenChecker* s_ins;
		static TokenChecker* instance();
		static void destroy();
		static std::string errormsg(int code);
	public:
		int init(ZKClient*c, const std::string& kpath, int timeout);
		int generateToken(const std::map<std::string, std::string> &minfo, std::string &token);
		int checkToken(const std::string &token, std::map<std::string, std::string> &minfo);
	private:
		TokenChecker();
		~TokenChecker();
		ToekenCheckerImpl* getImpl();
	private:
		TokenKeyBox* m_kbox;
		int m_timeout;
	};
}
#endif //_TOKEN_CHECKER_H_
