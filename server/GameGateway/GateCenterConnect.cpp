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
		Config.GetCenterServerIP(),
		Config.GetCenterServerPort(),
		Config.GetCenterServerID(),
		Config.GetCenterServerName()
	))
	{
		RunStateError("添加中心服务器失败!");
		return false;
	}

	return CConnectMgr::Init(
		Config.GetServerID(),
		Config.GetServerType(),
		Config.GetServerName(),
		Config.GetPingTime(),
		Config.GetOverTime()
	);
}

void CGateCenterConnect::Destroy()
{
	CConnectMgr::Destroy();
}

void CGateCenterConnect::ServerRegisterSucc(connector * con)
{
	// 发送负载信息给Center
	svrData::ServerLoadInfo sendMsg;
	sendMsg.set_nlineid(Config.GetLineID());
	sendMsg.set_nmaxclient(Config.GetMaxClientNum());
	sendMsg.set_nnowclient(ClientAuth.GetClientSize());
	sendMsg.set_nport(Config.GetListenPort());
	sendMsg.set_sip(Config.GetServerIP());
	SendMsgToServer(Config.GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_SERVER_LOADINFO);
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
				ClientAuth.AddAccountInfo(pMsg);
				break;
			}
			case SVR_SUB_KICKCLIENT:
			{
				ClientAuth.KickClient(static_cast<int32>(tl->id));
				break;
			}
			case SVR_SUB_CHANGELINE_RET:
			{
				svrData::ChangeLineRet msg;
				_CHECK_PARSE_(pMsg, msg);
				if (msg.ncode() != svrData::ChangeLineRet::EC_SUCC)
				{
					ClientAuthInfo *info = ClientAuth.FindAuthInfo(static_cast<int32>(tl->id));
					if(info)
						RunStateError("玩家换线失败！踢下线！账号：%s，目标地图：%d，目标线路：%d", info->Account.c_str(), msg.nmapid(), msg.nlineid());
					else
						RunStateError("玩家换线失败！踢下线！目标地图：%d，目标线路：%d", msg.nmapid(), msg.nlineid());
					ClientAuth.KickClient(static_cast<int32>(tl->id));
				}
				GateClientMgr.SendMsg(static_cast<int32>(tl->id), pMsg);
				break;
			}
			case SVR_SUB_CHANGELINE:
			{
				svrData::ChangeLine msg;
				_CHECK_PARSE_(pMsg, msg);

				CClient *cl = GateClientMgr.FindClientByClientID(static_cast<int32>(tl->id));
				if(cl)
				{
					ClientAuth.UpdateGameSvrID(static_cast<int32>(tl->id), msg.ngameid());
					cl->SetLogicServerID(msg.ngameid());
					svrData::LoadPlayerData Data;
					Data.CopyFrom(msg.data());
					Data.set_bchangeline(true);
					GameConnect.SendMsgToServer(msg.ngameid(), Data, SERVER_TYPE_MAIN, SVR_SUB_PLAYERDATA, tl->id);
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
				GateClientMgr.SendMsg(static_cast<int32>(tl->id), pMsg);
				break;
			}
			}
			break;
		}
		}
	}
}
