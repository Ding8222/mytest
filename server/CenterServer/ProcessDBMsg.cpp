#include "ProcessDBMsg.h"
#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "GlobalDefine.h"
#include "ServerStatusMgr.h"
#include "ServerLog.h"

#include "LoginType.h"
#include "ServerType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"

extern int64 g_currenttime;

void ProcessDBMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
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
		case SVR_SUB_PLAYERDATA:
		{
			svrData::LoadPlayerData msg;
			_CHECK_PARSE_(pMsg, msg);

			netData::SelectPlayerRet SendMsg;
			ClientAuthInfo *_pAuthInfo = CClientAuthMgr::Instance().FindClientAuthInfo(tl->id);
			if (_pAuthInfo)
			{
				ServerStatusInfo *_pGameInfo = CServerStatusMgr::Instance().GetGameServerInfo(msg.nmapid());
				ServerStatusInfo *_pGateInfo = CServerStatusMgr::Instance().GetGateServerInfo();
				if (_pGameInfo && _pGateInfo)
				{
					// 返回loginsvr，通知client选角成功，登陆gamegateway
					SendMsg.set_ncode(netData::SelectPlayerRet::EC_SUCC);
					SendMsg.set_nserverid(_pGateInfo->nServerID);
					SendMsg.set_sip(_pGateInfo->chIP);
					SendMsg.set_nport(_pGateInfo->nPort);
					SendMsg.set_nmapid(msg.nmapid());

					// 将角色数据和认证信息发送到gamegateway
					svrData::ClientAccount sendMsg;
					sendMsg.mutable_data()->MergeFrom(msg);
					sendMsg.set_account(_pAuthInfo->Account);
					sendMsg.set_secret(_pAuthInfo->Secret);
					sendMsg.set_ngameid(_pGameInfo->nServerID);
					CCentServerMgr::Instance().SendMsgToServer(sendMsg, SERVER_TYPE_MAIN, SVR_SUB_CLIENT_ACCOUNT, ServerEnum::EST_GATE, 0, _pGateInfo->nServerID);
				}
				else
				{
					if (!_pGameInfo)
						RunStateError("没有找到地图：%d所在的逻辑服务器！", msg.nmapid());
					if (!_pGateInfo)
						RunStateError("为账号：%s分配网关失败！", msg.account().c_str());
					SendMsg.set_ncode(netData::SelectPlayerRet::EC_SERVER);
				}
			}
			else
			{
				RunStateError("没有找到clientid：%d的认证信息！", tl->id);
				SendMsg.set_ncode(netData::SelectPlayerRet::EC_AUTH);
			}

			CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_SELECT_PLAYER_RET, ServerEnum::EST_LOGIN, tl->id);
			break;
		}
		}
		break;
	}
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_AUTH_RET:
		{
			netData::AuthRet msg;
			_CHECK_PARSE_(pMsg, msg);
			msg.set_ncode(netData::AuthRet::EC_AUTHINFO);

			ClientAuthInfo *clientinfo = CClientAuthMgr::Instance().FindClientAuthInfo(tl->id);
			if (clientinfo)
			{
				msg.set_account(clientinfo->Account);
				msg.set_ncode(netData::AuthRet::EC_SUCC);
				CCentServerMgr::Instance().SendMsgToServer(msg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, ServerEnum::EST_LOGIN, tl->id);
			}
			else
				RunStateError("处理DB认证返回失败！没有找到账号%s信息！clientid：%d", msg.account().c_str(), tl->id);
			break;
		}
		case LOGIN_SUB_PLAYER_LIST_RET:
		case LOGIN_SUB_CREATE_PLAYER_RET:
		case LOGIN_SUB_SELECT_PLAYER_RET:
		{
			CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_LOGIN, tl->id);
			break;
		}
		}
		break;
	}
	}
}
