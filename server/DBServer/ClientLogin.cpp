#include "connector.h"
#include "sqlinterface.h"
#include "Guid.h"
#include "Timer.h"
#include "GlobalDefine.h"
#include "fmt/ostream.h"
#include "msgbase.h"
#include "ClientLogin.h"
#include "DBCenterConnect.h"
#include "task.h"

#include "MainType.h"
#include "ServerType.h"
#include "LoginType.h"
#include "Login.pb.h"
#include "ServerMsg.pb.h"

CClientLogin::CClientLogin()
{
}

CClientLogin::~CClientLogin()
{
}

void CClientLogin::Destroy()
{

}

void CClientLogin::ProcessLoginMsg(task *tk, Msg *pMsg)
{
	switch (pMsg->GetSubType())
	{
	case LOGIN_SUB_AUTH:
	{
		ClientAuth(tk, pMsg);
		break;
	}
	case LOGIN_SUB_PLAYER_LIST:
	{
		GetPlayerList(tk, pMsg);
		break;
	}
	case LOGIN_SUB_CREATE_PLAYER:
	{
		CreatePlayer(tk, pMsg);
		break;
	}
	case LOGIN_SUB_SELECT_PLAYER:
	{
		SelectPlayer(tk, pMsg);
		break;
	}
	default:
		break;
	}
}

void CClientLogin::ClientAuth(task *tk, Msg *pMsg)
{
	if (!tk || !pMsg)
		return;

	DataBase::CConnection *dbhand = tk->GetDBHand();
	if (dbhand)
	{
		netData::Auth msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::AuthRet sendMsg;
		sendMsg.set_ncode(netData::AuthRet::EC_FAIL);
		DataBase::CRecordset *res = dbhand->Execute(fmt::format("select * from account where account = '{0}' limit 1", msg.setoken().c_str()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			// 存在的账号
			res = dbhand->Execute(fmt::format("update account set logintime ={0} where account = '{1}'", CTimer::GetTime(), msg.setoken().c_str()).c_str());
			if (res)
				sendMsg.set_ncode(netData::AuthRet::EC_SUCC);
		}
		else
		{
			// 不存在的账号，创建
			res = dbhand->Execute(fmt::format("insert into account (account,createtime,logintime) values ('{0}',{1},{2})", msg.setoken().c_str(), CTimer::GetTime(), CTimer::GetTime()).c_str());
			if (res)
			{
				res = dbhand->Execute("select @@IDENTITY");
				if (!res || res->IsEnd() || !res->IsOpen())
				{
					dbhand->Execute("delete from playerdate where account = ''");
				}
				else
					sendMsg.set_ncode(netData::AuthRet::EC_SUCC);
			}
		}
		sendMsg.set_nserverid(4000);

		MessagePack pk;
		pk.Pack(&sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET);
		tk->PushMsg(&pk);
	}
}

void CClientLogin::GetPlayerList(task *tk, Msg *pMsg)
{
	if (!tk || !pMsg)
		return;

	DataBase::CConnection *dbhand = tk->GetDBHand();
	if (dbhand)
	{
		netData::PlayerList msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::PlayerListRet sendMsg;

		DataBase::CRecordset *res = dbhand->Execute(fmt::format("select * from playerdate where account = '{0}'", msg.account().c_str()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			// 查询到的角色信息
			while (!res->IsEnd())
			{
				netData::PlayerLite *_pInfo = sendMsg.add_list();
				if (_pInfo)
				{
					_pInfo->set_guid(res->GetInt64("guid"));
					_pInfo->set_sname(res->GetChar("name"));
					_pInfo->set_njob(res->GetInt("job"));
					_pInfo->set_nsex(res->GetInt("sex"));
				}
				res->NextRow();
			}
		}

		MessagePack pk;
		pk.Pack(&sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_PLAYER_LIST_RET);
		tk->PushMsg(&pk);
	}
}

void CClientLogin::CreatePlayer(task *tk, Msg *pMsg)
{
	if (!tk || !pMsg)
		return;

	DataBase::CConnection *dbhand = tk->GetDBHand();
	if (dbhand)
	{
		netData::CreatePlayer msg;
		_CHECK_PARSE_(pMsg, msg);

		int64 guid = CGuid::Instance().Generate();
		netData::CreatePlayerRet sendMsg;
		sendMsg.set_ncode(netData::CreatePlayerRet::EC_FAIL);
		DataBase::CRecordset *res = dbhand->Execute(fmt::format("insert into playerdate (account,name,guid,sex,job,level,createtime,logintime,mapid,x,y,z,data) values ('{0}','{1}',{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12})",
			msg.account().c_str(), msg.sname(), guid, msg.nsex(), msg.njob(), 1, CTimer::GetTime(), CTimer::GetTime(), guid % 9 + 1, 1, 1, 1, "1").c_str());
		if (res)
		{
			res = dbhand->Execute("select @@IDENTITY");
			if (!res || res->IsEnd() || !res->IsOpen())
			{
				dbhand->Execute("delete from playerdate where guid = 0");
			}
			else
			{
				sendMsg.set_ncode(netData::CreatePlayerRet::EC_SUCC);
				netData::PlayerLite *_pInfo = sendMsg.mutable_info();
				if (_pInfo)
				{
					_pInfo->set_guid(guid);
				}
			}
		}

		MessagePack pk;
		pk.Pack(&sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER_RET);
		tk->PushMsg(&pk);
	}
}

void CClientLogin::SelectPlayer(task *tk, Msg *pMsg)
{
	if (!tk || !pMsg)
		return;

	DataBase::CConnection *dbhand = tk->GetDBHand();
	if (dbhand)
	{
		netData::SelectPlayer msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::SelectPlayerRet sendMsg;
		sendMsg.set_ncode(netData::SelectPlayerRet::EC_FAIL);
		DataBase::CRecordset *res = dbhand->Execute(fmt::format("select * from playerdate where guid = {0}", msg.guid()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			svrData::LoadPlayerData sendMsgToGame;
			sendMsgToGame.set_clientid(tk->GetClientID());
			sendMsgToGame.set_account(res->GetChar("account"));
			sendMsgToGame.set_name(res->GetChar("name"));
			sendMsgToGame.set_guid(res->GetInt64("guid"));
			sendMsgToGame.set_sex(res->GetInt("sex"));
			sendMsgToGame.set_job(res->GetInt("job"));
			sendMsgToGame.set_level(res->GetInt("level"));
			sendMsgToGame.set_createtime(res->GetInt("createtime"));
			sendMsgToGame.set_logintime(res->GetInt("logintime"));
			sendMsgToGame.set_mapid(res->GetInt("mapid"));
			sendMsgToGame.set_x(res->GetFloat("x"));
			sendMsgToGame.set_y(res->GetFloat("y"));
			sendMsgToGame.set_z(res->GetFloat("z"));
			sendMsgToGame.set_data(res->GetChar("data"));

			res = dbhand->Execute(fmt::format("update playerdate set logintime ={0} where guid = '{1}'", CTimer::GetTime(), msg.guid()).c_str());
			if (res)
			{
				sendMsg.set_ncode(netData::SelectPlayerRet::EC_SUCC);
				MessagePack pk;
				pk.Pack(&sendMsgToGame, SERVER_TYPE_MAIN, SVR_SUB_LOAD_PLAYERDATA);
				tk->PushMsg(&pk);
			}
		}
		MessagePack pk;
		pk.Pack(&sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_SELECT_PLAYER_RET);
		tk->PushMsg(&pk);
	}
}