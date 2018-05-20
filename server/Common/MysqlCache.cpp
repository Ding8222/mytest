﻿#include <list>
#include <sstream>
#include <iostream>
#include "MysqlCache.h"
#include "fmt/ostream.h"
#include "ServerLog.h"
#include "Timer.h"
#include "crosslib.h"

extern DataBase::CConnection g_dbhand;

CMysqlCache::CMysqlCache()
{
	m_Schema.clear();
	m_Con = nullptr;
	m_DBName.clear();
	m_CacheData.clear();
}

CMysqlCache::~CMysqlCache()
{
	Destroy();
}

bool CMysqlCache::Init(const std::string &dbname, DataBase::CConnection *con)
{
	m_DBName = dbname;
	m_Con = con;

	if (!LoadSchema())
	{
		RunStateError("加载表结构失败!");
		return false;
	}
	
	m_CacheData.reserve(m_Schema.size());
	
	m_DBTableConfig = {
		{ "account" ,
			{
				{ "table_name","account" },
				{ "cache_key","account" },
			}		
		},
		{ "playerdate" ,
			{
				{ "table_name","playerdate" },
				{ "cache_key","account" },
			}
		}
	};

	CTimer::UpdateTime();
	int64 starttime = CTimer::GetTime();
	LoadData(m_DBTableConfig["account"]);
	LoadData(m_DBTableConfig["playerdate"]);

	CTimer::UpdateTime();
	RunStateLog("加载Mysql耗时：%d秒", CTimer::GetTime() - starttime);

	int64 time = get_millisecond();

	nlohmann::json reault = ExecuteSingle("account","RoBot_3692");

	RunStateLog("查找耗时：%d ms", get_millisecond() - time);

	time = get_millisecond();
	DataBase::CRecordset *res = m_Con->Execute(fmt::format("select * from account where account = '{0}' limit 1","RoBot_3692").c_str());
	RunStateLog("查找2耗时：%d ms", get_millisecond() - time);

	return true;
}

void CMysqlCache::Destroy()
{
	m_Schema.clear();
	m_Con = nullptr;
	m_DBName.clear();
	m_CacheData.clear();
}

const char *CMysqlCache::GetPrimaryKey(const std::string &tablename)
{
	DataBase::CRecordset *res = m_Con->Execute(
		fmt::format("select k.column_name\
			from information_schema.table_constraints t\
			join information_schema.key_column_usage k\
			using (constraint_name,table_schema,table_name)\
			where t.constraint_type = 'PRIMARY KEY'\
			and t.table_schema= '{0}'\
			and t.table_name = '{1}'",
			m_DBName, tablename).c_str());
	if (res && res->IsOpen() && !res->IsEnd())
	{
		return res->GetChar("column_name");
	}
	return nullptr;
}

std::list<std::string> CMysqlCache::GetFields(const std::string &tablename)
{
	std::list<std::string> test;
	DataBase::CRecordset *res = m_Con->Execute(
		fmt::format("select column_name from information_schema.columns where table_schema = '{0}' and table_name = '{1}'",
			m_DBName, tablename).c_str());
	if (res && res->IsOpen() && !res->IsEnd())
	{
		while (!res->IsEnd())
		{
			test.push_back(res->GetChar("column_name"));
			res->NextRow();
		}
	}
	return test;
}

const char *CMysqlCache::GetFieldType(const std::string &tablename, const std::string &field)
{
	DataBase::CRecordset *res = m_Con->Execute(
		fmt::format("select data_type from information_schema.columns where table_schema='{0}' and table_name='{1}' and column_name='{2}'",
			m_DBName, tablename, field).c_str());
	if (res && res->IsOpen() && !res->IsEnd())
	{
		return res->GetChar("data_type");
	}
	return nullptr;
}

bool CMysqlCache::LoadSchema()
{
	std::list<std::string> tablename;
	DataBase::CRecordset *res = m_Con->Execute(
		fmt::format("select table_name from information_schema.tables where table_schema='{0}'",
			m_DBName).c_str());
	if (res && res->IsOpen() && !res->IsEnd())
	{
		// 查询到的信息
		while (!res->IsEnd())
		{
			tablename.push_back(res->GetChar("table_name"));
			res->NextRow();
		}
	}

	for(auto &i: tablename)
	{
		m_Schema[i] = {};
		m_Schema[i]["fields"] = {};
		m_Schema[i]["fieldtype"] = {};
		m_Schema[i]["pk"] = GetPrimaryKey(i);

		std::list<std::string> fields = GetFields(i);
		m_Schema[i]["fields"] = fields;
		for (auto &j : fields)
		{
			std::string fieldtype = GetFieldType(i, j);
			if (fieldtype == "float")
			{
				m_Schema[i]["fieldtype"][j] = "float";
			}
			else if (fieldtype == "double")
			{
				m_Schema[i]["fieldtype"][j] = "double";
			}
			else if (fieldtype == "tinyint")
			{
				m_Schema[i]["fieldtype"][j] = "int";
			}
			else if (fieldtype == "int")
			{
				m_Schema[i]["fieldtype"][j] = "int";
			}
			else if (fieldtype == "bigint")
			{
				m_Schema[i]["fieldtype"][j] = "int64";
			}
			else
			{
				m_Schema[i]["fieldtype"][j] = "string";
			}
		}
	}
	return true;
}

