#include "GlobalDefine.h"
#include "ProcessServerMsg.h"
#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
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
		case SVR_SUB_PLAYERDATA:
		{
			svrData::LoadPlayerData msg;
			_CHECK_PARSE_(pMsg, msg);

			netData::SelectPlayerRet SendMsg;
			ClientAuthInfo *_pAuthInfo = ClientAuthMgr.FindClientAuthInfo(static_cast<int32>(tl->id));
			if (_pAuthInfo)
			{
				ServerStatusInfo *_pGameInfo = ServerStatusMgr.GetGameServerInfo(msg.nmapid());
				ServerStatusInfo *_pGateInfo = ServerStatusMgr.GetGateServerInfo();
				if (_pGameInfo && _pGateInfo)
				{
					// 返回loginsvr，通知client选角成功，登陆gamegateway
					SendMsg.set_ncode(netData::SelectPlayerRet::EC_SUCC);
					SendMsg.set_nserverid(_pGateInfo->nServerID);
					SendMsg.set_sip(_pGateInfo->IP);
					SendMsg.set_nport(_pGateInfo->nPort);
					SendMsg.set_nmapid(msg.nmapid());

					// 将角色数据和认证信息发送到gamegateway
					svrData::ClientAccount sendMsg;
					sendMsg.mutable_data()->MergeFrom(msg);
					sendMsg.set_account(_pAuthInfo->Account);
					sendMsg.set_secret(_pAuthInfo->Secret);
					sendMsg.set_ngameid(_pGameInfo->nServerID);
					CentServerMgr.SendMsgToServer(sendMsg, SERVER_TYPE_MAIN, SVR_SUB_CLIENT_ACCOUNT, ServerEnum::EST_GATE, 0, _pGateInfo->nServerID);
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
			CentServerMgr.SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_SELECT_PLAYER_RET, ServerEnum::EST_LOGIN, tl->id);
			break;
		}
		default:
			DoServerMsg(info, pMsg, tl);
		}
		break;
	}
	case LOGIN_TYPE_MAIN:
	{
		DoLoginMsg(info, pMsg, tl);
		break;
	}
	}
}
