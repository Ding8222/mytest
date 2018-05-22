#include <string>
#include <fstream>
#include "baseconfig.h"
#include "log.h"
#include "fmt/ostream.h"
#include "json.hpp"

/*
	端口说明:
	最高位为1,第二位为服务器类型,第三到第四位为一机多服编号,第五位为线路ID
*/

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
	m_ServerName.clear();

	m_LogServerID = 0;
	m_LogServerPort = 0;
	m_LogServerIP.clear();
	m_LogServerName.clear();

	m_CenterServerID = 0;
	m_CenterServerPort = 0;
	m_CenterServerIP.clear();
	m_CenterServerName.clear();

	m_NameCheckServerID = 0;
	m_NameCheckServerPort = 0;
	m_NameCheckServerIP.clear();
	m_NameCheckServerName.clear();

	m_DBName.clear();
	m_DBUser.clear();
	m_DBPass.clear();
	m_DBIP.clear();

	m_MonitorPort = 0;
}
CBaseConfig::~CBaseConfig()
{
	m_ServerIP.clear();
	m_ServerName.clear();
	m_LogServerIP.clear();
	m_LogServerName.clear();
	m_CenterServerIP.clear();
	m_CenterServerName.clear();
	m_NameCheckServerIP.clear();
	m_NameCheckServerName.clear();
	m_DBName.clear();
	m_DBUser.clear();
	m_DBPass.clear();
	m_DBIP.clear();
}

