#include "ProcessGateMsg.h"
#include "ServerStatusMgr.h"
#include "ClientAuthMgr.h"
#include "ClientSvrMgr.h"
#include "CentServerMgr.h"

#include "MainType.h"
#include "ServerType.h"
#include "ServerMsg.pb.h"

extern int64 g_currenttime;

void ProcessGateMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
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
		case SVR_SUB_NEW_CLIENT:
		{
			svrData::AddNewClient msg;
			_CHECK_PARSE_(pMsg, msg);

			int32 id = CClientSvrMgr::Instance().AddClientSvr(tl->id, msg.ngameid(), info->GetServerID());
			if (id > 0)
			{
				CClientAuthMgr::Instance().SetCenterClientID(msg.account(), id);
			}
			svrData::AddNewClientRet SendMsg;
			SendMsg.set_ncenterclientid(id);
			CCentServerMgr::Instance().SendMsgToServer(SendMsg, SERVER_TYPE_MAIN, SVR_SUB_NEW_CLIENT_RET, ServerEnum::EST_GATE, id);
			break;
		}
		}
		break;
	}
	}
}