#include "PlayerOperate.h"
#include "Team.h"

#include "TeamType.h"
#include "Team.pb.h"

void DoTeamMsg(CPlayer *pPlayer, Msg *pMsg)
{
	switch (pMsg->GetSubType())
	{
	case TEAM_SUB_CREATE:
	case TEAM_SUB_APPLY:
	case TEAM_SUB_APPLY_RESPONSE:
	case TEAM_SUB_INVITE:
	case TEAM_SUB_INVITE_RESPONSE:
	case TEAM_SUB_EXIT:
	case TEAM_SUB_TICK:
	{
		FuncUti::SendMsgToCenter(pPlayer, *pMsg);
		break;
	}
	}
}
