#include "Client.h"
#include "GateClientMgr.h"
#include "GameConnect.h"
#include "GateCenterConnect.h"
#include "Config.h"
#include "ClientAuth.h"
#include "msgbase.h"

#include "MainType.h"
#include "ServerType.h"
#include "LoginType.h"
#include "ClientType.h"
#include "ServerMsg.pb.h"

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

int64 CGateClientMgr::OnNewClient()
{
	if (!CGameConnect::Instance().IsAlreadyRegister(CConfig::Instance().GetGameServerID()))
		return 0;

	if (!CGateCenterConnect::Instance().IsAlreadyRegister(CConfig::Instance().GetCenterServerID()))
		return 0;

	int64 nClientID = CClientMgr::OnNewClient();
	if (nClientID == 0)
		return 0;

	{
		svrData::AddNewClient sendMsg;
		CGateCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_NEW_CLIENT, nClientID);
	}

	svrData::UpdateServerLoad sendMsg;
	sendMsg.set_nclientcountmax(CConfig::Instance().GetMaxClientNum());
	sendMsg.set_nclientcountnow(GetClientConnectNum());

	CGateCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_UPDATE_LOAD);
	return nClientID;
}

void CGateClientMgr::OnClientDisconnect(CClient *cl)
{
	if (cl->IsAlreadyAuth())
	{
		svrData::DelClient sendMsg;
		sendMsg.set_nclientid(cl->GetClientID());
		CGameConnect::Instance().SendMsgToServer(CConfig::Instance().GetGameServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_DEL_CLIENT, cl->GetClientID());
		CClientAuth::Instance().DelClient(cl->GetClientID());
	}

	CClientMgr::OnClientDisconnect(cl);
}

void CGateClientMgr::ReleaseAllClient()
{
	CClientMgr::ReleaseAllClient();
	CClientAuth::Instance().Destroy();
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
					if (CGameConnect::Instance().IsAlreadyRegister(CConfig::Instance().GetGameServerID()))
					{
						CGameConnect::Instance().SendMsgToServer(CConfig::Instance().GetGameServerID(), *pMsg, cl->GetClientID());
					}
				}
				}
				break;
			}
			default:
			{
				// 登录成功的,转发至GameServer
				if (CGameConnect::Instance().IsAlreadyRegister(CConfig::Instance().GetGameServerID()))
				{
					CGameConnect::Instance().SendMsgToServer(CConfig::Instance().GetGameServerID(), *pMsg, cl->GetClientID());
				}
			}
			}
		}
		else
		{
			// 未登录（选角）
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
		default:
		{
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
			CClientAuth::Instance().AddNewClient(pMsg, cl);
			break;
		}
		case LOGIN_SUB_PLAYER_LIST:
		{
			CClientAuth::Instance().GetPlayerList(pMsg, cl);
			break;
		}
		case LOGIN_SUB_CREATE_PLAYER:
		{
			CClientAuth::Instance().CreatePlayer(pMsg, cl);
			break;
		}
		case LOGIN_SUB_SELECT_PLAYER:
		{
			CClientAuth::Instance().SelectPlayer(pMsg, cl);
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

void CGateClientMgr::SetClientAlreadyLogin(int64 clientid, bool bLogin)
{
	CClient *cl = FindClient(clientid);
	assert(cl);
	if (cl)
	{
		if (bLogin)
			cl->SetAlreadyLogin();
		else
			cl->SetNotLogin();
	}
}