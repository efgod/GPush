#include "token_check_impl.h"
#include "data_watcher.h"
#include "list_watcher.h"
#include <sstream>
#include "base/ef_aes.h"
#include "base/ef_base64.h"
#include "base/ef_utility.h"
#include "base/ef_tsd_ptr.h"
#include <iostream>
using namespace std;
namespace gim{
	TokenKeyWatcher::TokenKeyWatcher()
		:m_box(NULL), m_w(NULL){

	}
	TokenKeyWatcher::~TokenKeyWatcher(){
		if (m_w){
			delete m_w;
			m_w = NULL;
		}
	}
	int TokenKeyWatcher::init(TokenKeyBox* box, ZKClient* c, const string& path, int idx){
		m_box = box;
		m_idx = idx;
		m_w = new JsonWatcher<TokenKeyWatcher>(this, &TokenKeyWatcher::onDataChange);
		return m_w->init(c, path);
	}
	int TokenKeyWatcher::onDataChange(int ver, const Json::Value& v){
		if (v["key"].isString()){
			m_box->onKeyDataUpdate(m_idx, v["key"].asString());
		}
		return 0;
	}
	const string& TokenKeyWatcher::getData(){
		return m_w->getData();
	}
	TokenKeyBox::TokenKeyBox()
		:m_w(NULL){

	}
	TokenKeyBox::~TokenKeyBox(){
		if (m_w){
			delete m_w;
			m_w = NULL;
		}
	}
	int TokenKeyBox::init(ZKClient* c, const string kpath){
		m_zkc = c;
		m_kpath = kpath;
		m_w = new ListWatcher<TokenKeyBox>(this, &TokenKeyBox::onChildrenChange);
		if (m_w->init(c, kpath, false) < 0)
			return -1;

		return 0;
	}
	int TokenKeyBox::onKeyDataUpdate(int index, const string& data){
		TKeyMap::iterator it = m_keys.find(index);
		if (it != m_keys.end() && it->second != data){
			it->second = data;
			m_datasrc.setData(m_keys);
		}
		return 0;
	}
	int TokenKeyBox::onChildrenChange(const  map<string, string>& c){
		bool change = false;
		for (map<string, string>::const_iterator cit = c.begin();
			cit != c.end(); cit++){

			string child = cit->first;
			int idx = atoi(cit->first.c_str());
			if (m_keys.find(idx) == m_keys.end()){
				TokenKeyWatcher* z = new TokenKeyWatcher();
				if (z->init(this, m_zkc, m_kpath + "/" + child, idx) >= 0){
					m_wchs[idx] = z;
					try{
						Json::Value v;
						if (Json::Reader().parse(z->getData(), v) && v["key"].isString()){
							change = true;
							m_keys[idx] = v["key"].asString();
						}
					}
					catch (const exception& e){

					}
				}
			}
		}
		if (change){
			m_datasrc.setData(m_keys);
		}
		return 0;
	}
	TKeyDataSrc* TokenKeyBox::getDataSrc(){
		return &m_datasrc;
	}
	int ToekenCheckerImpl::poll(){
		time_t now = time(NULL);
		if (now - m_tokentime> m_timeout){
			m_loader.loadData();
			const TKeyMap& keys = m_loader.getData();
			if (keys.empty()){
				return -1;
			}

			TKeyMap::const_iterator it = keys.upper_bound(m_curidx);
			if (it == keys.end()){
				it = keys.begin();
			}

			m_curidx = it->first;
			m_curkey = it->second;
			m_tokentime = now;
		}
		return 0;
	}
	string ToekenCheckerImpl::errormsg(int code){
		switch (code) {
		case AUTH_OK:
			return "OK";
		case AUTH_ENCFAIL:
			return "encrypt fail";
		case AUTH_SIZE_ERROR:
			return "invalid token head";
		case AUTH_CHECKSUM_ERROR:
			return "token checksum do not match";
		case AUTH_TIMEOUT:
			return "token timeout";
		case AUTH_INVALID_INDEX:
			return "invalid key index";
		case AUTH_DECFAIL:
			return "decrypt fail";
		default:
			return "unknown error";
		}
	}
	static ef::uint64 seed = 0x0123456789abcdef;
	ToekenCheckerImpl::ToekenCheckerImpl()
		:m_loader(NULL),
		m_curidx(-1),
		m_tokentime(0),
		m_timeout(7200){
	}
	ToekenCheckerImpl::~ToekenCheckerImpl(){};
	int ToekenCheckerImpl::init(TKeyDataSrc* src, int timeout){
		m_loader.setSrc(src);
		m_timeout = timeout;
		return 0;
	}
	int ToekenCheckerImpl::generateToken(const map<string, string> &minfo, string &token)
	{
		//poll();
		time_t now = time(NULL);
		const string& key = m_curkey;
		stringstream ss;
		map<string, string>::const_iterator it = minfo.begin();
		for (; it != minfo.end(); it++)  {
			ss << it->first << "=" << it->second << "&";
			std::cout << "##key=" << it->first << ", v=" << it->second << std::endl;
		}
		string enctext;

		if (ef::aesEncrypt(ss.str(), key, enctext) < 0) {
			return AUTH_ENCFAIL;
		}
		string str;
		str.append((char *)&now, sizeof(now));
		str.append((char *)&m_curidx, sizeof(m_curidx));
		ef::uint64 checksum = now ^ m_curidx ^ seed;
		string padstr;
		padstr.append((char *)&enctext[0], enctext.size());
		if (enctext.size() % sizeof(checksum)) {
			padstr.append(((enctext.size() / sizeof(checksum)) + 1) * sizeof(checksum)-enctext.size(), '\0');
		}
		for (unsigned int i = 0; i < padstr.size(); i += sizeof(checksum)) {
			checksum ^= *((ef::uint64 *)&padstr[i]);
		}
		str.append((char *)&checksum, sizeof(checksum));
		str.append((char *)&enctext[0], enctext.size());

		token = ef::base64Encode(str);

		std::cout << token << std::endl;

		map<string, string> mm;
		checkToken(token, mm);
		for (map<string, string>::const_iterator it = mm.begin(); it != mm.end();it++)
		{
			std::cout << "key=" << it->first << ", v=" << it->second << std::endl;
		}
		return 0;
	}
	int ToekenCheckerImpl::checkToken(const string &token, map<string, string> &minfo)
	{
		string decstr = ef::base64Decode(token);
		if (decstr.size() <= sizeof(time_t)+sizeof(ef::uint16)+sizeof(ef::uint64)) {
			return AUTH_SIZE_ERROR;
		}

		time_t tm = *((time_t *)&decstr[0]);
		ef::uint16 idx = *((ef::uint16 *)&decstr[sizeof(tm)]);
		ef::uint64 cks = *((ef::uint64 *)&decstr[sizeof(tm)+sizeof(idx)]);


		cks ^= tm ^ idx ^ seed;
		string padstr;
		padstr.append((char *)&decstr[sizeof(tm)+sizeof(idx)+sizeof(cks)],
			decstr.size() - sizeof(tm)-sizeof(idx)-sizeof(cks));
		if (padstr.size() % sizeof(cks)) {
			padstr.append(((padstr.size() / sizeof(cks)) + 1) * sizeof(cks)-padstr.size(), '\0');
		}
		for (unsigned int i = 0; i < padstr.size(); i += sizeof(cks)) {
			cks ^= *((ef::uint64 *)&padstr[i]);
		}
		if (cks) {
			return AUTH_CHECKSUM_ERROR;
		}

		if (time(NULL) - tm > m_timeout) {
			return AUTH_TIMEOUT;
		}

		const TKeyMap& keys = m_loader.getData();
		TKeyMap::const_iterator cit = keys.find(idx);
		if (cit == keys.end()) {
			return AUTH_INVALID_INDEX;
		}

		string rawbody;
		if (ef::aesDecrypt(&decstr[sizeof(tm)+sizeof(idx)+sizeof(cks)],
			decstr.size() - sizeof(tm)-sizeof(idx)-sizeof(cks),
			cit->second, rawbody) < 0) {
			return AUTH_DECFAIL;
		}

		vector<string> vstr;
		ef::split(rawbody, vstr, "&");
		vector<string>::iterator it = vstr.begin();
		for (; it != vstr.end(); it++) {
			string::size_type pos = it->find("=");
			if (pos != string::npos && pos < it->size() - 1) {
				minfo[it->substr(0, pos)] = it->substr(pos + 1);
			}
		}
		return 0;
	}
}