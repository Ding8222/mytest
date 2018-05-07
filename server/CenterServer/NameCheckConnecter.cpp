#include "GlobalDefine.h"
#include "connector.h"
#include "msgbase.h"
#include "NameCheckConnecter.h"
#include "CentServerMgr.h"
#include "ServerLog.h"
#include "ClientAuthMgr.h"

#include "LoginType.h"
#include "ServerType.h"
#include "Login.pb.h"

extern int64 g_currenttime;

CNameCheckConnecter::CNameCheckConnecter()
{
	isReady = false;
	LogServerID = 0;
}

CNameCheckConnecter::~CNameCheckConnecter()
{
	Destroy();
}

bool CNameCheckConnecter::Init(const char *ip, int port, int id, const char *name,
	int serverid, int servertype, const char *servername, int pingtime, int overtime)
{
	if (!CConnectMgr::AddNewConnect(ip, port, id, name))
	{
		RunStateError("添加NameCheck服务器失败!");
		return false;
	}

	return CConnectMgr::Init(serverid, servertype, servername, pingtime, overtime);
}

void CNameCheckConnecter::Destroy()
{
	CConnectMgr::Destroy();
}

void CNameCheckConnecter::ServerRegisterSucc(connector * con)
{
	isReady = true;
	LogServerID = con->GetConnectID();
}

void CNameCheckConnecter::ConnectDisconnect(connector *)
{
	isReady = false;
	LogServerID = 0;
}

void CNameCheckConnecter::ProcessMsg(connector *_con)
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
		case LOGIN_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case LOGIN_SUB_CREATE_PLAYER:
			{
				netData::CreatePlayer msg;
				_CHECK_PARSE_(pMsg, msg);

				if (msg.nnamecheckret() == netData::CreatePlayer::EC_SUCC)
				{
					// 名称检查成功，转发至DB创建角色
					CentServerMgr.SendMsgToServer(*pMsg, ServerEnum::EST_DB, tl->id);
				}
				else
				{
					// 名称检查失败
					netData::CreatePlayerRet SendMsg;
					SendMsg.set_ncode(msg.nnamecheckret());
					CentServerMgr.SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER_RET, ServerEnum::EST_LOGIN, tl->id);
					ClientAuthMgr.SetPlayerOffline(msg.account());
				}
				break;
			}
			}
			break;
		}
		}
	}
}

bool CNameCheckConnecter::SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int64 nClientID)
{
	if (isReady)
	{
		return CConnectMgr::SendMsgToServer(LogServerID, pMsg, maintype, subtype, nClientID);
	}
	return false;
}

bool CNameCheckConnecter::SendMsgToServer(Msg &pMsg, int64 nClientID)
{
	if (isReady)
	{
		return CConnectMgr::SendMsgToServer(LogServerID, pMsg, nClientID);
	}
	return false;
}
