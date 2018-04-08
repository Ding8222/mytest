/*
* 对象状态
* Copyright (C) ddl
* 2018
*/

#pragma once
#include <unordered_map>
#include <list>
#include "platform_config.h"

struct stStatus 
{
	stStatus()
	{
		nID = 0;
		nLev = 0;
		nEndTime = 0;
	}

	bool isNeedRemove(const int64 &time)
	{
		return nEndTime > 0 && time >= nEndTime;
	}

	void SetRemove(const int64 &time)
	{
		nEndTime = time;
	}

	// 状态ID
	int32 nID;
	// 状态等级
	int32 nLev;
	// 结束时间
	int64 nEndTime;
};

class CBaseObj;
class CObjStatus
{
public:
	CObjStatus();
	~CObjStatus();

	virtual CBaseObj *GetObj() = 0;

	void StatusRun();

	bool AddStatus(int32 id, int32 lev);
	void DelStatus(int32 id);
	stStatus *FindStatus(int32 id);
private:
	// 更新状态列表
	void UpdateStatus(const int64 time);

	std::unordered_map<int32, stStatus *> m_StatusMap;
};
