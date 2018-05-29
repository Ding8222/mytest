#include "LoginCenterConnect.h"
#include "LoginClientMgr.h"
#include "connector.h"
#include "config.h"
#include "ServerLog.h"
#include "msgbase.h"
#include "GlobalDefine.h"
#include "ClientAuth.h"

#include "ServerType.h"
#include "DBSvrType.h"
#include "LoginType.h"
#include "Login.pb.h"
#include "DBServer.pb.h"

extern int64 g_currenttime;

CLoginCenterConnect::CLoginCenterConnect()
{

}

CLoginCenterConnect::~CLoginCenterConnect()
{

}

bool CLoginCenterConnect::Init()
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

void CLoginCenterConnect::Destroy()
{
	CConnectMgr::Destroy();
}

void CLoginCenterConnect::ConnectDisconnect(connector *)
{

}

void CLoginCenterConnect::ProcessMsg(connector *_con)
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
			default:
			{
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
			{
				LoginClientMgr.SendMsg(static_cast<int32>(tl->id), pMsg);
				break;
			}
			case LOGIN_SUB_SELECT_PLAYER_RET:
			{
				netData::SelectPlayerRet msg;
				_CHECK_PARSE_(pMsg, msg);

				if (msg.ncode() == netData::SelectPlayerRet::EC_SUCC)
				{
					ClientAuth.SetSelectPlayerSucc(static_cast<int32>(tl->id));
				}
				LoginClientMgr.SendMsg(static_cast<int32>(tl->id), pMsg);
				break;
			}
			case LOGIN_SUB_AUTH_RET:
			{
				netData::AuthRet msg;
				_CHECK_PARSE_(pMsg, msg);

				if (msg.ncode() == netData::AuthRet::EC_SUCC)
				{
					if (ClientAuth.AddAccount(static_cast<int32>(tl->id), msg.account()))
						LoginClientMgr.SetClientAuthSucceed(static_cast<int32>(tl->id));
					else
						msg.set_ncode(netData::AuthRet::EC_ADDACCOUNT);
				}
				LoginClientMgr.SendMsg(static_cast<int32>(tl->id), pMsg);
				break;
			}
			}
			break;
		}
		}
	}
}