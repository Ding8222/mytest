#include "stdfx.h"
#include "Client.h"
#include "GateClientMgr.h"
#include "GameConnect.h"
#include "Config.h"
#include "ClientAuth.h"

#include "LoginType.h"
#include "ClientType.h"

extern int64 g_currenttime;

CGateClientMgr::CGateClientMgr()
{

}

CGateClientMgr::~CGateClientMgr()
{

}

int64 CGateClientMgr::OnNewClient()
{
	if (!CGameConnect::Instance().IsAlreadyRegister(CConfig::Instance().GetGameServerID()))
		return 0;

	int64 nClientID = CClientMgr::OnNewClient();
	if (nClientID == 0)
		return 0;

	return nClientID;
}

void CGateClientMgr::OnClientDisconnect(CClient *cl)
{
	CClientMgr::OnClientDisconnect(cl);
}

void CGateClientMgr::ProcessClientMsg(CClient *cl)
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
			default:
			{
			}
			}
			break;
		}
		default:
		{
			if (cl->IsAlreadyAuth())
			{
				// 认证通过的,转发至GameServer
				if (CGameConnect::Instance().IsAlreadyRegister(CConfig::Instance().GetGameServerID()))
				{
					CGameConnect::Instance().SendMsgToServer(CConfig::Instance().GetGameServerID(),*pMsg,cl->GetClientID());
				}
			}
			else
			{
				// 未认证
				ProcessClientAuth(cl, pMsg);
			}
		}
		}
	}
}

void CGateClientMgr::ProcessClientAuth(CClient *cl, Msg *pMsg)
{
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_LOGIN:
		{
			CClientAuth::Instance().AddNewClient(pMsg, cl);
			break;
		}
		default:
		{
		}
		}
		break;
	}
	}
}