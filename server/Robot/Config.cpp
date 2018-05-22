#include <fstream>
#include "config.h"
#include "GlobalDefine.h"
#include "log.h"
#include "fmt/ostream.h"
#include "json.hpp"

CConfig::CConfig()
{
	s_LoginServerIP.clear();
	m_LoginServerPort = 0;
	m_LoginServerID = 0;
	m_MaxRobot = 0;
	m_OverTime = 0;
	m_PingTime = 0;
}
CConfig::~CConfig()
{
	s_LoginServerIP.clear();
	m_LoginServerPort = 0;
	m_LoginServerID = 0;
	m_MaxRobot = 0;
	m_OverTime = 0;
	m_PingTime = 0;
}

bool CConfig::Init(const char *servername)
{
	std::string filename = fmt::format("./config/{0}Config.json", servername);
	std::ifstream i(filename);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	nlohmann::json config = nlohmann::json::parse(i);

	if (config["LoginServer_IP"].is_null())
	{
		log_error("没有找到字段： 'LoginServer_IP'");
		return false;
	}
	s_LoginServerIP = config["LoginServer_IP"].get<std::string>();

	if (config["LoginServer_Port"].is_null())
	{
		log_error("没有找到字段： 'LoginServer_Port'");
		return false;
	}
	m_LoginServerPort = config["LoginServer_Port"];

	if (config["LoginServer_ID"].is_null())
	{
		log_error("没有找到字段： 'LoginServer_ID'");
		return false;
	}
	m_LoginServerID = config["LoginServer_ID"];

	if (config["Robot_Max"].is_null())
	{
		log_error("没有找到字段： 'Robot_Max'");
		return false;
	}
	m_MaxRobot = config["Robot_Max"];

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

	return true;
}