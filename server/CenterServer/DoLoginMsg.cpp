#include "GlobalDefine.h"
#include "DoMsg.h"
#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "ServerLog.h"
#include "NameCheckConnecter.h"

#include "LoginType.h"
#include "Login.pb.h"

void DoLoginMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetSubType())
	{
	case LOGIN_SUB_AUTH:
	{
		ClientAuthMgr.QueryAuth(pMsg, static_cast<int32>(tl->id), info->GetServerID());
		break;
	}
	case LOGIN_SUB_AUTH_RET:
	{
		netData::AuthRet msg;
		_CHECK_PARSE_(pMsg, msg);
		msg.set_ncode(netData::AuthRet::EC_AUTHINFO);

		ClientAuthInfo *clientinfo = ClientAuthMgr.FindClientAuthInfo(static_cast<int32>(tl->id));
		if (clientinfo)
		{
			msg.set_account(clientinfo->Account);
			msg.set_ncode(netData::AuthRet::EC_SUCC);
			CentServerMgr.SendMsgToServer(msg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, ServerEnum::EST_LOGIN, tl->id);
		}
		else
		{
			RunStateError("处理DB认证返回失败！没有找到账号%s信息！clientid：%d", msg.account().c_str(), tl->id);
		}
		break;
	}
	case LOGIN_SUB_CREATE_PLAYER:
	{
		if (!NameCheckConnecter.SendMsgToServer(*pMsg, tl->id))
		{
			netData::CreatePlayer msg;
			_CHECK_PARSE_(pMsg, msg);

			netData::CreatePlayerRet SendMsg;
			SendMsg.set_ncode(netData::CreatePlayerRet::EC_NAMESVR);
			CentServerMgr.SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER_RET, ServerEnum::EST_LOGIN, tl->id);
		}
		break;
	}
	case LOGIN_SUB_PLAYER_LIST:
	case LOGIN_SUB_SELECT_PLAYER:
	{
		CentServerMgr.SendMsgToServer(*pMsg, ServerEnum::EST_DB, tl->id);
		break;
	}
	case LOGIN_SUB_PLAYER_LIST_RET:
	case LOGIN_SUB_CREATE_PLAYER_RET:
	case LOGIN_SUB_SELECT_PLAYER_RET:
	{
		CentServerMgr.SendMsgToServer(*pMsg, ServerEnum::EST_LOGIN, tl->id);
		break;
	}
	}
}