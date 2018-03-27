#include "config.h"
#include "GlobalDefine.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

CConfig::CConfig()
{
	m_MaxClientNum = 0;
	m_RecvDataLimt = 0;
	m_SendDataLimt = 0;
	m_IsOpenClientConnectLog = false;
}
CConfig::~CConfig()
{
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

	return true;
}
