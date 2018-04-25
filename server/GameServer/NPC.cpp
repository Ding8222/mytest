#include "NPC.h"
#include "CSVLoad.h"
#include "objectpool.h"

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


CNPC::CNPC() :CBaseObj(EOT_NPC)
{

}

CNPC::~CNPC()
{

}

void CNPC::Run()
{
	CBaseObj::Run();

}

void CNPC::Die()
{
	if (IsDie())
		return;

	LeaveScene();

	//放在最后
	SetWaitRemove();
}

bool CNPC::Init(int32 npcid)
{
	CSVData::stNPC *npcinfo = CSVData::CNPCDB::FindById(npcid);
	if (!npcinfo)
		return false;

	SetName(npcinfo->Name.c_str());

	return true;
}