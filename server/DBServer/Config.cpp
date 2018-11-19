#include <fstream>
#include "GlobalDefine.h"
#include "log.h"
#include "fmt/ostream.h"
#include "json.hpp"
#include "config.h"

CConfig::CConfig()
{
	m_BeginMapID = 0;
	m_SQLLog = false;
}
CConfig::~CConfig()
{
	m_BeginMapID = 0;
	m_SQLLog = false;
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_DB);

	std::string filename = fmt::format("./config/{0}Config.json", servername);
	std::ifstream i(filename);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	nlohmann::json config = nlohmann::json::parse(i);

	m_BeginMapID = config["Begin_MapID"];
	if (m_BeginMapID <= 0)
	{
		log_error("Begin_MapID小于等于0：%d", m_BeginMapID);
		return false;
	}

	m_SQLLog = config["SQL_Log"];

	i.clear();
	i.close();
	config.clear();
	return true;
}