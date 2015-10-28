#ifndef _TOKEN_CHECK_IMPL_H_
#define _TOKEN_CHECK_IMPL_H_
#include <string>
#include <map>
#include <time.h>
#include <json/json.h>
#include "base/ef_btype.h"
#include "base/ef_loader.h"

namespace gim{
	class JsonWatcherBase;
	class ListWatcherBase;
	class TokenKeyBox;
	class ZKClient;

	class TokenKeyWatcher{
	public:
		TokenKeyWatcher();
		~TokenKeyWatcher();
		int init(TokenKeyBox* box, ZKClient* c, const std::string& path, int idx);
		int onDataChange(int ver, const Json::Value& v);
		const std::string& getData();
	private:
		TokenKeyBox* m_box;
		JsonWatcherBase* m_w;
		int m_idx;
	};
	typedef std::map<int, std::string> TKeyMap;
	typedef std::map<int, TokenKeyWatcher*> TKeyWchMap;
	typedef ef::DataSrc<TKeyMap> TKeyDataSrc;
	typedef ef::Loader<TKeyMap> TKeyLoader;

	class TokenKeyBox{
	public:
		TokenKeyBox();
		~TokenKeyBox();
		int init(ZKClient* c, const std::string kpath);
		int onKeyDataUpdate(int index, const std::string& data);
		int onChildrenChange(const std::map<std::string/*id*/, std::string>& c);
		TKeyDataSrc* getDataSrc();
	private:
		TKeyMap m_keys;
		TKeyWchMap m_wchs;
		TKeyDataSrc m_datasrc;
		ZKClient* m_zkc;
		std::string m_kpath;
		ListWatcherBase* m_w;
	};

	class ToekenCheckerImpl{
	public:
		enum
		{
			AUTH_OK = 0,
			AUTH_ENCFAIL = -1,
			AUTH_SIZE_ERROR = -100,
			AUTH_CHECKSUM_ERROR = -101,
			AUTH_TIMEOUT = -102,
			AUTH_INVALID_INDEX = -103,
			AUTH_DECFAIL = -104
		};
		static std::string errormsg(int code);
	public:
		ToekenCheckerImpl();
		~ToekenCheckerImpl();
		int init(TKeyDataSrc* src, int timeout);
		int generateToken(const std::map<std::string, std::string> &minfo, std::string &token);
		int checkToken(const std::string &token, std::map<std::string, std::string> &minfo);
		int poll();
	private:
		TKeyLoader m_loader;
		ef::uint16 m_curidx;
		std::string m_curkey;
		time_t m_tokentime;
		int m_timeout;
	};
}
#endif //_TOKEN_CHECK_IMPL_H_