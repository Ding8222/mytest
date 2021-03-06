﻿#pragma once
#include <unordered_map>
#include "lxnet\base\platform_config.h"
#include "sqlinterface.h"
#include "json.hpp"
#include "lxnet\base\cthread.h"

class CStringPool;
class CDBWorkInstance
{
public:
	CDBWorkInstance();
	~CDBWorkInstance();

	bool Init(int32 delay, const char* dbname, const char *username, const char* password, const char* ipstring, bool opensqllog);
	void Run();
	void Destroy();

	int32 GetSize() { return m_SqlQueue.size(); }
	int32 GetTempSize() { return m_TempQueue.size(); }
	int32 GetMaxSize() { return m_MaxSize; }

	void Push(void *sql);
private:
	volatile bool m_Run;
	volatile bool m_WorkFinish;
	int32 m_Delay;
	int32 m_MaxSize;
	cspin m_lock;
	std::list<void *> m_SqlQueue;
	std::list<void *> m_TempQueue;
	DataBase::CConnection m_Con;
};

#define DBCache CDBCache::Instance()
class CDBCache
{
public:
	CDBCache();
	~CDBCache();

	static CDBCache &Instance()
	{
		static CDBCache m;
		return m;
	}

	bool Init(const char* dbname, const char *username, const char* password, const char* ipstring, bool opensqllog);
	void Destroy();
	void GetCurrentInfo(char *buf, size_t buflen);

	// 查询单条数据
	nlohmann::json ExecuteSingle(const std::string &tablename, const char *guid = nullptr, std::list<std::string> *fields = nullptr);
	// 查询多条数据
	nlohmann::json ExecuteMulti(const std::string &tablename, const std::string &guid, int64 id = 0, std::list<std::string> *fields = nullptr);
	// 插入数据
	bool Insert(const std::string &tablename, nlohmann::json &fields);
	// 更新数据
	bool Update(const std::string &tablename, nlohmann::json &fields);
private:
	// 获取表的主键
	const char *GetPrimaryKey(const std::string &tablename);
	// 获取表的字段
	std::list<std::string> GetFields(const std::string &tablename);
	// 获取变量类型
	const char *GetFieldType(const std::string &tablename, const std::string &Field);
	// 从数据库加载所有表格结构
	bool LoadSchema();
	// 根据数值类型转化
	void ConvertRecord(const std::string &tablename, nlohmann::json &t);
private:
	// m_CacheData的key
	const std::string MakeKey(std::unordered_map<std::string, std::string> &fields, const std::string &key);
	// 加载数据
	std::vector<std::unordered_map<std::string, std::string>> LoadData(const nlohmann::json &config, const char *guid = nullptr);
	// 加载单条数据
	std::unordered_map<std::string, std::string> *LoadDataSingle(const std::string &tablename, const char *uid);
	// 加载多条数据
	nlohmann::json LoadDataMulti(const std::string &tablename, const char *uid);
private:
	// <表名:主键值,<字段名,值>>
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_CacheData;
	// 临时存放字段数据的map
	std::unordered_map<std::string, std::string> m_TempMap;
	// 表结构
	nlohmann::json m_Schema;
	// 表配置
	nlohmann::json m_DBTableConfig;
	// 数据库连接
	DataBase::CConnection m_Con;
	// 数据库名称
	std::string m_DBName;

private:
	CDBWorkInstance * m_WorkInstance;
};

void StartTaskQueueThread(CDBWorkInstance *instance);
