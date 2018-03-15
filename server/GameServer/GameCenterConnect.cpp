#include "GameCenterConnect.h"
#include "GameGatewayMgr.h"
#include "serverinfo.h"
#include "connector.h"
#include "config.h"
#include "PlayerMgr.h"
#include "Utilities.h"
#include "serverlog.h"
#include "MapConfig.h"
#include "MapInfo.h"
#include "SceneMgr.h"

#include "ServerType.h"
#include "ClientType.h"
#include "ClientMsg.pb.h"
#include "ServerMsg.pb.h"

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

void CGameCenterConnect::Destroy()
{
	CConnectMgr::Destroy();
}

void CGameCenterConnect::ServerRegisterSucc(int id, const char *ip, int port)
{
	// 发送负载信息给Center
	svrData::ServerLoadInfo sendMsg;
	sendMsg.set_nlineid(CConfig::Instance().GetLineID());
	sendMsg.set_nmaxclient(CConfig::Instance().GetMaxClientNum());
	sendMsg.set_nnowclient(CPlayerMgr::Instance().GetPlayerSize());
	sendMsg.set_nport(CConfig::Instance().GetListenPort());
	sendMsg.set_sip(CConfig::Instance().GetServerIP());
	const std::list<CMapInfo*> maplist = CMapConfig::Instance().GetMapList();
	for (auto &iter : maplist)
	{
		sendMsg.add_mapid(iter->GetMapID());
	}
	std::list<CPlayer *> &temp = CPlayerMgr::Instance().GetPlayerList();
	for (auto &i : temp)
	{
		CPlayer *player = i;
		if (FuncUti::isValidCret(player))
		{
			svrData::AddPlayerToCenter *info = sendMsg.add_info();
			if (info)
			{
				info->set_nguid(player->GetGuid());
				info->set_nclientid(player->GetClientID());
				info->set_ngateid(player->GetGateID());
			}
		}
	}
	SendMsgToServer(CConfig::Instance().GetCenterServerID(), sendMsg, SERVER_TYPE_MAIN, SVR_SUB_SERVER_LOADINFO);
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
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				_con->SetRecvPingTime(g_currenttime);
				break;
			}
			}
			break;
		}
		}
	}
}
