/*
* 对象战斗
* Copyright (C) ddl
* 2018
*/

#pragma once
#include <map>
#include <list>
#include "lxnet\base\platform_config.h"

#define HURT_REMOVE_TIME 60

struct stHurt
{
	stHurt()
	{
		nHurt = 0;
		nLastHurtTime = 0;
	}

	bool isNeedRemove(const int64 &time)
	{
		return nLastHurtTime > 0 && time >= nLastHurtTime + HURT_REMOVE_TIME;
	}

	// 伤害值
	int64 nHurt;
	// 最后攻击时间
	int64 nLastHurtTime;
};

struct stAttackSkill
{
	stAttackSkill()
	{
		clear();
	}

	void clear()
	{
		nSkillID = 0;
		nLev = 0;
		nTime = 0;
		nTargetTempID = 0;
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		pSkill = nullptr;
	}

	bool isValid()
	{
		return nSkillID;
	}

	// 技能ID
	int32 nSkillID;
	//技能等级
	int32 nLev;
	// 使用时间
	int64 nTime;
	// 目标ID
	uint32 nTargetTempID;

	// 坐标
	float x;
	float y;
	float z;

	void *pSkill;
};

class CBaseObj;
class CObjFight
{
public:
	CObjFight();
	~CObjFight();

	virtual CBaseObj *GetObj() = 0;

	void FightRun();

	// 主动攻击
	// 目标临时id,技能id,坐标x,y,z,方向
	void ActiveAttack(uint32 targettempid, int32 skillid, float x, float y, float z);
	
private:
	// 应用技能
	void ApplySkill(const int64 &time);
	// 筛选目标
	void FilterTarget(std::list<CBaseObj *> &list);
	// 伤害计算
	int32 CalculateDamage(CBaseObj *target);
	// 应用伤害
	void ApplyDamage(CBaseObj *target, int32 value);

	// 更新伤害列表
	void UpdateHurtMap(const int64 &time);
	// 添加到伤害列表
	void AddToHurtMap(uint32 tempid, int32 value);
	// 清空伤害列表
	void CleanHurtMap();
private:

	// 当前使用的技能
	stAttackSkill m_AttackSkill;

	// 伤害列表
	std::map<int32, stHurt *> m_HurtMap;
};
