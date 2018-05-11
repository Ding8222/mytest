#include "Monster.h"
#include "CSVLoad.h"
#include "Timer.h"
#include "Scene.h"
#include "ServerLog.h"
#include "msgbase.h"
#include "Utilities.h"
#include "GameGatewayMgr.h"
#include "objectpool.h"
#include "AIMonster.h"

#define MONSTER_MAX 100000

static objectpool<CAIMonster> &MonsterPool()
{
	static objectpool<CAIMonster> m(MONSTER_MAX, "CAIMonster pools");
	return m;
}

static CAIMonster *monster_create()
{
	CAIMonster *self = MonsterPool().GetObject();
	if (!self)
	{
		RunStateError("创建 CAIMonster 失败!");
		return NULL;
	}
	new(self) CAIMonster();
	return self;
}

static void monster_release(CAIMonster *self)
{
	if (!self)
		return;
	self->~CAIMonster();
	MonsterPool().FreeObject(self);
}

CMonster *CMonsterCreator::CreateMonster()
{
	CAIMonster *monster = monster_create();
	if (!monster)
	{
		RunStateError("创建CAIMonster失败!");
		return nullptr;
	}

	return monster;
}

void CMonsterCreator::ReleaseMonster(CMonster *monster)
{
	monster_release((CAIMonster *)monster);
}

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
	SetName(monsterinfo->Name.c_str());
	m_ReliveCD = relivecd;
	m_ReliveX = relivex;
	m_ReliveY = relivey;
	m_ReliveZ = relivez;
	SetSpeed(300);

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
		FSMUpdate();
	}
}

void CMonster::SendMsgToMe(Msg &pMsg, bool bRef)
{
	if (bRef)
	{
		msgtail tail;
		std::unordered_map<uint32, CBaseObj *> *playerlist = GetAoiList();
		std::unordered_map<uint32, CBaseObj *>::iterator iter = playerlist->begin();
		for (; iter != playerlist->end(); ++iter)
		{
			if (iter->second->IsPlayer())
			{
				CPlayer * p = (CPlayer *)iter->second;
				if (FuncUti::isValidCret(p))
				{
					tail.id = p->GetClientID();
					GameGatewayMgr.SendMsg(p->GetGateInfo(), pMsg, &tail, sizeof(tail));
				}
			}
		}
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
