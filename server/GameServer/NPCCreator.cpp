#include "NPCCreator.h"
#include "objectpool.h"
#include "serverlog.h"

#define MONSTER_MAX 10000

static objectpool<CNPC> &NPCPool()
{
	static objectpool<CNPC> m(MONSTER_MAX, "CNPC pools");
	return m;
}

static CNPC *npc_create()
{
	CNPC *self = NPCPool().GetObject();
	if (!self)
	{
		RunStateError("创建 CNPC 失败!");
		return NULL;
	}
	new(self) CNPC();
	return self;
}

static void npc_release(CNPC *self)
{
	if (!self)
		return;
	self->~CNPC();
	NPCPool().FreeObject(self);
}

CNPCCreator::CNPCCreator()
{

}

CNPCCreator::~CNPCCreator()
{

}

CNPC *CNPCCreator::CreateNPC()
{
	CNPC *npc = npc_create();
	if (!npc)
	{
		RunStateError("创建CNPC失败!");
		return nullptr;
	}

	return npc;
}

void CNPCCreator::ReleaseNPC(CNPC *npc)
{
	npc_release(npc);
}
