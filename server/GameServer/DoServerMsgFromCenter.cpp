#include "DoMsgFromCenter.h"

#include "ServerType.h"

void DoServerMsgFromCenter(connector *con, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetSubType())
	{
	case SVR_SUB_PING:
	{
		con->SetRecvPingTime(g_currenttime);
		break;
	}
	}
}