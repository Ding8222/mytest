#include "GlobalDefine.h"
#include "ProcessServerMsg.h"
#include "ServerStatusMgr.h"

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
		case SVR_SUB_SERVER_LOADINFO:
		{
			// 添加服务器负载信息
			CServerStatusMgr::Instance().AddGateServer(info, pMsg);
			break;
		}
		case SVR_SUB_UPDATE_LOAD:
		{
			// 更新服务器负载信息
			svrData::UpdateServerLoad msg;
			_CHECK_PARSE_(pMsg, msg);

			CServerStatusMgr::Instance().UpdateGateServerLoad(info->GetServerID(), msg.nclientcountnow(), msg.nclientcountmax());
			break;
		}
		default:
			DoServerMsg(info, pMsg, tl);
		}
		break;
	}
	}
}