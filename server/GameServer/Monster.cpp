#include "Monster.h"
#include "CSVLoad.h"
#include "Timer.h"
#include "Scene.h"
#include "ServerLog.h"

CMonster::CMonster():CBaseObj(EOT_MONSTER)
{
	m_bCanRelive = false;
	m_MonsterID = 0;
	m_MonsterType = 0;
	m_ReliveCD = 0;
}

CMonster::~CMonster()
{

}

bool CMonster::Init(int32 monsterid)
{
	CSVData::stMonster *monsterinfo = CSVData::CMonsterDB::FindById(monsterid);
	if (!monsterinfo)
		return false;

	m_bCanRelive = monsterinfo->bCanRelive;
	m_MonsterID = monsterinfo->nMonsterID;
	m_MonsterType = monsterinfo->nMonsterType;
	m_ReliveCD = monsterinfo->nReliveCD;

	return true;
}

void CMonster::Destroy()
{

}

void CMonster::Run()
{
	if (IsDie())
	{
		Relive();
	}
	else
	{
		CBaseObj::Run();
	}
}

void CMonster::Die()
{
	if (IsWaitRemove())
		return;

	LeaveScene();

	SetDieTime(CTimer::GetTime64());
	//放在最后
	SetWaitRemove();
}

void CMonster::Relive()
{
	if (!m_bCanRelive)
		return;
	
	if (!IsNeedRelive(CTimer::GetTime64()))
		return;

	CSVData::stMonster *monsterinfo = CSVData::CMonsterDB::FindById(GetMonsterID());
	if (!monsterinfo)
	{
		RunStateError("怪物：%d 复活失败！没有找到怪物配置信息。", GetMonsterID());
		m_bCanRelive = false;
		return;
	}

	CScene *scene = GetScene();
	if (!scene)
	{
		RunStateError("怪物：%d 复活失败！没有找到怪物场景指针。", GetMonsterID());
		m_bCanRelive = false;
		return;
	}

	if (!scene->AddObj(this))
	{
		RunStateError("怪物：%d 复活失败！添加到场景：%d 失败。", GetMonsterID(), scene->GetMapID());
		m_bCanRelive = false;
		Die();
		return;
	}

	if (!MoveTo(monsterinfo->nX, monsterinfo->nY, monsterinfo->nZ))
	{
		RunStateError("怪物：%d 复活失败！移动失败！场景：%d", GetMonsterID(), scene->GetMapID());
		m_bCanRelive = false;
		Die();
		return;
	}
	SetDieTime(0);
	RestWaitRemove();
}
