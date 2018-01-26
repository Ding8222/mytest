#include "stdfx.h"
#include "GameCenterConnect.h"
#include "connector.h"
#include "config.h"

extern int64 g_currenttime;

CGameCenterConnect::CGameCenterConnect()
{

}

CGameCenterConnect::~CGameCenterConnect()
{

}

bool CGameCenterConnect::Init()
{
	return CServerConnect::Init(CConfig::Instance().GetCenterServerIP().c_str(),
		CConfig::Instance().GetCenterServerPort(),
		CConfig::Instance().GetCenterServerID(),
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime());
}

void CGameCenterConnect::ConnectDisconnect()
{

}

void CGameCenterConnect::ProcessMsg(connector *_con)
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