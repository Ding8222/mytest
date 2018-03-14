#include "config.h"
#include "GlobalDefine.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

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

	s_LoginServerIP = pinfo->Attribute("LoginServer_IP");
	if (s_LoginServerIP.empty())
	{
		log_error("没有找到字段： 'LoginServer_IP'");
		return false;
	}

	if (pinfo->QueryIntAttribute("LoginServer_Port", &m_LoginServerPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'LoginServer_Port'");
		return false;
	}

	if (pinfo->QueryIntAttribute("LoginServer_ID", &m_LoginServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'LoginServer_ID'");
		return false;
	}

	if (pinfo->QueryIntAttribute("Robot_Max", &m_MaxRobot) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Robot_Max'");
		return false;
	}

	if (pinfo->QueryIntAttribute("Over_Time", &m_OverTime) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Over_Time'");
		return false;
	}

	if (m_OverTime <= 0)
	{
		log_error("Over_Time小于等于0：%d", m_OverTime);
		return false;
	}

	if (pinfo->QueryIntAttribute("Ping_Time", &m_PingTime) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'Ping_Time'");
		return false;
	}

	if (m_PingTime <= 0)
	{
		log_error("Ping_Time小于等于0：%d", m_PingTime);
		return false;
	}

	return true;
}