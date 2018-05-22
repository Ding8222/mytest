#include <fstream>
#include "GlobalDefine.h"
#include "config.h"
#include "log.h"
#include "fmt/ostream.h"
#include "json.hpp"

CConfig::CConfig()
{
	m_DBID = 0;

}
CConfig::~CConfig()
{
	m_DBID = 0;

}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_CENTER);

	std::string filename = fmt::format("./config/{0}Config.json", servername);
	std::ifstream i(filename);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	nlohmann::json config = nlohmann::json::parse(i);

	if (config["DBID"].is_null())
	{
		log_error("没有找到字段： 'DBID'");
		return false;
	}

	m_DBID = config["DBID"];

	return true;
}