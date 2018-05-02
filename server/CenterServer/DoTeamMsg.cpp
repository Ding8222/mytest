#include "GlobalDefine.h"
#include "DoMsg.h"
#include "TeamMgr.h"
#include "CenterPlayerMgr.h"

#include "TeamType.h"
#include "Team.pb.h"

void DoTeamMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetSubType())
	{
	case TEAM_SUB_CREATE:
	{
		CTeamMgr::Instance().QueryCreateTeam(tl->id);
		break;
	}
	case TEAM_SUB_APPLY:
	{
		netData::ApplyTeam msg;
		_CHECK_PARSE_(pMsg, msg);

		CTeamMgr::Instance().QueryApplyTeam(tl->id, msg.teamid());
		break;
	}
	case TEAM_SUB_APPLY_RESPONSE:
	{
		netData::ResponseApplyTeam msg;
		_CHECK_PARSE_(pMsg, msg);

		CTeamMgr::Instance().ReasponseApplyTeam(tl->id, msg.playerguid(), msg.agree());
		break;
	}
	case TEAM_SUB_INVITE:
	{
		netData::InviteTeam msg;
		_CHECK_PARSE_(pMsg, msg);

		CTeamMgr::Instance().QueryInviteTeam(tl->id, msg.playerguid());
		break;
	}
	case TEAM_SUB_INVITE_RESPONSE:
	{
		netData::ResponseInviteTeam msg;
		_CHECK_PARSE_(pMsg, msg);

		CTeamMgr::Instance().ReasponseInviteTeam(tl->id, msg.playerguid(), msg.agree());
		break;
	}
	case TEAM_SUB_EXIT:
	{
		CTeamMgr::Instance().QueryExitTeam(tl->id);
		break;
	}
	case TEAM_SUB_TICK:
	{
		netData::KickTeamMember msg;
		_CHECK_PARSE_(pMsg, msg);

		CTeamMgr::Instance().QueryKickPlayer(tl->id, msg.playerguid());
		break;
	}
	}
}