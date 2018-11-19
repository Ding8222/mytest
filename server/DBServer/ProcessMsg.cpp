#include "msgbase.h"
#include "ProcessMsg.h"
#include "GlobalDefine.h"
#include "Timer.h"
#include "fmt/ostream.h"
#include "ServerLog.h"

#include "LoginType.h"
#include "ServerType.h"
#include "ClientLogin.h"

#include "ServerMsg.pb.h"

#include "DBCache.h"
#include "task.h"

void ProcessMsg(task *tk, Msg *pMsg)
{
	switch (pMsg->GetMainType())
	{
	case SERVER_TYPE_MAIN:
	{
		ProcessServerMsg(tk, pMsg);
		break;
	}
	case LOGIN_TYPE_MAIN:
	{
		ProcessLoginMsg(tk, pMsg);
		break;
	}
	default:
		break;
	}
}

void ProcessServerMsg(task *tk, Msg *pMsg)
{
	switch (pMsg->GetSubType())
	{
	case SVR_SUB_PLAYERDATA:
	{
		svrData::LoadPlayerData msg;
		_CHECK_PARSE_(pMsg, msg);

		CDBCache *dbhand = tk->GetCacheDBHand();
		if (dbhand)
		{
			nlohmann::json sql = {
				{ "guid",fmt::format("{0}", msg.nguid()) },
				{ "level",fmt::format("{0}",msg.nlevel()) },
				{ "mapid",fmt::format("{0}",msg.nmapid()) },
				{ "x",fmt::format("{0}",msg.nx()) },
				{ "y",fmt::format("{0}",msg.ny()) },
				{ "z",fmt::format("{0}",msg.nz()) },
				{ "data",msg.data() },
			};
			if (dbhand->Update("playerdate", sql))
			{
				RunStateLog("玩家数据保存成功！guid：%I64d", msg.nguid());
			}
			else
				RunStateError("玩家数据保存失败！guid：%I64d", msg.nguid());
		}
		else
			RunStateError("获取dbhand失败！");
		break;
	}
	default:
		break;
	}
}

void ProcessLoginMsg(task *tk, Msg *pMsg)
{
	switch (pMsg->GetSubType())
	{
	case LOGIN_SUB_AUTH:
	{
		ClientLogin.ClientAuth(tk, pMsg);
		break;
	}
	case LOGIN_SUB_PLAYER_LIST:
	{
		ClientLogin.GetPlayerList(tk, pMsg);
		break;
	}
	case LOGIN_SUB_CREATE_PLAYER:
	{
		ClientLogin.CreatePlayer(tk, pMsg);
		break;
	}
	case LOGIN_SUB_SELECT_PLAYER:
	{
		ClientLogin.SelectPlayer(tk, pMsg);
		break;
	}
	default:
		break;
	}
}
