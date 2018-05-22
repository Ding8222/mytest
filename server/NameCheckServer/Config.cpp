#include <fstream>
#include "GlobalDefine.h"
#include "config.h"
#include "log.h"
#include "fmt/ostream.h"
#include "json.hpp"

CConfig::CConfig()
{
	m_tablelist.clear();
}
CConfig::~CConfig()
{
	for (std::list<DBInfo *>::iterator itr = m_tablelist.begin(); itr != m_tablelist.end(); ++itr)
	{
		delete (*itr);
	}
	m_tablelist.clear();
}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_NAME);

	std::string filename = fmt::format("./config/{0}Config.json", servername);
	std::ifstream i(filename);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	nlohmann::json config = nlohmann::json::parse(i);
	
	for (nlohmann::json::iterator iter = config["DBList"].begin(); iter != config["DBList"].end(); ++iter)
	{
		DBInfo *server = new DBInfo;

		if ((*iter)["DBName"].is_null())
		{
			log_error("没有找到字段： 'DBName'");
			delete server;
			return false;
		}
		server->dbname = (*iter)["DBName"].get<std::string>();

		if ((*iter)["DBUser"].is_null())
		{
			log_error("没有找到字段： 'DBUser'");
			delete server;
			return false;
		}
		server->dbusername = (*iter)["DBUser"].get<std::string>();

		if ((*iter)["DBPass"].is_null())
		{
			log_error("没有找到字段： 'DBPass'");
			delete server;
			return false;
		}
		server->dbpassword = (*iter)["DBPass"].get<std::string>();

		if ((*iter)["DBIP"].is_null())
		{
			log_error("没有找到字段： 'DBIP'");
			delete server;
			return false;
		}
		server->dbip = (*iter)["DBIP"].get<std::string>();

		if ((*iter)["SQL"].is_null())
		{
			log_error("没有找到字段： 'SQL'");
			delete server;
			return false;
		}
		server->sqlstr = (*iter)["SQL"].get<std::string>();

		if ((*iter)["FieldName"].is_null())
		{
			log_error("没有找到字段： 'FieldName'");
			delete server;
			return false;
		}
		server->fieldname = (*iter)["FieldName"].get<std::string>();

		m_tablelist.push_back(server);
	}

	return true;
}