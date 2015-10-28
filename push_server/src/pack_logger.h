#ifndef __PACK_LOGGER_H__
#define __PACK_LOGGER_H__


namespace gim{

class SVCon;

class PackLogger{
public:
	PackLogger(SVCon* s);
	~PackLogger();
private:
	SVCon* m_s;
};

}

#endif/*__PACK_LOGGER_H__*/
