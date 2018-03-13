#include "ProcessGameMsg.h"
#include "ServerStatusMgr.h"
#include "ClientSvrMgr.h"
#include "CentServerMgr.h"
#include "ClientAuthMgr.h"

#include "MainType.h"
#include "ServerType.h"
#include "LoginType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"

extern int64 g_currenttime;

void ProcessGameMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
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
		case SVR_SUB_SERVER_LOADINFO:
		{
			// 添加服务器负载信息
			CServerStatusMgr::Instance().AddNewServer(info, pMsg);
			break;
		}
		case SVR_SUB_UPDATE_LOAD:
		{
			// 更新服务器负载信息
			svrData::UpdateServerLoad msg;
			_CHECK_PARSE_(pMsg, msg);

			CServerStatusMgr::Instance().UpdateServerLoad(info->GetServerID(), msg.nclientcountnow(), msg.nclientcountmax());
			break;
		}
		case SVR_SUB_CHANGELINE:
		{
			// client换线切图
			svrData::ChangeLine msg;
			_CHECK_PARSE_(pMsg, msg);

			svrData::ChangeLineRet SendMsg;
			ServerStatusInfo *_pInfo = CServerStatusMgr::Instance().GetGateInfoByMapID(msg.nmapid(), msg.nlineid());
			if (_pInfo)
			{
				// 返回loginsvr，通知client选角成功，登陆gamegateway
				SendMsg.set_ncode(netData::SelectPlayerRet::EC_SUCC);
				SendMsg.set_nserverid(_pInfo->nServerID);
				SendMsg.set_sip(_pInfo->chIP);
				SendMsg.set_nport(_pInfo->nPort);
				SendMsg.set_nmapid(msg.nmapid());

				// 将角色数据和认证信息发送到gamegateway
				svrData::ClientToken sendMsg;
				sendMsg.mutable_data()->MergeFrom(msg.data());
				sendMsg.set_setoken(msg.setoken());
				sendMsg.set_ssecret(msg.ssecret());
				CCentServerMgr::Instance().SendMsgToServer(sendMsg, SERVER_TYPE_MAIN, SVR_SUB_CLIENT_TOKEN, ServerEnum::EST_GATE, 0, _pInfo->nServerID);
			}
			else
				SendMsg.set_ncode(netData::SelectPlayerRet::EC_SERVER);

			CCentServerMgr::Instance().SendMsgToServer(SendMsg, SERVER_TYPE_MAIN, SVR_SUB_CHANGELINE_RET, ServerEnum::EST_GAME, tl->id);
			break;
		}
		case SVR_SUB_DEL_CLIENT:
		{
			// client断开
			svrData::DelClient msg;
			_CHECK_PARSE_(pMsg, msg);

			CClientSvrMgr::Instance().DelClientSvr(tl->id);
			if (!msg.bchangeline())
			{
				CClientAuthMgr::Instance().ClientOffline(msg.account());
			}
			break;
		}
		}
		break;
	}
	}
}