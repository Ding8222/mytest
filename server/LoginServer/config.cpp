#include <fstream>
#include "config.h"
#include "GlobalDefine.h"
#include "log.h"
#include "fmt/ostream.h"
#include "json.hpp"

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

	std::string filename = fmt::format("./config/{0}Config.json", servername);
	std::ifstream i(filename);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	nlohmann::json config = nlohmann::json::parse(i);

	if (config["ClientNum_Max"].is_null())
	{
		log_error("没有找到字段： 'ClientNum_Max'");
		return false;
	}
	m_MaxClientNum = config["ClientNum_Max"];

	if (config["DataLimt_Recv"].is_null())
	{
		log_error("没有找到字段： 'DataLimt_Recv'");
		return false;
	}
	m_RecvDataLimt = config["DataLimt_Recv"];

	if (config["DataLimt_Send"].is_null())
	{
		log_error("没有找到字段： 'DataLimt_Send'");
		return false;
	}
	m_SendDataLimt = config["DataLimt_Send"];

	if (config["ClientConnect_Log"].is_null())
	{
		log_error("没有找到字段： 'ClientConnect_Log'");
		return false;
	}
	m_IsOpenClientConnectLog = config["ClientConnect_Log"];

	return true;
}
