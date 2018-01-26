#include "stdfx.h"
#include "LoginCenterConnect.h"
#include "LoginClientMgr.h"
#include "connector.h"
#include "config.h"

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
	return CServerConnect::Init(CConfig::Instance().GetCenterServerIP().c_str(),
		CConfig::Instance().GetCenterServerPort(),
		CConfig::Instance().GetCenterServerID(),
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime());
}

void CLoginCenterConnect::ConnectDisconnect()
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
		case DBSVR_SUB_EXECUTE_RET:
		{
			switch (pMsg->GetSubType())
			{
			case DBSVR_SUB_EXECUTE:
			{
				SvrData::ExecuteRet msg;
				_CHECK_PARSE_(pMsg, msg);


				netData::AuthRet sendMsg;
				sendMsg.set_ncode(netData::ChallengeRet::EC_SUCC);
				//CLoginClientMgr::Instance().SendMsg(msg.clientid(), sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET);
				break;
			}
			default:
			{
			}
			}
		}
		default:
		{
		}
		}
	}
}