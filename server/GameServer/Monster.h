/*
* 怪物对象
* Copyright (C) ddl
* 2018
*/

#pragma once
#include "BaseObj.h"

class CMonster :public CBaseObj
{
public:
	CMonster();
	~CMonster();

	virtual void Run();
	virtual void Die();
};