#include "config.h"
#include "GlobalDefine.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

CConfig::CConfig()
{
	m_LineID = 0;

	m_MaxClientNum = 0;
	m_RecvDataLimt = 0;
	m_SendDataLimt = 0;
	m_IsOpenClientConnectLog = false;

	m_GameSvrList.clear();
}
CConfig::~CConfig()
{
	m_LineID = 0;

	m_MaxClientNum = 0;
	m_RecvDataLimt = 0;
	m_SendDataLimt = 0;
	m_IsOpenClientConnectLog = false;

	m_GameSvrList.clear();
}

bool CConfig::Init(const char *servername, int lineid)
{
	if (!CBaseConfig::Init(servername,lineid))
		return false;
	
	SetServerType(ServerEnum::EST_GATE);
	SetLineID(lineid);

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

	pinfo = doc.FirstChildElement("GameSvrList");
	if (!pinfo)
	{
		log_error("没有找到节点：'GameSvrList'");
		return false;
	}

	pinfo = pinfo->FirstChildElement("GameSvr");
	if (!pinfo)
	{
		log_error("没有找到字段： 'GameSvr'");
		return false;
	}

	while (pinfo)
	{
		GameSvr server;

		server.ip = pinfo->Attribute("IP");
		if (server.ip.empty())
		{
			log_error("没有找到字段： 'IP'");
			return false;
		}

		if (pinfo->QueryIntAttribute("PORT", &server.port) != XML_SUCCESS)
		{
			log_error("没有找到字段： 'PORT'");
			return false;
		}

		server.port += GetGroupID() * 10;

		if (pinfo->QueryIntAttribute("ID", &server.id) != XML_SUCCESS)
		{
			log_error("没有找到字段： 'ID'");
			return false;
		}

		server.id += GetGroupID() * 10;

		server.name = pinfo->Attribute("NAME");
		if (server.name.empty())
		{
			log_error("没有找到字段： 'NAME'");
			return false;
		}

		m_GameSvrList.push_back(server);

		pinfo = pinfo->NextSiblingElement("GameSvr");
	}

	return true;
}
