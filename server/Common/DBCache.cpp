#include <list>
#include <fstream>
#include "DBCache.h"
#include "fmt/ostream.h"
#include "ServerLog.h"
#include "Timer.h"
#include "crosslib.h"
#include "StringPool.h"

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#define delaytime(v)	Sleep(v)
#else
#include <pthread.h>
#define delaytime(v)	usleep((v) * 1000)
#endif

CDBWorkInstance::CDBWorkInstance()
{
	m_Run = false;
	m_WorkFinish = false;
	m_Delay = 0;
	LOCK_INIT(&m_lock);
	m_SqlQueue.clear();
	m_TempQueue.clear();
}

CDBWorkInstance::~CDBWorkInstance()
{
	Destroy();
}

bool CDBWorkInstance::Init(int32 delay, const char* dbname, const char *username, const char* password, const char* ipstring, bool opensqllog)
{
	m_Con.SetLogDirectory("log_log/DBServer_Log/write_dbhand_log");
	m_Con.SetEnableLog(opensqllog);
	if (!m_Con.Open(dbname,
		username,
		password,
		ipstring))
	{
		RunStateError("连接数据库失败!");
		return false;
	}

	if (!m_Con.SetCharacterSet("utf8"))
	{
		RunStateError("设置UTF-8失败!");
		return false;
	}

	m_Run = true;

	if (delay < 0)
		m_Delay = 10;
	else
		m_Delay = delay;

	return true;
}

void CDBWorkInstance::Destroy()
{
	//设置退出
	m_Run = false;

	while (!m_WorkFinish)
	{
		delaytime(10);
	}
	m_SqlQueue.clear();
	m_TempQueue.clear();
	LOCK_DELETE(&m_lock);
}

void CDBWorkInstance::Push(void *sql)
{
	LOCK_LOCK(&m_lock);
	m_SqlQueue.push_back(sql);
	LOCK_UNLOCK(&m_lock);
}

void CDBWorkInstance::Run()
{
	static const int maxdelay = m_Delay;
	int64 currenttime;
	int64 delay;

	m_WorkFinish = false;
	while (m_Run || !m_SqlQueue.empty())
	{
		currenttime = get_microsecond();

		LOCK_LOCK(&m_lock);
		m_TempQueue = std::move(m_SqlQueue);
		LOCK_UNLOCK(&m_lock);

		while (!m_TempQueue.empty())
		{
			CStringPool *sql = (CStringPool *)m_TempQueue.front();
			const char *str = sql->GetMsg();
			if (str)
			{
				DataBase::CRecordset *res = m_Con.Execute(str);
			}
			string_release(sql);
			m_TempQueue.pop_front(); 
		}
		delay = get_microsecond() - currenttime;
		if (delay < maxdelay)
			delaytime(static_cast<DWORD>(maxdelay - delay));
	}
	m_WorkFinish = true;
}

CDBCache::CDBCache()
{
	m_CacheData.clear();
	m_TempMap.clear();
	m_Schema.clear();
	m_DBTableConfig.clear();
	m_DBName.clear();
	m_WorkInstance = nullptr;
}

CDBCache::~CDBCache()
{
	Destroy();
}

bool CDBCache::Init(const char* dbname, const char *username, const char* password, const char* ipstring, bool opensqllog)
{
	if (!CStringPool::InitPools())
	{
		RunStateError("初始化task pool失败!");
		return false;
	}

	m_DBName = dbname;

	m_Con.SetLogDirectory("log_log/DBServer_Log/read_dbhand_log");
	m_Con.SetEnableLog(opensqllog);
	if (!m_Con.Open(dbname,
		username,
		password,
		ipstring))
	{
		RunStateError("连接数据库失败!");
		return false;
	}

	if (!m_Con.SetCharacterSet("utf8"))
	{
		RunStateError("设置UTF-8失败!");
		return false;
	}

	m_WorkInstance = (CDBWorkInstance *)malloc(sizeof(CDBWorkInstance));
	if (!m_WorkInstance)
	{
		RunStateError("创建CDBWorkInstance内存失败!");
		return false;
	}
	new(m_WorkInstance) CDBWorkInstance();
	if (!m_WorkInstance->Init(10, dbname,
		username,
		password,
		ipstring,
		opensqllog))
	{
		RunStateError("初始化CDBWorkInstance失败!");
		return false;
	}
	
	StartTaskQueueThread(m_WorkInstance);

	if (!LoadSchema())
	{
		RunStateError("加载表结构失败!");
		return false;
	}
	
	m_CacheData.reserve(m_Schema.size());
	
	std::ifstream i("./config/DBTable.json");
	if (!i.is_open())
	{
		RunStateError("加载DBTable.json失败!");
		return false;
	}
	
	m_DBTableConfig = nlohmann::json::parse(i);
	
	i.close();

	int64 time = get_millisecond();

	for (nlohmann::json::iterator iter = m_DBTableConfig.begin(); iter != m_DBTableConfig.end(); ++iter)
	{
		LoadData(iter.value());
	}

	RunStateLog("加载数据库耗时：%d ms", get_millisecond() - time);
	return true;
}

