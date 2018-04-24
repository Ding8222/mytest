/*
* 状态
* Copyright (C) ddl
* 2018
*/

#pragma once
#include "platform_config.h"

static int status_delay_time = 30000;

class CBaseObj;
class CStatus
{
public:
	CStatus();
	~CStatus();
	
	bool Init(int32 id, int32 lev);
	void Run();

	void SetEndTime(const int64 &time)
	{
		nEndTime = time;
	}

	bool isEnd(const int64 &time)
	{
		return nEndTime > 0 && time >= nEndTime;
	}

	bool CanRemove(const int64 &time)
	{
		return time >= nEndTime + status_delay_time;
	}

	// 添加状态效果
	void AddStatusEffect(CBaseObj *obj);
	// 移除状态效果
	void DelStatusEffect(CBaseObj *obj);
private:
	// 状态ID
	int32 nID;
	// 状态等级
	int32 nLev;
	// 结束时间
	int64 nEndTime;
};
