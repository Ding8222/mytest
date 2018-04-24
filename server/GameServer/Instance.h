/*
* 副本
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "Scene.h"
#include "CSVLoad.h"

class CInstance: public CScene
{
public:
	CInstance();
	~CInstance();
	void Destroy();

	bool Init(int32 instancebaseid);
	void Run();

	// 副本中怪物死亡
	void MonsterDie(int32 monsterid);
public:
	// 设置副本通关结果
	void SetInstanceResult(bool result) { m_InstanceResult = result; }
	// 获取副本通关结果
	bool GetInstanceResult() { return m_InstanceResult; }
	// 获取副本基本ID
	int32 GetInstanceBaseID() { return m_InstanceBaseID; }
	// 设置副本唯一ID
	void SetInsranceID(int32 id) { m_InstanceID = id; }
	// 获取副本唯一ID
	int32 GetInsranceID() { return m_InstanceID; }

	void SetWaitRemove() { m_RemoveTime = g_currenttime; }
	bool IsNeedRemove() { return m_RemoveTime > 0; }
	bool CanRemove(int64 currenttime) { if (!IsNeedRemove()) return false; return currenttime >= m_RemoveTime; }
	bool IsOverTime(int64 currenttime) { return currenttime >= m_CreateTime + m_LimitTime; }
private:
	// 副本刷怪
	void CreateMonsterRun(const int64 &time);

private:
	// 副本基本ID
	int32 m_InstanceBaseID;
	// 副本实例ID
	int32 m_InstanceID;
	// 副本结果
	bool m_InstanceResult;

	// 当前波数
	int32 m_NowWave;
	// 最大波数
	int32 m_MaxWave;
	// 刷怪延迟时间
	int32 m_CreateMonsterDelayTime;
	// 剩余怪物数量
	int32 m_MonsterCount;

	// 副本限制时间
	int32 m_LimitTime;
	// 副本创建时间
	int64 m_CreateTime;
	int64 m_RemoveTime;

	// 刷怪信息
	std::list<CSVData::stInstanceMonster *> m_InstanceMonster;
};