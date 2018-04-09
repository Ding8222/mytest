#pragma once
#include "Monster.h"

class CMonsterCreator
{
public:
	CMonsterCreator();
	~CMonsterCreator();

	static CMonster *CreateMonster();
	static void ReleaseMonster(CMonster *monster);
private:
};