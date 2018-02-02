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

	s_GameServerIP.clear();
	m_GameServerPort = 0;
	m_GameServerID = 0;

	m_MaxClientNum = 0;
	m_RecvDataLimt = 0;
	m_SendDataLimt = 0;
}
CConfig::~CConfig()
{
	s_CenterServerIP.clear();
	m_CenterServerPort = 0;
	m_CenterServerID = 0;

	s_GameServerIP.clear();
	m_GameServerPort = 0;
	m_GameServerID = 0;

	m_MaxClientNum = 0;
	m_RecvDataLimt = 0;
	m_SendDataLimt = 0;
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_GATE);

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

	s_GameServerIP = pinfo->Attribute("GameServer_IP");
	if (s_GameServerIP.empty())
	{
		log_error("没有找到字段： 'GameServer_IP'");
		return false;
	}

	if (pinfo->QueryIntAttribute("GameServer_Port", &m_GameServerPort) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'GameServer_Port'");
		return false;
	}

	if (pinfo->QueryIntAttribute("GameServer_ID", &m_GameServerID) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'GameServer_ID'");
		return false;
	}

	if (pinfo->QueryIntAttribute("ClientNum_Max", &m_MaxClientNum) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'ClientNum_Max'");
		return false;
	}

	if (pinfo->QueryIntAttribute("DataLimt_Recv", &m_RecvDataLimt) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'DataLimt_Recv'");
		return false;
	}

	if (pinfo->QueryIntAttribute("DataLimt_Send", &m_SendDataLimt) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'DataLimt_Send'");
		return false;
	}
	return true;
}