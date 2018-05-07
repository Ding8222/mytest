#include "Client.h"
#include "GateClientMgr.h"
#include "GameConnect.h"
#include "GateCenterConnect.h"
#include "Config.h"
#include "ClientAuth.h"
#include "msgbase.h"

#include "ServerType.h"
#include "LoginType.h"
#include "ClientType.h"
#include "ServerMsg.pb.h"
#include "ClientMsg.pb.h"

extern int64 g_currenttime;

CGateClientMgr::CGateClientMgr()
{

}

CGateClientMgr::~CGateClientMgr()
{
	Destroy();
}

void CGateClientMgr::Destroy()
{
	CClientMgr::Destroy();
}

int32 CGateClientMgr::OnNewClient()
{
	if (!GateCenterConnect.IsAlreadyRegister(Config.GetCenterServerID()))
		return 0;

	int32 nClientID = CClientMgr::OnNewClient();
	if (nClientID == 0)
		return 0;

	svrData::UpdateServerLoad sendMsg;
	sendMsg.set_nclientcountmax(Config.GetMaxClientNum());
	sendMsg.set_nclientcountnow(GetClientConnectNum());

	GateCenterConnect.SendMsgToServer(Config.GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_UPDATE_LOAD);
	return nClientID;
}

void CGateClientMgr::OnClientDisconnect(CClient *cl)
{
	if (cl->IsAlreadyAuth())
	{
		ClientAuth.KickClient(cl->GetClientID(), false);
	}
	CClientMgr::OnClientDisconnect(cl);
}

void CGateClientMgr::ReleaseAllClient()
{
	CClientMgr::ReleaseAllClient();
	ClientAuth.Destroy();
}

void CGateClientMgr::ProcessClientMsg(CClient *cl)
{
	Msg *pMsg = NULL;
	for (int i = 0; i < 2; ++i)
	{
		pMsg = cl->GetMsg();
		if (!pMsg)
			break;

		if (cl->IsAlreadyLogin())
		{
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
				default:
				{
					// 登录成功的,转发至GameServer
					if (GameConnect.IsAlreadyRegister(cl->GetLogicServer()))
					{
						GameConnect.SendMsgToServer(cl->GetLogicServer(), *pMsg, cl->GetClientID());
					}
				}
				}
				break;
			}
			default:
			{
				// 登录成功的,转发至GameServer
				if (GameConnect.IsAlreadyRegister(cl->GetLogicServer()))
				{
					GameConnect.SendMsgToServer(cl->GetLogicServer(), *pMsg, cl->GetClientID());
				}
			}
			}
		}
		else
		{
			// 未登录
			ProcessClientAuth(cl, pMsg);
		}
	}
}

void CGateClientMgr::ProcessClientAuth(CClient *cl, Msg *pMsg)
{
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
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_LOGIN:
		{
			ClientAuth.QueryLogin(pMsg, cl);
			break;
		}
		}
		break;
	}
	}
}

void CGateClientMgr::SetClientAlreadyLogin(int32 clientid, int32 gameserverid)
{
	CClient *cl = FindClient(clientid);
	if (cl)
	{
		if (gameserverid > 0)
		{
			cl->SetLogicServerID(gameserverid);
			cl->SetAlreadyLogin();
		}
		else
		{
			cl->SetLogicServerID(0);
			cl->SetNotLogin();
		}
	}
}