#include "GlobalDefine.h"
#include "config.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

CConfig::CConfig()
{
	m_SQLLog = false;
}
CConfig::~CConfig()
{
	m_SQLLog = false;
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_LOG);

	std::string filename = fmt::format("./config/{0}Config.xml", servername);
	XMLDocument doc;
	if (doc.LoadFile(filename.c_str()) != XML_SUCCESS)
	{
		log_error("���� %s ʧ��!", filename.c_str());
		return false;
	}

	XMLElement *pinfo = doc.FirstChildElement(servername);
	if (!pinfo)
	{
		log_error("û���ҵ��ڵ㣺'%s'", servername);
		return false;
	}

	if (pinfo->QueryBoolAttribute("SQL_Log", &m_SQLLog) != XML_SUCCESS)
	{
		log_error("û���ҵ��ֶΣ� 'SQL_Log'");
		return false;
	}

	return true;
}