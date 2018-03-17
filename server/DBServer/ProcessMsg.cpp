
#include "task.h"
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

		DataBase::CConnection *dbhand = tk->GetDBHand();
		if (dbhand)
		{
			DataBase::CRecordset *res = dbhand->Execute(
				fmt::format("update playerdate set level = {0},mapid = {1},x = {2},y = {3},z = {4},data = '{5}' where guid = {6}",
					msg.nlevel(), msg.nmapid(), msg.nx(), msg.ny(), msg.nz(), msg.data(), msg.nguid()).c_str());
			if (res)
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
		CClientLogin::Instance().ClientAuth(tk, pMsg);
		break;
	}
	case LOGIN_SUB_PLAYER_LIST:
	{
		CClientLogin::Instance().GetPlayerList(tk, pMsg);
		break;
	}
	case LOGIN_SUB_CREATE_PLAYER:
	{
		CClientLogin::Instance().CreatePlayer(tk, pMsg);
		break;
	}
	case LOGIN_SUB_SELECT_PLAYER:
	{
		CClientLogin::Instance().SelectPlayer(tk, pMsg);
		break;
	}
	default:
		break;
	}
}
