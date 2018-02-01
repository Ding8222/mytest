#include"baseconfig.h"
#include"tinyxml2.h"
#include"log.h"

using namespace tinyxml2;

CBaseConfig::CBaseConfig()
{
	m_ListenPort = 0;
	m_OverTime = 0;
	m_PingTime = 0;
	m_ServerID = 0;
}
CBaseConfig::~CBaseConfig()
{
	m_ListenPort = 0;
	m_OverTime = 0;
	m_PingTime = 0;
	m_ServerID = 0;
}

bool CBaseConfig::Init(const char *servername)
{
	const char *filename = "./config/serverconfig.xml";
	XMLDocument doc;
	if (doc.LoadFile(filename) != XML_SUCCESS)
	{
		log_error("加载 %s 失败!", filename);
		return false;
	}

	XMLElement *pBaseInfo = doc.FirstChildElement("Base");
	if (!pBaseInfo)
	{
		log_error("没有找到节点：'%s'", servername);
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("Over_Time", &m_OverTime) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Over_Time'");
		return false;
	}

	if (m_OverTime <= 0)
	{
		log_error("Over_Time小于等于0：%d", m_OverTime);
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("Ping_Time", &m_PingTime) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Ping_Time'");
		return false;
	}

	if (m_PingTime <= 0)
	{
		log_error("Ping_Time小于等于0：%d", m_PingTime);
		return false;
	}

	XMLElement *pServerInfo = doc.FirstChildElement(servername);
	if (!pServerInfo)
	{
		log_error("没有找到节点：'%s'", servername);
		return false;
	}

	if (pServerInfo->QueryIntAttribute("Listen_Port", &m_ListenPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Listen_Port'");
		return false;
	}

	if (pServerInfo->QueryIntAttribute("Server_ID", &m_ServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Server_ID'");
		return false;
	}

	s_ServerIP = pServerInfo->Attribute("Server_IP");
	if (s_ServerIP.empty())
	{
		log_error("没有找到字段： 'Server_IP'");
		return false;
	}

	return true;
}