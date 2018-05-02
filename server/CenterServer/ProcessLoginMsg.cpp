#include "GlobalDefine.h"
#include "ProcessServerMsg.h"
#include "ClientAuthMgr.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

extern int64 g_currenttime;

void ProcessLoginMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case SERVER_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case SVR_SUB_DEL_CLIENT:
		{
			// client断开
			svrData::DelClient msg;
			_CHECK_PARSE_(pMsg, msg);

			CClientAuthMgr::Instance().DelClientAuthInfo(tl->id);
			if(msg.offline())
				CClientAuthMgr::Instance().SetPlayerOffline(msg.account());
			break;
		}
		default:
			DoServerMsg(info, pMsg, tl);
		}
		break;
	}
	case LOGIN_TYPE_MAIN:
	{
	default:
		DoLoginMsg(info, pMsg, tl);
	}
	}
}