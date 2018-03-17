#include "GlobalDefine.h"
#include "config.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

CConfig::CConfig()
{
	m_DBID = 0;

	m_NameCheckServerID = 0;
	m_NameCheckServerPort = 0;
	memset(s_NameCheckServerIP, 0, sizeof(s_NameCheckServerIP));
}
CConfig::~CConfig()
{
	m_DBID = 0;

	m_NameCheckServerID = 0;
	m_NameCheckServerPort = 0;
	memset(s_NameCheckServerIP, 0, sizeof(s_NameCheckServerIP));
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_CENTER);

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

	if (pinfo->QueryIntAttribute("DBID", &m_DBID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'DBID'");
		return false;
	}

	if (pinfo->QueryIntAttribute("NameCheckServer_ID", &m_NameCheckServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'NameCheckServer_ID'");
		return false;
	}

	if (pinfo->QueryIntAttribute("NameCheckServer_Port", &m_NameCheckServerPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'NameCheckServer_Port'");
		return false;
	}

	const char *NameCheckServerIP = pinfo->Attribute("NameCheckServer_IP");
	if (!NameCheckServerIP)
	{
		log_error("没有找到字段： 'NameCheckServer_IP'");
		return false;
	}

	strncpy_s(s_NameCheckServerIP, NameCheckServerIP, MAX_SECRET_LEN - 1);
	s_NameCheckServerIP[MAX_SECRET_LEN - 1] = '\0';
	if (strlen(s_NameCheckServerIP) <= 1)
	{
		log_error("NameCheckServer_IP 填写错误：%s", s_NameCheckServerIP);
		return false;
	}

	return true;
}