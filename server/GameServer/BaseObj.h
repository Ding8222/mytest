/*
* 基础对象
* Copyright (C) ddl
* 2018
*/

#pragma once
#include <unordered_map>
#include "GlobalDefine.h"
#include "platform_config.h"
#include "ObjScene.h"
#include "ObjAttribute.h"
#include "ObjStatus.h"
#include "ObjFight.h"

static int obj_delay_time = 30000;
extern int64 g_currenttime;

enum eObjType
{
	EOT_PLAYER = 0,
	EOT_MONSTER = 1,
	EOT_NPC = 2,
	EOT_MAX ,
};

struct Msg;
class CPlayer;
class CMonster;
class CNPC;
class serverinfo;
class CBaseObj :public CObjScene, public CObjAttribute, public CObjStatus, public CObjFight
{
public:
	CBaseObj(int8 nObjType);
	~CBaseObj();

	virtual CBaseObj *GetObj() { return this; }
	virtual void SendMsgToMe(Msg &pMsg, bool bRef = false) {}
	void SendRefMsg(Msg &pMsg);
	virtual void Run();
	virtual void Die() {}

	CPlayer *ToPlayer() { return IsPlayer() ? (CPlayer *)this : nullptr; }
	CMonster *ToMonster() { return IsMonster() ? (CMonster *)this : nullptr; }
	CNPC *ToNPC() { return IsNPC() ? (CNPC *)this : nullptr; }

public:
	void UpdataObjInfo(CBaseObj *obj = nullptr);
	void DelObjFromView(uint32 tempid);
	int8 GetObjType() { return m_ObjType; }
	void SetSex(int8 sex) { m_ObjSex = sex; }
	int8 GetSex() { return m_ObjSex; }
	void SetJob(int8 job) { m_ObjJob = job; }
	int8 GetJob() { return m_ObjJob; }
	void SetLevel(int8 level) { m_ObjLevel = level; }
	int8 GetLevel() { return m_ObjLevel; }
	bool IsPlayer() { return m_ObjType == EOT_PLAYER; }
	bool IsMonster() { return m_ObjType == EOT_MONSTER; }
	bool IsNPC() { return m_ObjType == EOT_NPC; }
	char *GetName() { return m_ObjName; }
	void SetName(const char *_Name) {
		strncpy_s(m_ObjName, _Name, MAX_NAME_LEN); m_ObjName
			[MAX_NAME_LEN - 1] = '\0';
	}
private:
	// 对象类型
	int8 m_ObjType;
	int8 m_ObjSex;
	int8 m_ObjJob;
	int8 m_ObjLevel;
	// 名称
	char m_ObjName[MAX_NAME_LEN];
public:
	void SetWaitRemove() { m_WaitRemoveTime = g_currenttime; }
	bool IsWaitRemove() { return m_WaitRemoveTime > 0; }
	bool CanRemove(int64 time) { return time >= m_WaitRemoveTime + obj_delay_time; }
	void SetDieTime(int64 time) { m_DieTime = time; }
	int64 GetDieTime() { return m_DieTime; }
	bool IsDie() { return m_DieTime > 0; }
private:
	// 待移除时间
	int64 m_WaitRemoveTime;
	int64 m_DieTime;

	static std::unordered_map<serverinfo *, std::list<int32>> gateinfo;
};
