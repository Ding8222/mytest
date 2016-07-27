#include"mapconfig.h"
#include"tinyxml2.h"
#include"log.h"

using namespace tinyxml2;

mapconfig::mapconfig()
{
	m_maplist.clear();
}

mapconfig::~mapconfig()
{
	auto iterB = m_maplist.begin();
	auto iterE = m_maplist.end();
	for (; iterB != iterE; iterB++)
	{
		delete iterB->second;
	}

	m_maplist.clear();
}

bool mapconfig::init()
{
	const char *filename = "config/maplist.xml";
	XMLDocument doc;
	if (doc.LoadFile(filename) != XML_SUCCESS)
	{
		log_error("load %s failed!", filename);
		return false;
	}

	// todo
	// ��maplist.xml�е���Ϣ��ӵ�m_maplist

	return true;
}