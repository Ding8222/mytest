﻿#include "stdfx.h"
#include "GateClientMgr.h"
#include "GameConnect.h"
#include "connector.h"
#include "config.h"
#include "ClientAuth.h"

extern int64 g_currenttime;

CGameConnect::CGameConnect()
{

}

CGameConnect::~CGameConnect()
{

}

bool CGameConnect::Init()
{
	if (!CConnectMgr::AddNewConnect(
		CConfig::Instance().GetGameServerIP().c_str(),
		CConfig::Instance().GetGameServerPort(),
		CConfig::Instance().GetGameServerID()
	))
	{
		log_error("添加逻辑服务器失败!");
		return false;
	}

	return CConnectMgr::Init(
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime(),
		CConfig::Instance().GetListenPort()
	);
}

void CGameConnect::ServerRegisterSucc(int id, const char *ip, int port)
{

}

void CGameConnect::ConnectDisconnect(connector *)
{

}

void CGameConnect::ProcessMsg(connector *_con)
{
	Msg *pMsg = NULL;
	for (;;)
	{
		pMsg = _con->GetMsg();
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
				_con->SetRecvPingTime(g_currenttime);
				break;
			}
			case SVR_SUB_CLIENT_TOKEN:
			{
				msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
				pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

				CClientAuth::Instance().AddAuthInfo(pMsg);
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
			msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
			pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));
			// 转发给client
			CGateClientMgr::Instance().SendMsg(tl->id, pMsg);
		}
		}
	}
}