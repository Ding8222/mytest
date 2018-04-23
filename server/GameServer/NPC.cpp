#include "NPC.h"
#include "CSVLoad.h"

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
