/*
* 怪物对象
* Copyright (C) ddl
* 2018
*/

#pragma once
#include "BaseObj.h"

enum MonsterType
{
	EMT_NORMAL = 0,		// 普通怪物
	EMT_BOSS,			// BOSS
};

class CMonster :public CBaseObj
{
public:
	CMonster();
	~CMonster();

	bool Init(int32 monsterid, float relivex, float relivey, float relivez, bool relive, int32 relivecd);
	void Destroy();
	virtual void Run();
	virtual void SendMsgToMe(Msg &pMsg, bool bRef = false);
	virtual void Die();
	void Relive();
	
	int32 GetMonsterID() { return m_MonsterID; }
	int32 GetMonsterType() { return m_MonsterType; }
	bool IsNeedRelive(int64 time) { return m_bCanRelive && time >= GetDieTime() + m_ReliveCD; }
private:
	bool m_bCanRelive;
	int32 m_MonsterID;
	int32 m_MonsterType;
	int32 m_ReliveCD;
	float m_ReliveX;
	float m_ReliveY;
	float m_ReliveZ;
};