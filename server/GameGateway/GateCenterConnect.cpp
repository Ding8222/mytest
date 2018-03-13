#include "GateCenterConnect.h"
#include "GateClientMgr.h"
#include "serverinfo.h"
#include "connector.h"
#include "config.h"
#include "serverlog.h"
#include "msgbase.h"
#include "GameConnect.h"

#include "MainType.h"
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
		CConfig::Instance().GetCenterServerIP().c_str(),
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
	sendMsg.set_nmaxclient(0);
	sendMsg.set_nnowclient(CClientAuth::Instance().GetClientSize());
	sendMsg.set_nport(CConfig::Instance().GetListenPort());
	sendMsg.set_sip(CConfig::Instance().GetServerIP());
	sendMsg.set_nsubserverid(CConfig::Instance().GetGameServerID());
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
			case SVR_SUB_NEW_CLIENT_RET:
			{
				svrData::AddNewClientRet msg;
				_CHECK_PARSE_(pMsg, msg);

				netData::LoginRet sendMsg;
				if (msg.ncenterclientid() > 0)
				{
					ClientAuthInfo *info = CClientAuth::Instance().FindAuthInfo(tl->id);
					if (info)
					{
						info->Data.set_ncenterclientid(msg.ncenterclientid());
						CGameConnect::Instance().SendMsgToServer(CConfig::Instance().GetGameServerID(), info->Data, SERVER_TYPE_MAIN, SVR_SUB_LOAD_PLAYERDATA, tl->id);
						return;
					}
					else
						sendMsg.set_ncode(netData::LoginRet::EC_FAIL);
				}
				else
					sendMsg.set_ncode(netData::LoginRet::EC_FAIL);

				CGateClientMgr::Instance().SendMsg(tl->id, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_LOGIN_RET);
				break;
			}
			case SVR_SUB_CLIENT_TOKEN:
			{
				CClientAuth::Instance().AddAuthInfo(pMsg);
				break;
			}
			case SVR_SUB_KICKCLIENT:
			{
				CClientAuth::Instance().KickClient(tl->id);
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
