#include "stdfx.h"
#include "ServerConnect.h"
#include "GameConnect.h"
#include "connector.h"
#include "config.h"

extern int64 g_currenttime;

CGameConnect::CGameConnect()
{

}

CGameConnect::~CGameConnect()
{

}

bool CGameConnect::Init()
{
	return CServerConnect::Init(CConfig::Instance().GetGameServerIP().c_str(),
		CConfig::Instance().GetGameServerPort(),
		CConfig::Instance().GetGameServerID(),
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime());
}

void CGameConnect::ConnectDisconnect()
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
			default:
			{
			}
			}
			break;
		}
		default:
		{
		}
		}
	}
}