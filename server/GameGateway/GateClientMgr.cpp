#include "stdfx.h"
#include "Client.h"
#include "GateClientMgr.h"
#include "GameConnect.h"
#include "Config.h"

#include "LoginType.h"
#include "Login.pb.h"

extern int64 g_currenttime;

CGateClientMgr::CGateClientMgr()
{

}

CGateClientMgr::~CGateClientMgr()
{

}

int64 CGateClientMgr::OnNewClient()
{
	if (!CGameConnect::Instance().IsReady())
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
		case SERVER_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
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
				if (CGameConnect::Instance().IsReady())
				{
					msgtail tail;
					tail.id = cl->GetClientID();
					CGameConnect::Instance().SendMsg(*pMsg, &tail, sizeof(tail));
				}
			}
			else
			{
				// 未认证
				switch (pMsg->GetMainType())
				{
				case LOGIN_TYPE_MAIN:
				{
					switch (pMsg->GetSubType())
					{
					case SVR_SUB_PING:
					{
						cl->SendMsg(pMsg);
						cl->SetPingTime(g_currenttime);
						break;
					}
					case LOGIN_SUB_LOGIN:
					{
						netData::Login msg;
						_CHECK_PARSE_(pMsg, msg);

						if (CGameConnect::Instance().AddNewClientSvrID(msg.stoken(), ServerEnum::EST_GATE, CConfig::Instance().GetServerID(), cl->GetClientID()))
						{
							// 认证成功
							MessagePack msg;
							msg.SetMainType(SERVER_TYPE_MAIN);
							msg.SetSubType(SVR_SUB_NEW_CLIENT);
							CGameConnect::Instance().SendMsgToServer(msg, cl->GetClientID());

							cl->SetAlreadyAuth();
						}
						else
						{
							// 认证失败
						}
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
		}
		}
	}
}