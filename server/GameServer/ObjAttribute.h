/*
* 对象属性
* Copyright (C) ddl
* 2018
*/

#pragma once
#include "platform_config.h"

class CBaseObj;
class CObjAttribute
{
public:
	CObjAttribute();
	~CObjAttribute();

	virtual CBaseObj *GetObj() = 0;

	void SetMaxHP(int32 value) { m_MaxHP = value; }
	void SetMaxMP(int32 value) { m_MaxMP = value; }
	int32 GetMaxHP() { return m_MaxHP; }
	int32 GetMaxMP() { return m_MaxMP; }
	void ChangeMaxHP(int32 value) { m_MaxHP += value; if (m_MaxHP < 0)m_MaxHP = 0; }
	void ChangeMaxMP(int32 value) { m_MaxMP += value; if (m_MaxMP < 0)m_MaxMP = 0; }

	void SetNowHP(int32 value) { m_NowHP = value; }
	void SetNowMP(int32 value) { m_NowMP = value; }
	int32 GetNowHP() { return m_NowHP; }
	int32 GetNowMP() { return m_NowMP; }
	void ChangeNowHP(int32 value) { m_NowHP += value; if (m_NowHP < 0)m_NowHP = 0; }
	void ChangeNowMP(int32 value) { m_NowMP += value; if (m_NowMP < 0)m_NowMP = 0; }
private:
	//等级
	int32 m_LEV;

	// 物理攻击
	int32 m_ATN;
	// 物理防御
	int32 m_DEF;
	// 魔法防御
	int32 m_RES;
	
	//移动速度
	int32 m_SEP;

	int32 m_MaxHP;
	int32 m_MaxMP;
	int32 m_NowHP;
	int32 m_NowMP;
};