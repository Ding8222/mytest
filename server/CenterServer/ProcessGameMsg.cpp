#include "ProcessGameMsg.h"
#include "ServerStatusMgr.h"
#include "CenterPlayerMgr.h"
#include "CentServerMgr.h"
#include "ClientAuthMgr.h"

#include "ServerType.h"
#include "LoginType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"

extern int64 g_currenttime;

void ProcessGameMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case SERVER_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case SVR_SUB_PING:
		{
			info->SendMsg(pMsg);
			info->SetPingTime(g_currenttime);
			break;
		}
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
		case SVR_SUB_ADD_PLAYER_TO_CENTER:
		{
			svrData::AddPlayerToCenter msg;
			_CHECK_PARSE_(pMsg, msg);

			CCenterPlayerMgr::Instance().AddPlayer(msg.nguid(), msg.nclientid(), info->GetServerID(), msg.ngateid());
			CClientAuthMgr::Instance().SetGuid(msg.account(),msg.nguid());
			break;
		}
		case SVR_SUB_CHANGELINE:
		{
			// client换线切图
			svrData::ChangeLine msg;
			_CHECK_PARSE_(pMsg, msg);

			svrData::ChangeLineRet SendMsg;
			ServerStatusInfo *_pGameInfo = CServerStatusMgr::Instance().GetGameServerInfo(msg.nmapid());
			if (_pGameInfo)
			{
				msg.set_ngameid(_pGameInfo->nServerID);
				CCenterPlayerMgr::Instance().UpdatePlayerGameSvr(tl->id, _pGameInfo->nServerID);
				CCentServerMgr::Instance().SendMsgToServer(msg, SERVER_TYPE_MAIN, SVR_SUB_CHANGELINE, ServerEnum::EST_GATE, tl->id);
				return;
			}
			else
				SendMsg.set_ncode(netData::SelectPlayerRet::EC_SERVER);

			CCentServerMgr::Instance().SendMsgToServer(SendMsg, SERVER_TYPE_MAIN, SVR_SUB_CHANGELINE_RET, ServerEnum::EST_GATE, tl->id);
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
		}
		break;
	}
	}
}