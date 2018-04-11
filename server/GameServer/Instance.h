/*
* 副本
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "Scene.h"

class CInstance: public CScene
{
public:
	CInstance();
	~CInstance();
	void Destroy();

	bool Init(int32 instancebaseid);
	void Run();

	void SetInsranceID(int32 id) { m_InstanceID = id; }
	int32 GetInsranceID() { return m_InstanceID; }
	void SetWaitRemove() { m_RemoveTime = g_currenttime; }
	bool IsNeedRemove() { return m_RemoveTime > 0; }
	bool CanRemove(int64 currenttime) { if (!IsNeedRemove()) return false; return currenttime >= m_RemoveTime; }
	bool IsOverTime(int64 currenttime) { return currenttime >= m_CreateTime + m_LimitTime; }
private:

	// 副本实例ID
	int32 m_InstanceID;
	// 副本限制时间
	int32 m_LimitTime;
	// 副本创建时间
	int64 m_CreateTime;
	int64 m_RemoveTime;
};