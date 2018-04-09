#pragma once
#include "NPC.h"

class CNPCCreator
{
public:
	CNPCCreator();
	~CNPCCreator();

	static CNPC *CreateNPC();
	static void ReleaseNPC(CNPC *npc);
private:
};