void CDBCache::Destroy()
{
	m_CacheData.clear();
	m_TempMap.clear();
	m_Schema.clear();
	m_DBTableConfig.clear();
	m_DBName.clear();
	if (m_WorkInstance)
	{
		m_WorkInstance->Destroy();
		delete m_WorkInstance;
	}
	m_WorkInstance = nullptr;
	CStringPool::DestroyPools();
}

const char *CDBCache::GetPrimaryKey(const std::string &tablename)
{
	DataBase::CRecordset *res = m_Con.Execute(
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

std::list<std::string> CDBCache::GetFields(const std::string &tablename)
{
	std::list<std::string> test;
	DataBase::CRecordset *res = m_Con.Execute(
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

const char *CDBCache::GetFieldType(const std::string &tablename, const std::string &field)
{
	DataBase::CRecordset *res = m_Con.Execute(
		fmt::format("select data_type from information_schema.columns where table_schema='{0}' and table_name='{1}' and column_name='{2}'",
			m_DBName, tablename, field).c_str());
	if (res && res->IsOpen() && !res->IsEnd())
	{
		return res->GetChar("data_type");
	}
	return "";
}

bool CDBCache::LoadSchema()
{
	std::list<std::string> tablename;
	DataBase::CRecordset *res = m_Con.Execute(
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
	std::list<std::string> fields;
	for(auto &i: tablename)
	{
		m_Schema[i] = {};
		m_Schema[i]["fields"] = {};
		m_Schema[i]["fieldtype"] = {};
		m_Schema[i]["pk"] = GetPrimaryKey(i);
		fields = GetFields(i);

		m_Schema[i]["fields"] = fields;
		for (auto &j : fields)
		{
			const std::string &fieldtype = GetFieldType(i, j);
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

void CDBCache::ConvertRecord(const std::string &tablename, nlohmann::json &t)
{
	for (nlohmann::json::iterator iter = t.begin(); iter != t.end(); ++iter)
	{
		const std::string &field = iter.key();
		const std::string &value = iter.value();
		const std::string &type = m_Schema[tablename]["fieldtype"][field].get<std::string>();
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

const std::string CDBCache::MakeKey(std::unordered_map<std::string, std::string> &fields,const std::string &key)
{
	std::string cachekey;

	std::unordered_map<std::string, std::string>::iterator iter = fields.find(key);
	if (iter != fields.end())
		cachekey = iter->second;

	return cachekey;
}

std::vector<std::unordered_map<std::string, std::string>> CDBCache::LoadData(const nlohmann::json &config, const char *guid)
{
	const std::string &tablename = config["table_name"];
	const std::string &pk = m_Schema[tablename]["pk"];
	int32 offset = 0;
	std::string sql;
	std::vector<std::unordered_map<std::string, std::string>> data;

	while (1)
	{
		if (guid)
		{
			if (config.find("columns") == config.end())
			{
				sql = fmt::format("select * from {0} where account = '{1}' order by {2} asc limit {3}, 1000", tablename, guid, pk, offset);
			}
			else
			{
				sql = fmt::format("select {0} from {1} where account = '{2}' order by {3} asc limit {4}, 1000", config["columns"], tablename, guid, pk, offset);
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

		DataBase::CRecordset *res = m_Con.Execute(sql.c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			// 查询到的信息
			while (!res->IsEnd())
			{
				// 将获取的数据添加到缓存中
				for (auto &i : m_Schema[tablename]["fields"])
				{
					std::string field = i.get<std::string>();
					m_TempMap[field] = res->GetChar(field.c_str());
				}
				const std::string &key = fmt::format("{0}:{1}", tablename, MakeKey(m_TempMap, config["cache_key"]));
				m_CacheData[key] = m_TempMap;
				// 对需要排序的数据插入有序集合
				if (config.find("index_key") != config.end())
				{
					const std::string &indexkey = fmt::format("{0}:index:{1}", tablename, MakeKey(m_TempMap, config["index_key"]));
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
				m_TempMap.clear();
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

std::unordered_map<std::string, std::string> *CDBCache::LoadDataSingle(const std::string &tablename, const char *uid)
{
	std::vector<std::unordered_map<std::string, std::string>> data = LoadData(m_DBTableConfig[tablename], uid);
	if (data.size() >= 1)
		return &(data[0]);
	else
		return nullptr;
}

nlohmann::json CDBCache::LoadDataMulti(const std::string &tablename, const char *uid)
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

nlohmann::json CDBCache::ExecuteSingle(const std::string &tablename, const char *guid, std::list<std::string> *fields)
{
	nlohmann::json result;
	const std::string &key = fmt::format("{0}:{1}", tablename, guid);

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
		std::unordered_map<std::string, std::string> *t = LoadDataSingle(tablename, guid);
		if (t)
		{
			if (fields && !(*t).empty())
			{
				for (auto &i : *fields)
				{
					result[i] = (*t)[i];
				}
			}
			else
				result = (*t);
		}
	}

	ConvertRecord(tablename, result);

	return result;
}

nlohmann::json CDBCache::ExecuteMulti(const std::string &tablename, const std::string &guid, int64 id, std::list<std::string> *fields)
{
	nlohmann::json result;
	const std::string &key = fmt::format("{0}:index:{1}", tablename, guid);

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
				auto iter = m_CacheData.find(i.first);
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
					}
					ConvertRecord(tablename, result[i.first]);
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
			std::string idstr = fmt::format("{0}", id);
			if (fields)
			{
				for (auto &i : *fields)
				{
					result[i] = t[idstr][i];
				}
			}
			else
			{
				result = t[idstr];
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
				result = std::move(t);
			}
		}
	}
	
	return result;
}

bool CDBCache::Insert(const std::string &tablename, nlohmann::json &fields)
{
	nlohmann::json config = m_DBTableConfig[tablename];
	const std::string &key = fmt::format("{0}:{1}", tablename, fields[config["cache_key"].get<std::string>()].get<std::string>());

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
				valus = "'" + iter.value().get<std::string>() + "'";
			}
			else
			{
				columns += "," + iter.key();
				valus += ",";
				valus += "'" + iter.value().get<std::string>() + "'";
			}
		}
		m_CacheData[key] = temp;
		if (config.find("index_key") != config.end())
		{
			std::string indexkey = fmt::format("{0}:index:{1}", tablename, MakeKey(temp, config["index_key"]));
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
		// 同步到数据库
		CStringPool *str = string_create();
		if (!str)
		{
			RunStateError("创建String失败!");
			return false;
		}
		if (!str->PushMsg(fmt::format("insert into {0} ({1}) values({2})\0", tablename, columns, valus)))
		{
			RunStateError("添加消息至String失败!");
			string_release(str);
			return false;
		}
		m_WorkInstance->Push(str);
		return true;
	}

	return false;
}

bool CDBCache::Update(const std::string &tablename, nlohmann::json &fields)
{
	nlohmann::json config = m_DBTableConfig[tablename];
	const std::string &key = fmt::format("{0}:{1}", tablename, fields[config["cache_key"].get<std::string>()].get<std::string>());

	auto iter = m_CacheData.find(key);
	if (iter != m_CacheData.end())
	{
		std::string setvalues;
		std::unordered_map<std::string, std::string> &temp = iter->second;
		for (nlohmann::json::iterator iter = fields.begin(); iter != fields.end(); ++iter)
		{
			if (iter.key() == config["cache_key"].get<std::string>())
				continue;

			temp[iter.key()] = iter.value().get<std::string>();
			setvalues += iter.key() + "='" + iter.value().get<std::string>() + "',";
		}
		setvalues.pop_back();
		std::string pk = m_Schema[tablename]["pk"];
		// 同步到数据库
		CStringPool *str = string_create();
		if (!str)
		{
			RunStateError("创建String失败!");
			return false;
		}
		if (!str->PushMsg(fmt::format("update {0} set {1} where {2} = '{3}'\0", tablename, setvalues, pk, fields[pk].get<std::string>())))
		{
			RunStateError("添加消息至String失败!");
			string_release(str);
			return false;
		}
		m_WorkInstance->Push(str);
		return true;
	}
	return false;
}

#ifdef WIN32
static void thiread_ran(void *data)
#else
static void *thiread_ran(void *data)
#endif
{
	assert(data);
	CDBWorkInstance *instance = (CDBWorkInstance*)data;
	instance->Run();
#ifndef WIN32
	return nullptr;
#endif
}

void StartTaskQueueThread(CDBWorkInstance *instance)
{
	//开启逻辑线程
#ifdef WIN32
	_beginthread(thiread_ran, 0, instance);
#else
	pthread_t thandle;
	pthread_create(&thandle, 0, thiread_ran, (void*)instance);
#endif

}
