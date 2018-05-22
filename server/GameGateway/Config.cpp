#include <fstream>
#include "config.h"
#include "GlobalDefine.h"
#include "log.h"
#include "fmt/ostream.h"
#include "json.hpp"

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

	for (auto &i : m_GameSvrList)
	{
		delete i;
	}
	m_GameSvrList.clear();
}

bool CConfig::Init(const char *servername, int lineid)
{
	if (!CBaseConfig::Init(servername,lineid))
		return false;
	
	SetServerType(ServerEnum::EST_GATE);
	SetLineID(lineid);

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

	for (nlohmann::json::iterator iter = config["GameSvrList"].begin(); iter != config["GameSvrList"].end(); ++iter)
	{
		GameSvr *server = new GameSvr;

		if ((*iter)["IP"].is_null())
		{
			log_error("没有找到字段： 'IP'");
			delete server;
			return false;
		}
		server->ip = (*iter)["IP"].get<std::string>();
		
		if ((*iter)["PORT"].is_null())
		{
			log_error("没有找到字段： 'PORT'");
			delete server;
			return false;
		}
		server->port = (*iter)["PORT"];
		server->port += GetGroupID() * 10;

		if ((*iter)["ID"].is_null())
		{
			log_error("没有找到字段： 'ID'");
			delete server;
			return false;
		}
		server->id = (*iter)["ID"];
		server->id += GetGroupID() * 10;

		if ((*iter)["NAME"].is_null())
		{
			log_error("没有找到字段： 'NAME'");
			delete server;
			return false;
		}
		server->name = (*iter)["NAME"].get<std::string>();

		m_GameSvrList.push_back(server);
	}

	return true;
}
