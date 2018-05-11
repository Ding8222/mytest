#pragma once
#include "StateMachine.h"
#include "Monster.h"

class CAIMonster:public CMonster
{
public:
	CAIMonster();
	~CAIMonster();

	virtual void FSMUpdate();
	CStateMachine<CAIMonster> *GetFSM() { return &m_StateMachine; }

	void SetTargetPos(const Vector3D &pos)
	{
		m_vTargetPos = pos;
		m_TargetPos[EOP_X] = pos.x;
		m_TargetPos[EOP_Y] = pos.y;
		m_TargetPos[EOP_Z] = pos.z;
	}

	float GetTargetPosX() { return m_TargetPos[EOP_X]; }
	float GetTargetPosY() { return m_TargetPos[EOP_Y]; }
	float GetTargetPosZ() { return m_TargetPos[EOP_Z]; }
	// 是否已经到目标位置
	bool AtTargetPos();

	void SetTargetObj(CBaseObj *obj) { m_TargetObj = obj; }
	CBaseObj *GetTargetObj() { return m_TargetObj; }
	// 目标单位是否在视野中
	bool TargetObjInView();
	// 目标单位是否在攻击范围
	bool AtAttackRange();


private:
	// 目标坐标
	float m_TargetPos[EOP_MAX];
	Vector3D m_vTargetPos;
	// 目标单位
	CBaseObj *m_TargetObj;
	CStateMachine<CAIMonster> m_StateMachine;
};
