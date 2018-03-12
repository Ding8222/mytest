#include "ProcessDBMsg.h"
#include "ClientAuthMgr.h"
#include "CentServerMgr.h"
#include "GlobalDefine.h"
#include "ServerStatusMgr.h"

#include "MainType.h"
#include "LoginType.h"
#include "ServerType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"

extern int64 g_currenttime;

void ProcessDBMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
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
		case SVR_SUB_LOAD_PLAYERDATA:
		{
			svrData::LoadPlayerData msg;
			_CHECK_PARSE_(pMsg, msg);

			netData::SelectPlayerRet SendMsg;
			ClientAuthInfo *_pAuthInfo = CClientAuthMgr::Instance().FindClientAuthInfo(tl->id);
			if (_pAuthInfo)
			{
				ServerStatusInfo *_pInfo = CServerStatusMgr::Instance().GetGateInfoByMapID(msg.mapid());
				if (_pInfo)
				{
					// 返回loginsvr，通知client选角成功，登陆gamegateway
					SendMsg.set_ncode(netData::SelectPlayerRet::EC_SUCC);
					SendMsg.set_nserverid(_pInfo->nServerID);
					SendMsg.set_sip(_pInfo->chIP);
					SendMsg.set_nport(_pInfo->nPort);
					SendMsg.set_nmapid(msg.mapid());

					// 将角色数据和认证信息发送到gamegateway
					svrData::ClientToken sendMsg;
					sendMsg.mutable_data()->MergeFrom(msg);
					sendMsg.set_setoken(_pAuthInfo->Token);
					sendMsg.set_ssecret(_pAuthInfo->Secret);
					CCentServerMgr::Instance().SendMsgToServer(sendMsg, SERVER_TYPE_MAIN, SVR_SUB_CLIENT_TOKEN, ServerEnum::EST_GATE, 0, _pInfo->nServerID);
				}
				else
					SendMsg.set_ncode(netData::SelectPlayerRet::EC_SERVER);
			}
			else
				SendMsg.set_ncode(netData::SelectPlayerRet::EC_AUTH);

			CCentServerMgr::Instance().SendMsgToServer(SendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_SELECT_PLAYER_RET, ServerEnum::EST_LOGIN, tl->id);
			break;
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
			msg.set_ncode(netData::AuthRet::EC_AUTHINFO);

			ClientAuthInfo *clientinfo = CClientAuthMgr::Instance().FindClientAuthInfo(tl->id);
			if (clientinfo)
			{
				msg.set_stoken(clientinfo->Token);
				msg.set_ncode(netData::AuthRet::EC_SUCC);
			}

			CCentServerMgr::Instance().SendMsgToServer(msg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, ServerEnum::EST_LOGIN, tl->id);

			break;
		}
		case LOGIN_SUB_PLAYER_LIST_RET:
		case LOGIN_SUB_CREATE_PLAYER_RET:
		case LOGIN_SUB_SELECT_PLAYER_RET:
		{
			CCentServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_LOGIN, tl->id);
			break;
		}
		}
		break;
	}
	}
}
