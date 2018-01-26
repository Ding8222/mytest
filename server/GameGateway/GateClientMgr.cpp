#include "stdfx.h"
#include "Client.h"
#include "GateClientMgr.h"
#include "GameConnect.h"

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
					tail.type = msgtail::enum_type_from_client;
					tail.id = cl->GetClientID();
					CGameConnect::Instance().SendMsg(pMsg, &tail, sizeof(tail));
				}
			}
			else
			{
				// 未认证的，进行认证
			}
		}
		}
	}
}