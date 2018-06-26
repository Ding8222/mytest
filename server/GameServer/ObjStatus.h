/*
* 对象状态
* Copyright (C) ddl
* 2018
*/

#pragma once
#include <unordered_map>
#include <list>
#include "lxnet\base\platform_config.h"

class CStatus;
class CBaseObj;
class CObjStatus
{
public:
	CObjStatus();
	~CObjStatus();

	virtual CBaseObj *GetObj() = 0;

	void StatusRun();

	// 添加状态
	bool AddStatus(int32 id, int32 lev);
	// 删除状态
	void DelStatus(int32 id);
	CStatus *FindStatus(int32 id);
private:
	// 更新状态列表
	void UpdateStatus(const int64 &time);
	void CheckAndRemove();
	std::unordered_map<int32, CStatus *> m_StatusMap;
	std::list<CStatus *> m_WaitRemove;
};
