#include "config.h"
#include "GlobalDefine.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

CConfig::CConfig()
{
	m_LineID = 0;
	m_MaxClientNum = 0;
}
CConfig::~CConfig()
{
	m_LineID = 0;
	m_MaxClientNum = 0;
}

bool CConfig::Init(const char *servername, int lineid)
{
	if (!CBaseConfig::Init(servername, lineid))
		return false;

	SetServerType(ServerEnum::EST_GAME);
	SetLineID(lineid);

	std::string filename = fmt::format("./config/{0}Config.xml", servername);
	XMLDocument doc;
	if (doc.LoadFile(filename.c_str()) != XML_SUCCESS)
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	XMLElement *pinfo = doc.FirstChildElement(servername);
	if (!pinfo)
	{
		log_error("没有找到节点：'%s'", servername);
		return false;
	}

	if (pinfo->QueryIntAttribute("ClientNum_Max", &m_MaxClientNum) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'ClientNum_Max'");
		return false;
	}

	return true;
}