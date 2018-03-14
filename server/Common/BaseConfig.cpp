#include <string>
#include "baseconfig.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

CBaseConfig::CBaseConfig()
{
	m_IsOpenElapsedLog = false;
	m_ListenPort = 0;
	m_OverTime = 0;
	m_PingTime = 0;
	m_ServerID = 0;
	m_ServerType = 0;
	memset(s_ServerIP, 0, sizeof(s_ServerIP));

	m_LogServerID = 0;
	m_LogServerPort = 0;
	memset(s_LogServerIP, 0, sizeof(s_ServerIP));
}
CBaseConfig::~CBaseConfig()
{
	m_IsOpenElapsedLog = false;
	m_ListenPort = 0;
	m_OverTime = 0;
	m_PingTime = 0;
	m_ServerID = 0;
	m_ServerType = 0;
	memset(s_ServerIP, 0, sizeof(s_ServerIP));

	m_LogServerID = 0;
	m_LogServerPort = 0;
	memset(s_LogServerIP, 0, sizeof(s_ServerIP));
}

bool CBaseConfig::Init(const std::string &servername, int lineid)
{
	std::string filename = fmt::format("./config/{0}Config.xml", servername);
	XMLDocument doc;
	if (doc.LoadFile(filename.c_str()) != XML_SUCCESS)
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	XMLElement *pBaseInfo = doc.FirstChildElement("Base");
	if (!pBaseInfo)
	{
		log_error("没有找到节点：'%s'", "Base");
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

	if (pBaseInfo->QueryBoolAttribute("Elapsed_Log", &m_IsOpenElapsedLog) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Elapsed_Log'");
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("LogServer_ID", &m_LogServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'LogServer_ID'");
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("LogServer_Port", &m_LogServerPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'LogServer_Port'");
		return false;
	}

	const char *LogServerIP = pBaseInfo->Attribute("LogServer_IP");
	if (!LogServerIP)
	{
		log_error("没有找到字段： 'LogServer_IP'");
		return false;
	}

	strncpy_s(s_LogServerIP, LogServerIP, MAX_SECRET_LEN - 1);
	s_LogServerIP[MAX_SECRET_LEN - 1] = '\0';
	if (strlen(s_LogServerIP) <= 1)
	{
		log_error("LogServer_IP 填写错误：%s", s_LogServerIP);
		return false;
	}

	XMLElement *pServerInfo = doc.FirstChildElement(servername.c_str());
	if (!pServerInfo)
	{
		log_error("没有找到节点：'%s'", servername.c_str());
		return false;
	}

	if (pServerInfo->QueryIntAttribute("Listen_Port", &m_ListenPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Listen_Port'");
		return false;
	}

	m_ListenPort += lineid;

	if (pServerInfo->QueryIntAttribute("Server_ID", &m_ServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Server_ID'");
		return false;
	}

	m_ServerID += lineid;

	const char *ServerIP = pServerInfo->Attribute("Server_IP");
	if (!ServerIP)
	{
		log_error("没有找到字段： 'Server_IP'");
		return false;
	}

	strncpy_s(s_ServerIP, ServerIP, MAX_SECRET_LEN - 1);
	s_ServerIP[MAX_SECRET_LEN - 1] = '\0';
	if (strlen(s_ServerIP) <= 1)
	{
		log_error("Server_IP 填写错误：%s", ServerIP);
		return false;
	}

	return true;
}