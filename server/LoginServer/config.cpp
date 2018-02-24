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
	m_MaxClientNum = 0;
	m_RecvDataLimt = 0;
	m_SendDataLimt = 0;
	m_IsOpenClientConnectLog = false;
}
CConfig::~CConfig()
{
	s_CenterServerIP.clear();
	m_CenterServerPort = 0;
	m_CenterServerID = 0;
	m_MaxClientNum = 0;
	m_RecvDataLimt = 0;
	m_SendDataLimt = 0;
	m_IsOpenClientConnectLog = false;
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_LOGIN);

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

	if (pinfo->QueryBoolAttribute("ClientConnect_Log", &m_IsOpenClientConnectLog) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'ClientConnect_Log'");
		return false;
	}

	return true;
}
