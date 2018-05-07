#pragma once
#include "GlobalDefine.h"
#include "CentServerMgr.h"

namespace FuncUti
{
	inline void SendMsgToGame(int64 playerguid, google::protobuf::Message &pMsg, int maintype, int subtype, int nServerID = 0, bool bBroad = false)
	{
		CentServerMgr.SendMsgToServer(pMsg, maintype, subtype, ServerEnum::EST_GAME, playerguid, nServerID, bBroad);
	}
	inline void SendMsgToGame(int64 playerguid, Msg &pMsg, int nServerID = 0, bool bBroad = false)
	{
		CentServerMgr.SendMsgToServer(pMsg, ServerEnum::EST_GAME, playerguid, nServerID, bBroad);
	}
}