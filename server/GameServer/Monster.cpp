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
	m_ReliveX = 0;
	m_ReliveY = 0;
	m_ReliveZ = 0;
}

CMonster::~CMonster()
{
	Destroy();
}

bool CMonster::Init(int32 monsterid, float relivex, float relivey, float relivez, bool relive, int32 relivecd)
{
	CSVData::stMonster *monsterinfo = CSVData::CMonsterDB::FindById(monsterid);
	if (!monsterinfo)
		return false;

	m_bCanRelive = relive;
	m_MonsterID = monsterinfo->nMonsterID;
	m_MonsterType = monsterinfo->nMonsterType;
	m_ReliveCD = relivecd;
	m_ReliveX = relivex;
	m_ReliveY = relivey;
	m_ReliveZ = relivez;

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
	if (IsDie())
		return;
	
	LeaveScene();

	SetDieTime(CTimer::GetTime64());

	// 不需要复活的怪物，移除
	if(!m_bCanRelive)
		SetWaitRemove();
}

void CMonster::Relive()
{
	if (!IsNeedRelive(CTimer::GetTime64()))
		return;

	CSVData::stMonster *monsterinfo = CSVData::CMonsterDB::FindById(GetMonsterID());
	if (!monsterinfo)
	{
		RunStateError("怪物：%d 复活失败！没有找到怪物配置信息。", GetMonsterID());
		SetWaitRemove();
		return;
	}

	CScene *scene = GetScene();
	if (!scene)
	{
		RunStateError("怪物：%d 复活失败！没有找到怪物场景指针。", GetMonsterID());
		SetWaitRemove();
		return;
	}

	if (!scene->AddObj(this))
	{
		RunStateError("怪物：%d 复活失败！添加到场景：%d 失败。", GetMonsterID(), scene->GetMapID());
		SetWaitRemove();
		return;
	}

	if (!MoveTo(m_ReliveX, m_ReliveY, m_ReliveZ))
	{
		RunStateError("怪物：%d 复活失败！移动失败！场景：%d", GetMonsterID(), scene->GetMapID());
		SetWaitRemove();
		Die();
		return;
	}
	SetDieTime(0);
}
