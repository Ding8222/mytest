#include "Client.h"
#include "Config.h"
#include "LoginClientMgr.h"
#include "LoginCenterConnect.h"
#include "ClientAuth.h"

#include "msgbase.h"
#include "ServerType.h"
#include "ClientType.h"
#include "LoginType.h"
#include "DBSvrType.h"
#include "ServerMsg.pb.h"

extern int64 g_currenttime;

CLoginClientMgr::CLoginClientMgr()
{

}

CLoginClientMgr::~CLoginClientMgr()
{

}

void CLoginClientMgr::Destroy()
{
	CClientMgr::Destroy();
}

int32 CLoginClientMgr::OnNewClient()
{
	if (!LoginCenterConnect.IsAlreadyRegister(Config.GetCenterServerID()))
		return 0;

	int32 nClientID = CClientMgr::OnNewClient();
	if (nClientID == 0)
		return 0;

	return nClientID;
}

void CLoginClientMgr::OnClientDisconnect(CClient *cl)
{
	if (cl->IsAlreadyAuth())
	{
		// 通知Center删除认证信息
		svrData::DelClient sendMsg;
		ClientAuthInfo *info = ClientAuth.GetClientAuthInfo(cl->GetClientID());
		if (info)
		{
			sendMsg.set_account(info->Account);
			if(!info->SelectSucc)
				sendMsg.set_offline(true);
		}
		LoginCenterConnect.SendMsgToServer(Config.GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_DEL_CLIENT, cl->GetClientID());
	}
	// 删除Client中记录的Secret
	ClientAuth.OnClientDisconnect(cl);
	CClientMgr::OnClientDisconnect(cl);
}

void CLoginClientMgr::ProcessClientMsg(CClient *cl)
{
	Msg *pMsg = NULL;
	for (int i = 0; i < 2; ++i)
	{
		pMsg = cl->GetMsg();
		if (!pMsg)
			break;

		switch (pMsg->GetMainType())
		{
		case CLIENT_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case CLIENT_SUB_PING:
			{
				cl->SendMsg(pMsg);
				cl->SetPingTime(g_currenttime);
				break;
			}
			}
			break;
		}
		case LOGIN_TYPE_MAIN:
		{
			if (cl->IsAlreadyAuth())
			{
				// 认证通过的选角
				switch (pMsg->GetSubType())
				{
				case LOGIN_SUB_PLAYER_LIST:
				{
					ClientAuth.GetPlayerList(cl, pMsg);
					break;
				}
				case LOGIN_SUB_CREATE_PLAYER:
				{
					ClientAuth.CreatePlayer(cl, pMsg);
					break;
				}
				case LOGIN_SUB_SELECT_PLAYER:
				{
					ClientAuth.SelectPlayer(cl, pMsg);
					break;
				}
				}
			}
			else
			{
				// 未认证的，进行认证
				switch (pMsg->GetSubType())
				{
				case LOGIN_SUB_HANDSHAKE:
				{
					ClientAuth.HandShake(cl, pMsg);
					break;
				}
				case LOGIN_SUB_CHALLENGE:
				{
					ClientAuth.Challenge(cl, pMsg);
					break;
				}
				case LOGIN_SUB_AUTH:
				{
					ClientAuth.Auth(cl, pMsg);
					break;
				}
				}
				break;
			}
			break;
		}
		}
	}
}