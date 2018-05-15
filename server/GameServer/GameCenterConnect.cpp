#include "GameCenterConnect.h"
#include "connector.h"
#include "config.h"
#include "PlayerMgr.h"
#include "Utilities.h"
#include "serverlog.h"
#include "MapConfig.h"
#include "MapInfo.h"
#include "DoMsgFromCenter.h"

#include "MainType.h"
#include "ServerType.h"
#include "ServerMsg.pb.h"

CGameCenterConnect::CGameCenterConnect()
{

}

CGameCenterConnect::~CGameCenterConnect()
{

}

bool CGameCenterConnect::Init()
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

void CGameCenterConnect::Destroy()
{
	CConnectMgr::Destroy();
}

void CGameCenterConnect::ServerRegisterSucc(connector *)
{
	// 发送负载信息给Center
	svrData::ServerLoadInfo sendMsg;
	sendMsg.set_nlineid(Config.GetLineID());
	sendMsg.set_nmaxclient(Config.GetMaxClientNum());
	sendMsg.set_nnowclient(PlayerMgr.GetPlayerSize());
	sendMsg.set_nport(Config.GetListenPort());
	sendMsg.set_sip(Config.GetServerIP());
	const std::list<CMapInfo*> maplist = MapConfig.GetMapList();
	for (auto &iter : maplist)
	{
		sendMsg.add_mapid(iter->GetMapID());
	}
	std::list<CPlayer *> &temp = PlayerMgr.GetPlayerList();
	for (auto &i : temp)
	{
		CPlayer *player = i;
		if (FuncUti::isValidCret(player))
		{
			svrData::AddPlayerToCenter *info = sendMsg.add_info();
			if (info)
			{
				info->set_nguid(player->GetGuid());
				info->set_ngameid(player->GetGameID());
				info->set_nclientid(player->GetClientID());
				info->set_ngateid(player->GetGateID());
				info->set_account(player->GetAccount());
			}
		}
	}
	SendMsgToServer(Config.GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_SERVER_LOADINFO);
}

void CGameCenterConnect::ConnectDisconnect(connector *_con)
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
			DoServerMsgFromCenter(_con, pMsg, tl);
			break;
		}
		case TEAM_TYPE_MAIN:
		{
			DoTeamMsgFromCenter(_con, pMsg, tl);
			break;
		}
		}
	}
}
