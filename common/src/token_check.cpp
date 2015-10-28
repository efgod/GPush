#include "token_check.h"
#include "token_check_impl.h"
#include "data_watcher.h"
#include "list_watcher.h"
#include <sstream>
#include "base/ef_aes.h"
#include "base/ef_base64.h"
#include "base/ef_utility.h"
#include "base/ef_tsd_ptr.h"
using namespace std;
namespace gim{
	static ef::TSDPtr<ToekenCheckerImpl> g_tcher;
	int TokenChecker::init(ZKClient*c, const string& kpath, int timeout){
		m_kbox = new TokenKeyBox();
		if (m_kbox->init(c, kpath) < 0){
			return -1;
		}
		return 0;
	}
	int TokenChecker::generateToken(const map<string, string> &minfo, string &token){
		ToekenCheckerImpl* chcker = getImpl();
		if (chcker){
			return chcker->generateToken(minfo, token);
		}
		return -1;
	}
	int TokenChecker::checkToken(const string &token, map<string, string> &minfo){
		ToekenCheckerImpl* chcker = getImpl();
		if (chcker){
			return chcker->checkToken(token, minfo);
		}
		return -1;
	}
	TokenChecker::TokenChecker()
		:m_kbox(NULL),
		m_timeout(7200){

	}
	TokenChecker::~TokenChecker(){
		if (m_kbox){
			delete m_kbox;
			m_kbox = NULL;
		}
	}

	ToekenCheckerImpl* TokenChecker::getImpl(){
		ToekenCheckerImpl* chcker = g_tcher.get();
		if (!chcker){
			chcker = new ToekenCheckerImpl();
			if (chcker->init(m_kbox->getDataSrc(), m_timeout) < 0){
				delete chcker;
				return NULL;
			}
			g_tcher.set(chcker);
		}
		chcker->poll();
		return chcker;
	}
	TokenChecker* TokenChecker::s_ins = NULL;
	TokenChecker* TokenChecker::instance(){
		if (!s_ins){
			s_ins = new TokenChecker();
		}
		return s_ins;
	}
	void TokenChecker::destroy(){
		if (s_ins){
			delete s_ins;
			s_ins = NULL;
		}
	}
	std::string TokenChecker::errormsg(int code){
		return ToekenCheckerImpl::errormsg(code);
	}
}