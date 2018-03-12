#include "GlobalDefine.h"
#include "config.h"
#include "tinyxml2.h"
#include "log.h"

using namespace tinyxml2;

CConfig::CConfig()
{
	m_DBID = 0;
}
CConfig::~CConfig()
{
	m_DBID = 0;
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_CENTER);

	const char *filename = "./config/serverconfig.xml";
	XMLDocument doc;
	if (doc.LoadFile(filename) != XML_SUCCESS)
	{
		log_error("加载 %s 失败!", filename);
		return false;
	}

	XMLElement *pinfo = doc.FirstChildElement(servername);
	if (!pinfo)
	{
		log_error("没有找到节点：'%s'", servername);
		return false;
	}

	if (pinfo->QueryIntAttribute("DBID", &m_DBID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'DBID'");
		return false;
	}

	return true;
}