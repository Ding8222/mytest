#include "MonsterCreator.h"
#include "objectpool.h"
#include "serverlog.h"

#define MONSTER_MAX 100000

static objectpool<CMonster> &MonsterPool()
{
	static objectpool<CMonster> m(MONSTER_MAX, "CMonster pools");
	return m;
}

static CMonster *monster_create()
{
	CMonster *self = MonsterPool().GetObject();
	if (!self)
	{
		RunStateError("创建 CMonster 失败!");
		return NULL;
	}
	new(self) CMonster();
	return self;
}

static void monster_release(CMonster *self)
{
	if (!self)
		return;
	self->~CMonster();
	MonsterPool().FreeObject(self);
}

CMonsterCreator::CMonsterCreator()
{

}

CMonsterCreator::~CMonsterCreator()
{

}

CMonster *CMonsterCreator::CreateMonster()
{
	CMonster *monster = monster_create();
	if (!monster)
	{
		RunStateError("创建CMonster失败!");
		return nullptr;
	}
	
	return monster;
}

void CMonsterCreator::ReleaseMonster(CMonster *monster)
{
	monster_release(monster);
}
