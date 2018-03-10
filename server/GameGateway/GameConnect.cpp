#include "GateClientMgr.h"
#include "GameConnect.h"
#include "ClientAuth.h"
#include "connector.h"
#include "config.h"
#include "serverlog.h"
#include "msgbase.h"

#include "MainType.h"
#include "ServerType.h"
#include "ClientType.h"
#include "ServerMsg.pb.h"
#include "ClientMsg.pb.h"

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
	if (!CConnectMgr::AddNewConnect(
		CConfig::Instance().GetGameServerIP().c_str(),
		CConfig::Instance().GetGameServerPort(),
		CConfig::Instance().GetGameServerID()
	))
	{
		RunStateError("添加逻辑服务器失败!");
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

void CGameConnect::Destroy()
{
	CConnectMgr::Destroy();
}

void CGameConnect::ServerRegisterSucc(int id, const char *ip, int port)
{

}

void CGameConnect::ConnectDisconnect(connector *)
{
	CGateClientMgr::Instance().ReleaseAllClient();
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
			int nClienNum = abs(tl->id);
			if(nClienNum >0)
			{
				MessagePack *pkmain = (MessagePack *)pMsg;
				pkmain->Begin();
				int pbSize = pkmain->GetInt32();
				size_t size = 0;
				MessagePack *pk = (MessagePack *)pkmain->GetBlockRef(pbSize, &size);
				int nClientID = 0;
				for (int i = 0; i < nClienNum; ++i)
				{
					nClientID = pkmain->GetInt32();
					CGateClientMgr::Instance().SendMsg(nClientID, pk);
				}
			}
		}
		// 转发给client
		CGateClientMgr::Instance().SendMsg(tl->id, pMsg);
		
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
		case CLIENT_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			}
			break;
		}
		}
	}
}