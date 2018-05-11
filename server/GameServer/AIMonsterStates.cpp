#include "AIMonsterStates.h"
#include "State.h"
#include "AIMonster.h"

// 靠近（固定点）
CSeek* CSeek::Instance()
{
	static CSeek instance;
	return &instance;
}

void CSeek::Enter(CAIMonster* pMonster)
{

}

void CSeek::Execute(CAIMonster* pMonster)
{
	if (pMonster->AtTargetPos())
	{
		pMonster->GetFSM()->ChangeState(CWander::Instance());
	}
	else
	{

	}
}

void CSeek::Exit(CAIMonster* pMonster)
{

}

// 徘徊
CWander* CWander::Instance()
{
	static CWander instance;
	return &instance;
}

void CWander::Enter(CAIMonster* pMonster)
{

}

void CWander::Execute(CAIMonster* pMonster)
{
	float x = pMonster->GetNowPosX();
	float y = pMonster->GetNowPosY();
	float z = pMonster->GetNowPosZ();
	int nRand = rand() % 10000;
	if (nRand > 5000)
		x += rand() % 500;
	else
		x -= rand() % 500;

	if (x > 1000)
		x = 1000;
	else if (x < 1)
		x = 1;

	nRand = rand() % 10000;
	if (nRand > 5000)
		y += rand() % 500;
	else
		y -= rand() % 500;

	if (y > 1000)
		y = 1000;
	else if (y < 1)
		y = 1;

	pMonster->SetTargetPos(Vector3D(x, y, z));

	pMonster->GetFSM()->ChangeState(CSeek::Instance());
}

void CWander::Exit(CAIMonster* pMonster)
{

}

// 追逐（目标）
CPursuit* CPursuit::Instance()
{
	static CPursuit instance;

	return &instance;
}

void CPursuit::Enter(CAIMonster* pMonster)
{
	pMonster->SetTargetPos(pMonster->GetTargetObj()->GetNowPosV());
}

void CPursuit::Execute(CAIMonster* pMonster)
{
	// 抵达目标地点
	if (pMonster->AtTargetPos())
	{
		pMonster->GetFSM()->ChangeState(CAttack::Instance());
		return;
	}

	// 根据目标坐标更新目标点
	if (pMonster->TargetObjInView())
	{
		pMonster->SetTargetPos(pMonster->GetTargetObj()->GetNowPosV());
	}
}

void CPursuit::Exit(CAIMonster* pMonster)
{

}

// 逃避
CEvade* CEvade::Instance()
{
	static CEvade instance;

	return &instance;
}

void CEvade::Enter(CAIMonster* pMonster)
{
	pMonster->SetTargetPos(pMonster->GetHomePos());
}

void CEvade::Execute(CAIMonster* pMonster)
{
	if (pMonster->AtTargetPos())
		pMonster->GetFSM()->ChangeState(CWander::Instance());
}

void CEvade::Exit(CAIMonster* pMonster)
{

}

// 返回出生点
CReturnToHomeRegion* CReturnToHomeRegion::Instance()
{
	static CReturnToHomeRegion instance;

	return &instance;
}

void CReturnToHomeRegion::Enter(CAIMonster* pMonster)
{
	pMonster->SetTargetPos(pMonster->GetHomePos());
}

void CReturnToHomeRegion::Execute(CAIMonster* pMonster)
{
	if (pMonster->AtTargetPos())
	{
		pMonster->GetFSM()->ChangeState(CWander::Instance());
	}
}

void CReturnToHomeRegion::Exit(CAIMonster* pMonster)
{

}


// 攻击
CAttack* CAttack::Instance()
{
	static CAttack instance;

	return &instance;
}

void CAttack::Enter(CAIMonster* pMonster)
{

}

void CAttack::Execute(CAIMonster* pMonster)
{
	// 不在视野范围内,返回出生点
	if (!pMonster->TargetObjInView())
	{
		pMonster->GetFSM()->ChangeState(CReturnToHomeRegion::Instance());
		return;
	}

	// 不在攻击范围内,追逐
	if (!pMonster->AtAttackRange())
	{
		pMonster->GetFSM()->ChangeState(CPursuit::Instance());
		return;
	}
}

void CAttack::Exit(CAIMonster* pMonster)
{

}