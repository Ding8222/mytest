#include "GateCenterConnect.h"
#include "GateClientMgr.h"
#include "serverinfo.h"
#include "connector.h"
#include "config.h"
#include "serverlog.h"
#include "msgbase.h"
#include "GameConnect.h"
#include "Client.h"

#include "ServerType.h"
#include "ClientAuth.h"
#include "LoginType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"

extern int64 g_currenttime;

CGateCenterConnect::CGateCenterConnect()
{

}

CGateCenterConnect::~CGateCenterConnect()
{

}

bool CGateCenterConnect::Init()
{
	if (!CConnectMgr::AddNewConnect(
		CConfig::Instance().GetCenterServerIP(),
		CConfig::Instance().GetCenterServerPort(),
		CConfig::Instance().GetCenterServerID()
	))
	{
		RunStateError("添加中心服务器失败!");
		return false;
	}

	return CConnectMgr::Init(
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime()
	);
}

void CGateCenterConnect::Destroy()
{
	CConnectMgr::Destroy();
}

void CGateCenterConnect::ServerRegisterSucc(int id, const char *ip, int port)
{
	// 发送负载信息给Center
	svrData::ServerLoadInfo sendMsg;
	sendMsg.set_nlineid(CConfig::Instance().GetLineID());
	sendMsg.set_nmaxclient(CConfig::Instance().GetMaxClientNum());
	sendMsg.set_nnowclient(CClientAuth::Instance().GetClientSize());
	sendMsg.set_nport(CConfig::Instance().GetListenPort());
	sendMsg.set_sip(CConfig::Instance().GetServerIP());
	SendMsgToServer(CConfig::Instance().GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_SERVER_LOADINFO);
}

void CGateCenterConnect::ConnectDisconnect(connector *)
{

}

void CGateCenterConnect::ProcessMsg(connector *_con)
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
			case SVR_SUB_CLIENT_ACCOUNT:
			{
				CClientAuth::Instance().AddAccountInfo(pMsg);
				break;
			}
			case SVR_SUB_KICKCLIENT:
			{
				CClientAuth::Instance().KickClient(tl->id);
				break;
			}
			case SVR_SUB_CHANGELINE_RET:
			{
				svrData::ChangeLineRet msg;
				_CHECK_PARSE_(pMsg, msg);
				if (msg.ncode() != svrData::ChangeLineRet::EC_SUCC)
				{
					ClientAuthInfo *info = CClientAuth::Instance().FindAuthInfo(tl->id);
					if(info)
						RunStateError("玩家换线失败！踢下线！账号：%s，目标地图：%d，目标线路：%d", info->Account.c_str(), msg.nmapid(), msg.nlineid());
					else
						RunStateError("玩家换线失败！踢下线！目标地图：%d，目标线路：%d", msg.nmapid(), msg.nlineid());
					CClientAuth::Instance().KickClient(tl->id);
				}
				CGateClientMgr::Instance().SendMsg(tl->id, pMsg);
				break;
			}
			case SVR_SUB_CHANGELINE:
			{
				svrData::ChangeLine msg;
				_CHECK_PARSE_(pMsg, msg);

				CClient *cl = CGateClientMgr::Instance().FindClientByClientID(tl->id);
				if(cl)
				{
					CClientAuth::Instance().UpdateGameSvrID(tl->id, msg.ngameid());
					cl->SetLogicServerID(msg.ngameid());
					svrData::LoadPlayerData Data;
					Data.CopyFrom(msg.data());
					Data.set_bchangeline(true);
					CGameConnect::Instance().SendMsgToServer(msg.ngameid(), Data, SERVER_TYPE_MAIN, SVR_SUB_PLAYERDATA, tl->id);
				}
				break;
			}
			}
			break;
		}
		case LOGIN_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case LOGIN_SUB_PLAYER_LIST_RET:
			case LOGIN_SUB_CREATE_PLAYER_RET:
			case LOGIN_SUB_SELECT_PLAYER_RET:
			{
				CGateClientMgr::Instance().SendMsg(tl->id, pMsg);
				break;
			}
			}
			break;
		}
		}
	}
}
