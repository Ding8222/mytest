#include "GlobalDefine.h"
#include "DoMsgFromCenter.h"
#include "PlayerMgr.h"
#include "Utilities.h"

#include "TeamType.h"
#include "Team.pb.h"

void DoTeamMsgFromCenter(connector *con, Msg *pMsg, msgtail *tl)
{
	CPlayer *pPlayer = PlayerMgr.FindPlayerByGameID(tl->id);

	switch (pMsg->GetSubType())
	{
	case TEAM_SUB_CREATE_RET:
	{
		if(pPlayer)
			pPlayer->SendMsgToMe(*pMsg);
		break;
	}
	case TEAM_SUB_APPLY_RET:
	{
		break;
	}
	case TEAM_SUB_APPLY_TIPS:
	{
		break;
	}
	case TEAM_SUB_APPLY_LIST:
	{
		break;
	}
	case TEAM_SUB_APPLY_RESPONSE_RET:
	{
		break;
	}
	case TEAM_SUB_INVITE_RET:
	{
		break;
	}
	case TEAM_SUB_INVITE_TIPS:
	{
		break;
	}
	case TEAM_SUB_INVITE_RESPONSE_RET:
	{
		break;
	}
	case TEAM_SUB_EXIT_RET:
	{
		break;
	}
	case TEAM_SUB_TEAM_INFO:
	{
		netData::TeamInfo msg;
		_CHECK_PARSE_(pMsg, msg);

		if (pPlayer)
		{
			pPlayer->InitTeam();
			pPlayer->SetTeamID(msg.teamid());
			pPlayer->SetTeamLeaderGuid(msg.leaderguid());
			for (int i = 0; i < msg.info_size(); ++i)
			{
				const netData::TeamMemberInfo &info = msg.info(i);
				pPlayer->AddTeamMember(info.guid(), info.name(), info.online());
			}
			pPlayer->SendMsgToMe(*pMsg);
		}
		break;
	}
	case TEAM_SUB_UPDATE_MEMBER_INFO:
	{
		break;
	}
	case TEAM_SUB_DISSOLUTION:
	{
		break;
	}
	}
}