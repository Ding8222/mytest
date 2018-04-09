#include "NPC.h"

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
	if (IsWaitRemove())
		return;

	LeaveScene();

	//放在最后
	SetWaitRemove();
}

bool CNPC::Init(int32 npcid)
{
	return true;
}
