#include "config.h"
#include "GlobalDefine.h"
#include "tinyxml2.h"
#include "log.h"

using namespace tinyxml2;

CConfig::CConfig()
{
	s_CenterServerIP.clear();
	m_CenterServerPort = 0;
	m_CenterServerID = 0;
}
CConfig::~CConfig()
{
	s_CenterServerIP.clear();
	m_CenterServerPort = 0;
	m_CenterServerID = 0;
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_GAME);

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

	s_CenterServerIP = pinfo->Attribute("CenterServer_IP");
	if (s_CenterServerIP.empty())
	{
		log_error("没有找到字段： 'CenterServer_IP'");
		return false;
	}

	if (pinfo->QueryIntAttribute("CenterServer_Port", &m_CenterServerPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'CenterServer_Port'");
		return false;
	}

	if (pinfo->QueryIntAttribute("CenterServer_ID", &m_CenterServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'CenterServer_ID'");
		return false;
	}
	return true;
}