#include <string>
#include "baseconfig.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

CBaseConfig::CBaseConfig()
{
	m_GroupID = 0;
	m_OverTime = 0;
	m_PingTime = 0;
	m_IsOpenElapsedLog = false;

	m_ServerID = 0;
	m_ListenPort = 0;
	m_ServerType = 0;
	m_ServerIP.clear();

	m_LogServerID = 0;
	m_LogServerPort = 0;
	m_LogServerIP.clear();

	m_CenterServerID = 0;
	m_CenterServerPort = 0;
	m_CenterServerIP.clear();

	m_NameCheckServerID = 0;
	m_NameCheckServerPort = 0;
	m_NameCheckServerIP.clear();

	m_DBName.clear();
	m_DBUser.clear();
	m_DBPass.clear();
	m_DBIP.clear();

	m_MonitorPort = 0;
	m_ServerName.clear();
}
CBaseConfig::~CBaseConfig()
{
	m_ServerIP.clear();
	m_LogServerIP.clear();
	m_CenterServerIP.clear();
	m_NameCheckServerIP.clear();
	m_DBName.clear();
	m_DBUser.clear();
	m_DBPass.clear();
	m_DBIP.clear();
	m_ServerName.clear();
}

bool CBaseConfig::Init(const std::string &servername, int32 lineid)
{
	std::string filename = "./config/BaseConfig.xml";
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

	if (pBaseInfo->QueryIntAttribute("Group_ID", &m_GroupID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Group_ID'");
		return false;
	}

	if (m_GroupID < 0 || m_GroupID > 99)
	{
		log_error("Group_ID范围[0,99] ：%d", m_GroupID);
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

	m_LogServerID += GetGroupID() * 10;

	if (pBaseInfo->QueryIntAttribute("LogServer_Port", &m_LogServerPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'LogServer_Port'");
		return false;
	}

	m_LogServerPort += GetGroupID() * 10;

	m_LogServerIP = pBaseInfo->Attribute("LogServer_IP");
	if (m_LogServerIP.empty())
	{
		log_error("没有找到字段： 'LogServer_IP'");
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("CenterServer_ID", &m_CenterServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'CenterServer_ID'");
		return false;
	}

	m_CenterServerID += GetGroupID() * 10;

	if (pBaseInfo->QueryIntAttribute("CenterServer_Port", &m_CenterServerPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'CenterServer_Port'");
		return false;
	}

	m_CenterServerPort += GetGroupID() * 10;

	m_CenterServerIP = pBaseInfo->Attribute("CenterServer_IP");
	if (m_CenterServerIP.empty())
	{
		log_error("没有找到字段： 'CenterServer_IP'");
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("NameCheckServer_ID", &m_NameCheckServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'NameCheckServer_ID'");
		return false;
	}

	m_NameCheckServerID += GetGroupID() * 10;

	if (pBaseInfo->QueryIntAttribute("NameCheckServer_Port", &m_NameCheckServerPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'NameCheckServer_Port'");
		return false;
	}

	m_NameCheckServerPort += GetGroupID() * 10;

	m_NameCheckServerIP = pBaseInfo->Attribute("NameCheckServer_IP");
	if (m_NameCheckServerIP.empty())
	{
		log_error("没有找到字段： 'NameCheckServer_IP'");
		return false;
	}

	m_DBName = pBaseInfo->Attribute("DBName");
	if (m_DBName.empty())
	{
		log_error("没有找到字段： 'DBName'");
		return false;
	}

	m_DBUser = pBaseInfo->Attribute("DBUser");
	if (m_DBUser.empty())
	{
		log_error("没有找到字段： 'DBUser'");
		return false;
	}

	m_DBPass = pBaseInfo->Attribute("DBPass");
	if (m_DBPass.empty())
	{
		log_error("没有找到字段： 'DBPass'");
		return false;
	}

	m_DBIP = pBaseInfo->Attribute("DBIP");
	if (m_DBIP.empty())
	{
		log_error("没有找到字段： 'DBIP'");
		return false;
	}

	doc.Clear();
	std::string serverconfig = fmt::format("./config/{0}Config.xml", servername);
	if (doc.LoadFile(serverconfig.c_str()) != XML_SUCCESS)
	{
		log_error("加载 %s 失败!", serverconfig.c_str());
		return false;
	}

	XMLElement *pServerInfo = doc.FirstChildElement(servername.c_str());
	if (!pServerInfo)
	{
		log_error("没有找到节点：'%s'", servername.c_str());
		return false;
	}

	if (pServerInfo->QueryIntAttribute("Server_ID", &m_ServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Server_ID'");
		return false;
	}

	m_ServerID += GetGroupID() * 10 + lineid;

	if (pServerInfo->QueryIntAttribute("Listen_Port", &m_ListenPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Listen_Port'");
		return false;
	}

	m_ListenPort += GetGroupID() * 10 + lineid;
	
	m_ServerIP = pServerInfo->Attribute("Server_IP");
	if (m_ServerIP.empty())
	{
		log_error("没有找到字段： 'Server_IP'");
		return false;
	}

	SetServerName(servername.c_str());

	return true;
}