void CMysqlCache::ConvertRecord(const std::string &tablename, nlohmann::json &t)
{
	for (nlohmann::json::iterator iter = t.begin(); iter != t.end(); ++iter)
	{
		std::string field = iter.key();
		std::string value = iter.value();
		std::string type = m_Schema[tablename]["fieldtype"][field].get<std::string>();
		if (type == "float")
		{
			t[field] = std::stof(value);
		}
		else if (type == "double")
		{
			t[field] = std::stod(value);
		}
		else if (type == "int")
		{
			t[field] = std::stoi(value);
		}
		else if (type == "int64")
		{
			t[field] = std::stoll(value);
		}
	}
}

const std::string CMysqlCache::MakeKey(DataBase::CRecordset *row,const std::string &key)
{
	std::string cachekey;
	cachekey = row->GetChar(key.c_str());

	return cachekey;
}

std::vector<std::unordered_map<std::string, std::string>> CMysqlCache::LoadData(const nlohmann::json &config, const char *guid)
{
	std::string tablename = config["table_name"];
	std::string pk = m_Schema[tablename]["pk"];
	int32 offset = 0;
	std::string sql;
	std::vector<std::unordered_map<std::string, std::string>> data;

	while (1)
	{
		if (guid)
		{
			if (config.find("columns") == config.end())
			{
				sql = fmt::format("select * from {0} where account = {1} order by {2} asc limit {3}, 1000", tablename, guid, pk, offset);
			}
			else
			{
				sql = fmt::format("select {0} from {1} where account = {2} order by {3} asc limit {4}, 1000", config["columns"], tablename, guid, pk, offset);
			}
		}
		else
		{
			if (config.find("columns") == config.end())
			{
				sql = fmt::format("select * from {0} order by {1} asc limit {2}, 1000", tablename, pk, offset);
			}
			else
			{
				sql = fmt::format("select {0} from {1} order by {2} asc limit {3}, 1000", config["columns"], tablename, pk, offset);
			}
		}

		DataBase::CRecordset *res = m_Con->Execute(sql.c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			// 查询到的信息
			while (!res->IsEnd())
			{
				// 将获取的数据添加到缓存中
				std::string key = fmt::format("{0}:{1}", tablename,MakeKey(res, config["cache_key"]));
				m_TempMap.clear();
				for (auto &i : m_Schema[tablename]["fields"])
				{
					std::string field = i.get<std::string>();
					m_TempMap[field] = res->GetChar(field.c_str());
				}
				m_CacheData[key] = m_TempMap;
				// 对需要排序的数据插入有序集合
				if (config.find("index_key") != config.end())
				{
					std::string indexkey = fmt::format("{0}:index:{1}", tablename, MakeKey(res, config["index_key"]));
					auto iter = m_CacheData.find(indexkey);
					if (iter != m_CacheData.end())
					{
						std::unordered_map<std::string, std::string> &temp = iter->second;
						temp.insert(std::make_pair(key, ""));
					}
					else
					{
						std::unordered_map<std::string, std::string> temp;
						temp.insert(std::make_pair(key, ""));
						m_CacheData[indexkey] = temp;
					}
				}
				data.push_back(m_TempMap);
				res->NextRow();
			}
		}
		else
			break;
		
		if (res->GetRowCount() < 1000)
			break;

		offset += 1000;
	}

	return data;
}

std::unordered_map<std::string, std::string> CMysqlCache::LoadDataSingle(const std::string &tablename, const char *uid)
{
	std::vector<std::unordered_map<std::string, std::string>> data = LoadData(m_DBTableConfig[tablename], uid);
	assert(data.size() == 1);
	return data[0];
}

nlohmann::json CMysqlCache::LoadDataMulti(const std::string &tablename, const char *uid)
{
	nlohmann::json data;
	std::vector<std::unordered_map<std::string, std::string>> t = LoadData(m_DBTableConfig[tablename], uid);
	std::string pk = m_Schema[tablename]["pk"];
	for (auto &i : t)
	{
		data[i[pk]] = i;
	}
	return data;
}

nlohmann::json CMysqlCache::ExecuteSingle(const std::string &tablename, const char *guid, std::list<std::string> *fields)
{
	nlohmann::json result;
	std::string key = fmt::format("{0}:{1}", tablename, guid);

	auto iter = m_CacheData.find(key);
	if (iter != m_CacheData.end())
	{
		std::unordered_map<std::string, std::string> &temp = iter->second;
		if (fields)
		{
			for (auto &i : *fields)
			{
				result[i] = temp[i];
			}
		}
		else
		{
			result = temp;
		}
	}

	// 没有找到结果，到数据库中查找
	if (result.empty())
	{
		std::unordered_map<std::string, std::string> t = LoadDataSingle(tablename, guid);
		if (fields && !t.empty())
		{
			for (auto &i : *fields)
			{
				result[i] = t[i];
			}
		}
		else
			result = t;
	}

	ConvertRecord(tablename, result);

	return result;
}

