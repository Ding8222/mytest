#include "DoMsg.h"
#include "CenterPlayerMgr.h"
#include "ClientAuthMgr.h"
#include "ServerStatusMgr.h"
#include "GlobalDefine.h"
#include "CentServerMgr.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

void DoServerMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetSubType())
	{
	case SVR_SUB_PING:
	{
		info->SendMsg(pMsg);
		info->SetPingTime(g_currenttime);
		break;
	}
	case SVR_SUB_ADD_PLAYER_TO_CENTER:
	{
		svrData::AddPlayerToCenter msg;
		_CHECK_PARSE_(pMsg, msg);

		CCenterPlayerMgr::Instance().AddPlayer(msg.nguid(), msg.account(), msg.nclientid(), info->GetServerID(), msg.ngateid());
		CClientAuthMgr::Instance().SetGuid(msg.account(), msg.nguid());
		break;
	}
	case SVR_SUB_CHANGELINE:
	{
		// client换线切图
		svrData::ChangeLine msg;
		_CHECK_PARSE_(pMsg, msg);

		svrData::ChangeLineRet SendMsg;
		SendMsg.set_nmapid(msg.nmapid());
		SendMsg.set_nlineid(msg.nlineid());
		ServerStatusInfo *_pGameInfo = CServerStatusMgr::Instance().GetGameServerInfo(msg.nmapid());
		if (_pGameInfo)
		{
			msg.set_ngameid(_pGameInfo->nServerID);
			CCenterPlayerMgr::Instance().UpdatePlayerGameSvr(tl->id, _pGameInfo->nServerID);
			CCentServerMgr::Instance().SendMsgToServer(msg, SERVER_TYPE_MAIN, SVR_SUB_CHANGELINE, ServerEnum::EST_GATE, tl->id);
			return;
		}
		else
			SendMsg.set_ncode(svrData::ChangeLineRet::EC_SERVER);

		CCentServerMgr::Instance().SendMsgToServer(SendMsg, SERVER_TYPE_MAIN, SVR_SUB_CHANGELINE_RET, ServerEnum::EST_GATE, tl->id);
		break;
	}
	}
}