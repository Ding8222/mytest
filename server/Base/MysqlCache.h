#pragma once
#include <unordered_map>
#include "platform_config.h"
#include "sqlinterface.h"
#include "json.hpp"
#include "ossome.h"

class CDBWorkInstance
{
public:
	CDBWorkInstance();
	~CDBWorkInstance();

	bool Init(int32 delay, DataBase::CConnection *con);
	void Run();
	void Destroy();

	void Push(const std::string &sql);
private:
	bool m_Run;
	bool m_WorkFinish;
	int32 m_Delay;
	LOCK_struct m_lock;
	std::list<std::string> m_SqlQueue;
	std::list<std::string> m_TempQueue;
	DataBase::CConnection *m_Con;
};

#define MysqlCache CMysqlCache::Instance()
class CMysqlCache
{
public:
	CMysqlCache();
	~CMysqlCache();

	static CMysqlCache &Instance()
	{
		static CMysqlCache m;
		return m;
	}

	bool Init(const std::string &dbname, DataBase::CConnection *con);
	void Destroy();

	nlohmann::json ExecuteSingle(const std::string &tablename, const char *guid = nullptr, std::list<std::string> *fields = nullptr);
	nlohmann::json ExecuteMulti(const std::string &tablename, const std::string &guid, int64 id = 0, std::list<std::string> *fields = nullptr);
	bool Insert(const std::string &tablename, nlohmann::json &fields);
	bool Update(const std::string &tablename, nlohmann::json &fields);
private:
	// 获取表的主键
	const char *GetPrimaryKey(const std::string &tablename);
	// 获取表的字段
	std::list<std::string> GetFields(const std::string &tablename);
	// 获取变量类型
	const char *GetFieldType(const std::string &tablename, const std::string &Field);
	// 从mysql加载所有表格结构
	bool LoadSchema();
	// 根据数值类型转化
	void ConvertRecord(const std::string &tablename, nlohmann::json &t);
private:
	// m_CacheData的key
	const std::string MakeKey(DataBase::CRecordset *row, const std::string &key);
	// 加载数据
	std::vector<std::unordered_map<std::string, std::string>> LoadData(const nlohmann::json &config, const char *guid = nullptr);
	std::unordered_map<std::string, std::string> LoadDataSingle(const std::string &tablename, const char *uid);
	nlohmann::json LoadDataMulti(const std::string &tablename, const char *uid);
private:
	// <tablename,tabledata(<key,set(<key,value>)>)>
	std::unordered_map<std::string, std::unordered_map<int64, std::unordered_map<std::string*,std::string*>*>*> m_RecordMap;
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_CacheData;
	std::unordered_map<std::string, std::string> m_TempMap;
	// 表结构
	nlohmann::json m_Schema;
	nlohmann::json m_DBTableConfig;
	DataBase::CConnection *m_Con;
	std::string m_DBName;

private:
	CDBWorkInstance * m_WordInstance;
};

void StartTaskQueueThread(CDBWorkInstance *instance);