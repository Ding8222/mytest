#include "LoginCenterConnect.h"
#include "LoginClientMgr.h"
#include "connector.h"
#include "config.h"
#include "ServerLog.h"
#include "msgbase.h"
#include "GlobalDefine.h"

#include "MainType.h"
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

void CLoginCenterConnect::Destroy()
{
	CConnectMgr::Destroy();
}

void CLoginCenterConnect::ServerRegisterSucc(int id, const char *ip, int port)
{

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
			case LOGIN_SUB_AUTH_RET:
			{
				netData::AuthRet msg;
				_CHECK_PARSE_(pMsg, msg);

				if (msg.ncode() == netData::AuthRet::EC_SUCC)
				{
					CLoginClientMgr::Instance().SetClientAuthSucceed(tl->id);
				}
				CLoginClientMgr::Instance().SendMsg(tl->id, pMsg);
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