#include <fstream>
#include "GlobalDefine.h"
#include "config.h"
#include "log.h"
#include "fmt/ostream.h"
#include "json.hpp"

CConfig::CConfig()
{
	m_SQLLog = false;
}
CConfig::~CConfig()
{
	m_SQLLog = false;
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_LOG);

	std::string filename = fmt::format("./config/{0}Config.json", servername);
	std::ifstream i(filename);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	nlohmann::json config = nlohmann::json::parse(i);

	if (config["SQL_Log"].is_null())
	{
		log_error("没有找到字段： 'SQL_Log'");
		return false;
	}
	m_SQLLog = config["SQL_Log"];

	return true;
}