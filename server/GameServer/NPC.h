/*
* NPC对象
* Copyright (C) ddl
* 2018
*/

#pragma once
#include "BaseObj.h"

class CNPCCreator
{
public:
	CNPCCreator() {}
	~CNPCCreator() {}

	static CNPC *CreateNPC();
	static void ReleaseNPC(CNPC *npc);
private:
};

class CNPC :public CBaseObj
{
public:
	CNPC();
	~CNPC();

	virtual void Run();
	virtual void Die();

	bool Init(int32 npcid);
};