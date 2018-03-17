#include "ProcessLoginMsg.h"
#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "NameCheckConnecter.h"

#include "LoginType.h"
#include "ServerType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"

extern int64 g_currenttime;

void ProcessLoginMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case SERVER_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case SVR_SUB_PING:
		{
			info->SendMsg(pMsg);
			info->SetPingTime(g_currenttime);
			break;
		}
		case SVR_SUB_DEL_CLIENT:
		{
			// client断开
			svrData::DelClient msg;
			_CHECK_PARSE_(pMsg, msg);

			CClientAuthMgr::Instance().DelClientAuthInfo(tl->id);
			break;
		}
		}
		break;
	}
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_AUTH:
		{
			CClientAuthMgr::Instance().QueryAuth(pMsg, tl->id, info->GetServerID());
			break;
		}
		case LOGIN_SUB_CREATE_PLAYER:
		{
			if (!CNameCheckConnecter::Instance().SendMsgToServer(*pMsg, tl->id))
			{
				netData::CreatePlayerRet SendMsg;
				SendMsg.set_ncode(netData::CreatePlayerRet::EC_NAMESVR);
				CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER_RET, ServerEnum::EST_LOGIN, tl->id);
			}
			break;
		}
		case LOGIN_SUB_PLAYER_LIST:
		case LOGIN_SUB_SELECT_PLAYER:
		{
			CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, tl->id);
			break;
		}
		}
		break;
	}
	}
}