nlohmann::json CMysqlCache::ExecuteMulti(const std::string &tablename, const std::string &guid, int64 id, std::list<std::string> *fields)
{
	nlohmann::json result;
	std::string key = fmt::format("{0}:index:{1}", tablename, guid);

	if (id)
	{
		// 根据ID获取一条数据
		auto iter = m_CacheData.find(fmt::format("{0}:{1}", tablename, id));
		if (iter != m_CacheData.end())
		{
			std::unordered_map<std::string, std::string> &temp = iter->second;
			if (fields)
			{
				for (auto &i : *fields)
				{
					result[i] = temp[i];
				}
			}
			else
			{
				result = temp;
			}

			ConvertRecord(tablename, result);
		}
	}
	else
	{
		auto iter = m_CacheData.find(key);
		if (iter != m_CacheData.end())
		{
			// 获取全部数据
			std::unordered_map<std::string, std::string> &temp = iter->second;
			for (auto &i : temp)
			{
				auto iter = m_CacheData.find(fmt::format("{0}:{1}", tablename, i.first));
				if (iter != m_CacheData.end())
				{
					std::unordered_map<std::string, std::string> &temp = iter->second;
					if (fields)
					{
						for (auto &j : *fields)
						{
							result[i.first][j] = temp[j];
						}
					}
					else
					{
						result[i.first] = temp;
						ConvertRecord(tablename, result[i.first]);
					}
				}
			}
		}
	}

	// 没有找到结果，到数据库中查找
	if (result.empty())
	{
		nlohmann::json t = LoadDataMulti(tablename, guid.c_str());
		if (id)
		{
			if (fields)
			{
				for (auto &i : *fields)
				{
					result[i] = t[id][i];
				}
			}
			else
			{
				result = t[id];
			}
		}
		else
		{
			if (fields)
			{
				for (nlohmann::json::iterator iter = t.begin(); iter != t.end(); ++iter)
				{
					std::string pk = iter.key();
					result[pk] = {};
					for (auto &j : *fields)
					{
						result[pk][j] = iter.value()[j];
					}
				}
			}
			else
			{
				result = t;
			}
		}
	}
	
	return result;
}

bool CMysqlCache::Insert(const std::string &tablename, nlohmann::json &fields)
{
	nlohmann::json config = m_DBTableConfig[tablename];
	std::string key = fmt::format("{0}:{1}", tablename, fields[config["cache_key"].get<std::string>()].get<std::string>());

	auto iter = m_CacheData.find(key);
	if (iter == m_CacheData.end())
	{
		std::string columns;
		std::string valus;
		std::unordered_map<std::string, std::string> temp;
		for (nlohmann::json::iterator iter = fields.begin(); iter != fields.end(); ++iter)
		{
			temp[iter.key()] = iter.value().get<std::string>();
			if (columns.empty())
			{
				columns = iter.key();
				valus = iter.value().get<std::string>();
			}
			else
			{
				columns += "," + iter.key();
				valus += "," + iter.value().get<std::string>();
			}
		}
		m_CacheData[key] = temp;
		// 同步到Mysql
		DataBase::CRecordset *res = m_Con->Execute(fmt::format("insert into {0} ({1}) values({2})", tablename, columns, valus).c_str());
		if (res)
		{
			res = m_Con->Execute("select @@IDENTITY");
			if (!res || res->IsEnd() || !res->IsOpen())
			{
				m_Con->Execute("delete from playerdate where account = ''");
			}
		}
		return true;
	}

	return false;
}

bool CMysqlCache::Update(const std::string &tablename, nlohmann::json &fields)
{
	nlohmann::json config = m_DBTableConfig[tablename];
	std::string key = fmt::format("{0}:{1}", tablename, fields[config["cache_key"].get<std::string>()].get<std::string>());

	auto iter = m_CacheData.find(key);
	if (iter != m_CacheData.end())
	{
		std::string setvalues;
		std::unordered_map<std::string, std::string> &temp = iter->second;
		for (nlohmann::json::iterator iter = fields.begin(); iter != fields.end(); ++iter)
		{
			temp[iter.key()] = iter.value().get<std::string>();
			setvalues += iter.key() + "='" + iter.value().get<std::string>() + "',";
		}
		setvalues.pop_back();
		std::string pk = m_Schema[tablename]["pk"];
		// 同步到Mysql
		DataBase::CRecordset *res = m_Con->Execute(fmt::format("update {0} set {1} where {2} = '{3}')", tablename, setvalues, pk, fields[pk]).c_str());
		if (res)
		{
			res = m_Con->Execute("select @@IDENTITY");
			if (!res || res->IsEnd() || !res->IsOpen())
			{
				m_Con->Execute("delete from playerdate where account = ''");
			}
		}
		return true;
	}
	return false;
}
