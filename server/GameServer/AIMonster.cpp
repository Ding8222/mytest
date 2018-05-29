#include "AIMonster.h"
#include "AIMonsterStates.h"
#include "Utilities.h"

CAIMonster::CAIMonster()
{
	memset(m_TargetPos, 0, sizeof(float) * EOP_MAX);
	m_vTargetPos.Zero();
	m_TargetObj = nullptr;
	m_StateMachine.SetOwner(this);
	m_StateMachine.SetCurrentState(CWander::Instance());
}

CAIMonster::~CAIMonster()
{
	m_TargetObj = nullptr;
}

void CAIMonster::FSMUpdate()
{
	m_StateMachine.Update();
	
	SetHeading(Vec3DNormalize(m_vTargetPos - GetNowPosV()));
	Vector3D vNowPos = GetNowPosV();
	vNowPos += GetHeading() * static_cast<float>(GetSpeed() * 0.01f);
	MoveTo(vNowPos.x, vNowPos.y, vNowPos.z);
}

bool CAIMonster::AtTargetPos()
{
	return (Vec3DDistanceSq(GetNowPosV(), m_vTargetPos) < 4);
}

bool CAIMonster::TargetObjInView()
{
	if (!FuncUti::isValidCret(m_TargetObj))
		return false;

	if (!FindFromAoiList(m_TargetObj->GetTempID()))
		return false;

	return true;
}

bool CAIMonster::AtAttackRange()
{
	if (!FuncUti::isValidCret(m_TargetObj))
		return false;

	return true;
}