#include "stdfx.h"
#include "GameCenterConnect.h"
#include "GameGatewayMgr.h"
#include "serverinfo.h"
#include "connector.h"
#include "config.h"
#include "ClientSvrMgr.h"

extern int64 g_currenttime;

CGameCenterConnect::CGameCenterConnect()
{

}

CGameCenterConnect::~CGameCenterConnect()
{

}

bool CGameCenterConnect::Init()
{
	if (!CConnectMgr::AddNewConnect(
		CConfig::Instance().GetCenterServerIP().c_str(),
		CConfig::Instance().GetCenterServerPort(),
		CConfig::Instance().GetCenterServerID()
	))
	{
		log_error("添加中心服务器失败!");
		return false;
	}

	return CConnectMgr::Init(
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime()
	);
}

void CGameCenterConnect::ServerRegisterSucc(int id, const char *ip, int port)
{
	// 如果Gate准备好了，发送一次负载信息给Center
	if (CGameGatewayMgr::Instance().IsAlreadyRegister(CGameGatewayMgr::Instance().GetGateID()))
	{
		serverinfo *svr = CGameGatewayMgr::Instance().FindServer(CGameGatewayMgr::Instance().GetGateID(), ServerEnum::EST_GATE);
		if (svr->GetIP())
		{
			svrData::ServerLoadInfo sendMsg;
			sendMsg.set_nmaxclient(0);
			sendMsg.set_nnowclient(CClientSvrMgr::Instance().GetClientSvrSize());
			sendMsg.set_nport(CConfig::Instance().GetListenPort());
			sendMsg.set_sip(CConfig::Instance().GetServerIP());
			sendMsg.set_ngateport(svr->GetPort());
			sendMsg.set_sgateip(svr->GetIP());
			CGameCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_SERVER_LOADINFO);
		}
	}
}

void CGameCenterConnect::ConnectDisconnect(connector *)
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

		msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
		pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

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
				// 转发给Gate
				CGameGatewayMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_GATE, 0, tl->id);
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
