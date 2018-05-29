#include "GateClientMgr.h"
#include "GameConnect.h"
#include "ClientAuth.h"
#include "connector.h"
#include "config.h"
#include "serverlog.h"
#include "msgbase.h"
#include "GlobalDefine.h"

#include "ServerType.h"
#include "LoginType.h"
#include "ClientType.h"
#include "ServerMsg.pb.h"
#include "ClientMsg.pb.h"
#include "Login.pb.h"

extern int64 g_currenttime;

CGameConnect::CGameConnect()
{

}

CGameConnect::~CGameConnect()
{
	Destroy();
}

bool CGameConnect::Init()
{
	std::list<GameSvr *> List = Config.GetGameSvrList();
	for (auto &i : List)
	{
		if (!CConnectMgr::AddNewConnect(i->ip.c_str(), i->port, i->id, i->name.c_str()))
		{
			RunStateError("添加逻辑服务器失败!");
			return false;
		}
	}

	return CConnectMgr::Init(
		Config.GetServerID(),
		Config.GetServerType(),
		Config.GetServerName(),
		Config.GetPingTime(),
		Config.GetOverTime()
	);
}

void CGameConnect::Destroy()
{
	CConnectMgr::Destroy();
}

void CGameConnect::ConnectDisconnect(connector *con)
{
	GateClientMgr.AsLogicServerDisconnect(con->GetConnectID());
	ClientAuth.AsLogicServerDisconnect(con->GetConnectID());
}

void CGameConnect::ProcessMsg(connector *_con)
{
	Msg *pMsg = NULL;
	for (;;)
	{
		pMsg = _con->GetMsg();
		if (!pMsg)
			break;

		msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
		pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

		if (tl->id < 0)
		{
			int32 nClienNum = abs(static_cast<int32>(tl->id));
			if(nClienNum >0)
			{
				MessagePack *pkmain = (MessagePack *)pMsg;
				pkmain->Begin();
				int32 pbSize = pkmain->GetInt32();
				size_t size = 0;
				MessagePack *pk = (MessagePack *)pkmain->GetBlockRef(pbSize, &size);
				int32 nClientID = 0;
				for (int32 i = 0; i < nClienNum; ++i)
				{
					nClientID = pkmain->GetInt32();
					GateClientMgr.SendMsg(nClientID, pk);
				}
			}
		}
		else
			GateClientMgr.SendMsg(static_cast<int32>(tl->id), pMsg);
		
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
		case LOGIN_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case LOGIN_SUB_LOGIN_RET:
			{
				netData::LoginRet msg;
				_CHECK_PARSE_(pMsg, msg);
				if (msg.ncode() == netData::LoginRet::EC_SUCC)
				{
					ClientAuthInfo *info = ClientAuth.FindAuthInfo(static_cast<int32>((tl->id)));
					if (info)
					{
						GateClientMgr.SetClientAlreadyLogin(static_cast<int32>(tl->id), _con->GetConnectID());
						ClientConnectLog("账号：%s登陆成功！目标服务器：%d", info->Account.c_str(), _con->GetConnectID());
					}
				}
				break;
			}
			}
			break;
		}
		}
	}
}