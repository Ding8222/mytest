#include "GlobalDefine.h"
#include "ProcessServerMsg.h"
#include "ServerStatusMgr.h"
#include "CenterPlayerMgr.h"
#include "CentServerMgr.h"
#include "ClientAuthMgr.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

extern int64 g_currenttime;

void ProcessGameMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case SERVER_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case SVR_SUB_SERVER_LOADINFO:
		{
			// 添加服务器负载信息
			CServerStatusMgr::Instance().AddGameServer(info, pMsg);
			break;
		}
		case SVR_SUB_UPDATE_LOAD:
		{
			// 更新服务器负载信息
			svrData::UpdateServerLoad msg;
			_CHECK_PARSE_(pMsg, msg);

			CServerStatusMgr::Instance().UpdateGameServerLoad(info->GetServerID(), msg.nclientcountnow(), msg.nclientcountmax());
			break;
		}
		case SVR_SUB_PLAYERDATA:
		{
			CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, tl->id);
			break;
		}
		case SVR_SUB_DEL_CLIENT:
		{
			// client断开
			svrData::DelClient msg;
			_CHECK_PARSE_(pMsg, msg);

			CCenterPlayerMgr::Instance().DelPlayer(tl->id);
			CClientAuthMgr::Instance().SetPlayerOffline(msg.account());
			break;
		}
		default:
			DoServerMsg(info, pMsg, tl);
		}
		break;
	}
	case TEAM_TYPE_MAIN:
	{
		DoTeamMsg(info, pMsg, tl);
		break;
	}
	}
}