bool CBaseConfig::Init(const std::string &servername, int32 lineid)
{
	std::string filename = "./config/BaseConfig.json";
	std::ifstream i(filename);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	nlohmann::json config = nlohmann::json::parse(i);

	if (config["Group_ID"].is_null())
	{
		log_error("没有找到字段： 'Group_ID'");
		return false;
	}
	m_GroupID = config["Group_ID"];
	if (m_GroupID < 0 || m_GroupID > 99)
	{
		log_error("Group_ID范围[0,99] ：%d", m_GroupID);
		return false;
	}
	if (config["Over_Time"].is_null())
	{
		log_error("没有找到字段： 'Over_Time'");
		return false;
	}
	m_OverTime = config["Over_Time"];
	if (m_OverTime <= 0)
	{
		log_error("Over_Time小于等于0：%d", m_OverTime);
		return false;
	}
	if (config["Ping_Time"].is_null())
	{
		log_error("没有找到字段： 'Ping_Time'");
		return false;
	}
	m_PingTime = config["Ping_Time"];
	if (m_PingTime <= 0)
	{
		log_error("Ping_Time小于等于0：%d", m_PingTime);
		return false;
	}
	if (config["Elapsed_Log"].is_null())
	{
		log_error("没有找到字段： 'Elapsed_Log'");
		return false;
	}
	m_IsOpenElapsedLog = config["Elapsed_Log"];

	if (config["LogServer_ID"].is_null())
	{
		log_error("没有找到字段： 'LogServer_ID'");
		return false;
	}
	m_LogServerID = config["LogServer_ID"];
	m_LogServerID += GetGroupID() * 10;
	if (config["LogServer_Port"].is_null())
	{
		log_error("没有找到字段： 'LogServer_Port'");
		return false;
	}
	m_LogServerPort = config["LogServer_Port"];
	m_LogServerPort += GetGroupID() * 10;
	if (config["LogServer_IP"].is_null())
	{
		log_error("没有找到字段： 'LogServer_IP'");
		return false;
	}
	m_LogServerIP = config["LogServer_IP"].get<std::string>();
	if (m_LogServerIP.empty())
	{
		log_error("字段： 'LogServer_IP'为空");
		return false;
	}
	if (config["LogServer_Name"].is_null())
	{
		log_error("没有找到字段： 'LogServer_Name'");
		return false;
	}
	m_LogServerName = config["LogServer_Name"].get<std::string>();
	if (m_LogServerName.empty())
	{
		log_error("字段： 'LogServer_Name'为空");
		return false;
	}

	if (config["CenterServer_ID"].is_null())
	{
		log_error("没有找到字段： 'CenterServer_ID'");
		return false;
	}
	m_CenterServerID = config["CenterServer_ID"];
	m_CenterServerID += GetGroupID() * 10;
	if (config["CenterServer_Port"].is_null())
	{
		log_error("没有找到字段： 'CenterServer_Port'");
		return false;
	}
	m_CenterServerPort = config["CenterServer_Port"];
	m_CenterServerPort += GetGroupID() * 10;
	if (config["CenterServer_IP"].is_null())
	{
		log_error("没有找到字段： 'CenterServer_IP'");
		return false;
	}
	m_CenterServerIP = config["CenterServer_IP"].get<std::string>();
	if (m_CenterServerIP.empty())
	{
		log_error("字段： 'CenterServer_IP'为空");
		return false;
	}
	if (config["CenterServer_Name"].is_null())
	{
		log_error("没有找到字段： 'CenterServer_Name'");
		return false;
	}
	m_CenterServerName = config["CenterServer_Name"].get<std::string>();
	if (m_CenterServerName.empty())
	{
		log_error("字段： 'CenterServer_Name'为空");
		return false;
	}

	if (config["NameCheckServer_ID"].is_null())
	{
		log_error("没有找到字段： 'NameCheckServer_ID'");
		return false;
	}
	m_NameCheckServerID = config["NameCheckServer_ID"];
	if (config["NameCheckServer_Port"].is_null())
	{
		log_error("没有找到字段： 'NameCheckServer_Port'");
		return false;
	}
	m_NameCheckServerPort = config["NameCheckServer_Port"];
	if (config["NameCheckServer_IP"].is_null())
	{
		log_error("没有找到字段： 'NameCheckServer_IP'");
		return false;
	}
	m_NameCheckServerIP = config["NameCheckServer_IP"].get<std::string>();
	if (m_NameCheckServerIP.empty())
	{
		log_error("字段： 'NameCheckServer_IP'为空");
		return false;
	}
	if (config["NameCheckServer_Name"].is_null())
	{
		log_error("没有找到字段： 'NameCheckServer_Name'");
		return false;
	}
	m_NameCheckServerName = config["NameCheckServer_Name"].get<std::string>();
	if (m_NameCheckServerName.empty())
	{
		log_error("字段： 'NameCheckServer_Name'为空");
		return false;
	}

	if (config["DBName"].is_null())
	{
		log_error("没有找到字段： 'DBName'");
		return false;
	}
	m_DBName = config["DBName"].get<std::string>();
	if (m_DBName.empty())
	{
		log_error("字段： 'DBName'为空");
		return false;
	}
	if (config["DBUser"].is_null())
	{
		log_error("没有找到字段： 'DBUser'");
		return false;
	}
	m_DBUser = config["DBUser"].get<std::string>();
	if (m_DBUser.empty())
	{
		log_error("字段： 'DBUser'为空");
		return false;
	}
	if (config["DBPass"].is_null())
	{
		log_error("没有找到字段： 'DBPass'");
		return false;
	}
	m_DBPass = config["DBPass"].get<std::string>();
	if (m_DBPass.empty())
	{
		log_error("字段： 'DBPass'为空");
		return false;
	}
	if (config["DBIP"].is_null())
	{
		log_error("没有找到字段： 'DBIP'");
		return false;
	}
	m_DBIP = config["DBIP"].get<std::string>();
	if (m_DBIP.empty())
	{
		log_error("字段： 'DBIP'为空");
		return false;
	}

	i.close();
	config.clear();
	std::string serverconfig = fmt::format("./config/{0}Config.json", servername);
	i.open(serverconfig);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", serverconfig.c_str());
		return false;
	}

	config = nlohmann::json::parse(i);

	if (config["Server_ID"].is_null())
	{
		log_error("没有找到字段： 'Server_ID'");
		return false;
	}
	m_ServerID = config["Server_ID"];
	m_ServerID += GetGroupID() * 10 + lineid;
	if (config["Listen_Port"].is_null())
	{
		log_error("没有找到字段： 'Listen_Port'");
		return false;
	}
	m_ListenPort = config["Listen_Port"];
	m_ListenPort += GetGroupID() * 10 + lineid;
	if (config["Server_IP"].is_null())
	{
		log_error("没有找到字段： 'Server_IP'");
		return false;
	}
	m_ServerIP = config["Server_IP"].get<std::string>();
	if (m_ServerIP.empty())
	{
		log_error("字段： 'Server_IP'为空");
		return false;
	}

	SetMonitorPort(m_ListenPort + 10000);
	SetServerName(servername.c_str());

	i.close();
	config.clear();

	return true;
}