/*
* NPC对象
* Copyright (C) ddl
* 2018
*/

#pragma once
#include "BaseObj.h"

class CNPC :public CBaseObj
{
public:
	CNPC();
	~CNPC();

	virtual void Run();
	virtual void Die();

	bool Init(int32 npcid);
};