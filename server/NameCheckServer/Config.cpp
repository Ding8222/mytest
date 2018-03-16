#include "GlobalDefine.h"
#include "config.h"
#include "tinyxml2.h"
#include "log.h"
#include "fmt/ostream.h"

using namespace tinyxml2;

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

	SetServerType(ServerEnum::EST_LOG);

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
	
	pinfo = doc.FirstChildElement("DBList");
	if (!pinfo)
	{
		log_error("没有找到节点：'DBList'");
		return false;
	}

	pinfo = pinfo->FirstChildElement("DBInfo");
	if (!pinfo)
	{
		log_error("没有找到字段： 'DBInfo'");
		return false;
	}

	while (pinfo)
	{
		DBInfo *server = new DBInfo;
		
		const char *tmp = pinfo->Attribute("DBName");
		if (!tmp)
		{
			log_error("没有找到字段： 'DBName'");
			delete server;
			return false;
		}
		server->dbname = tmp;

		tmp = pinfo->Attribute("DBUser");
		if (!tmp)
		{
			log_error("没有找到字段： 'DBUser'");
			delete server;
			return false;
		}
		server->dbusername = tmp;

		tmp = pinfo->Attribute("DBPass");
		if (!tmp)
		{
			log_error("没有找到字段： 'DBPass'");
			delete server;
			return false;
		}
		server->dbpassword = tmp;

		tmp = pinfo->Attribute("DBIP");
		if (!tmp)
		{
			log_error("没有找到字段： 'DBIP'");
			delete server;
			return false;
		}
		server->dbip = tmp;
		
		tmp = pinfo->Attribute("SQL");
		if (!tmp)
		{
			log_error("没有找到字段： 'SQL'");
			delete server;
			return false;
		}
		server->sqlstr = tmp;

		tmp = pinfo->Attribute("FieldName");
		if (!tmp)
		{
			log_error("没有找到字段： 'FieldName'");
			delete server;
			return false;
		}
		server->fieldname = tmp;

		m_tablelist.push_back(server);

		pinfo = pinfo->NextSiblingElement("DBInfo");
	}

	return true;
}