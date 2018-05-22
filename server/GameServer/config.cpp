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
}
CConfig::~CConfig()
{
	m_LineID = 0;
	m_MaxClientNum = 0;
}

bool CConfig::Init(const char *servername, int lineid)
{
	if (!CBaseConfig::Init(servername, lineid))
		return false;

	SetServerType(ServerEnum::EST_GAME);
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

	return true;
}