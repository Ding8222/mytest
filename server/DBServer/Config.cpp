#include "config.h"
#include "GlobalDefine.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

CConfig::CConfig()
{
	m_BeginMapID = 0;
	m_SQLLog = false;
}
CConfig::~CConfig()
{
	m_BeginMapID = 0;
	m_SQLLog = false;
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_DB);

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

	if (pinfo->QueryIntAttribute("Begin_MapID", &m_BeginMapID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Begin_MapID'");
		return false;
	}

	if (m_BeginMapID <= 0)
	{
		log_error("Begin_MapID小于等于0：%d", m_BeginMapID);
		return false;
	}

	if (pinfo->QueryBoolAttribute("SQL_Log", &m_SQLLog) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'SQL_Log'");
		return false;
	}

	return true;
}