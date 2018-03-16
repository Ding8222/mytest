#pragma once
#include <string>
#include <list>
#include"BaseConfig.h"

struct TableInfo 
{
	TableInfo()
	{

	}

	std::string dbname;
	std::string dbusername;
	std::string dbpassword;
	std::string dbip;
	std::string sqlstr;
	std::string fieldname;
};

class CConfig :public CBaseConfig
{
public:
	CConfig();
	~CConfig();

	static CConfig &Instance()
	{
		static CConfig m;
		return m;
	}

	bool Init(const char *servername = "");

	std::list<TableInfo *> &GetTableList() { return m_tablelist; }
private:

	std::list<TableInfo *> m_tablelist;
};