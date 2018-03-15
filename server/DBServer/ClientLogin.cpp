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
#include "Config.h"
#include "ServerLog.h"

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
		sendMsg.set_account(msg.account());
		DataBase::CRecordset *res = dbhand->Execute(fmt::format("select * from account where account = '{0}' limit 1",
			msg.account()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			// 存在的账号
			res = dbhand->Execute(fmt::format("update account set logintime ={0} where account = '{1}'",
				CTimer::GetTime(), msg.account().c_str()).c_str());
			if (res)
				sendMsg.set_ncode(netData::AuthRet::EC_SUCC);
			else
			{
				RunStateError("更新账号:%s登陆时间失败！", msg.account().c_str());
				sendMsg.set_ncode(netData::AuthRet::EC_LOGINTIME);
			}
		}
		else
		{
			// 不存在的账号，创建
			res = dbhand->Execute(fmt::format("insert into account (account,createtime,logintime) values ('{0}',{1},{2})",
				msg.account(), CTimer::GetTime(), CTimer::GetTime()).c_str());
			if (res)
			{
				res = dbhand->Execute("select @@IDENTITY");
				if (!res || res->IsEnd() || !res->IsOpen())
				{
					dbhand->Execute("delete from playerdate where account = ''");
					sendMsg.set_ncode(netData::AuthRet::EC_CREATE);
					RunStateError("创建账号:%s失败！", msg.account().c_str());
				}
				else
					sendMsg.set_ncode(netData::AuthRet::EC_SUCC);
			}
		}

		MessagePack pk;
		pk.Pack(&sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET);
		tk->PushMsg(&pk);
	}
	else
		RunStateError("获取dbhand失败！");
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

		DataBase::CRecordset *res = dbhand->Execute(fmt::format("select * from playerdate where account = '{0}'", 
			msg.account()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			// 查询到的角色信息
			while (!res->IsEnd())
			{
				netData::PlayerLite *_pInfo = sendMsg.add_list();
				if (_pInfo)
				{
					_pInfo->set_nguid(res->GetInt64("guid"));
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
	else
		RunStateError("获取dbhand失败！");
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
			msg.account(), msg.sname(), guid, msg.nsex(), msg.njob(), 1, CTimer::GetTime(), CTimer::GetTime(), CConfig::Instance().GetBeginMap(), 1, 1, 1, "1").c_str());
		if (res)
		{
			res = dbhand->Execute("select @@IDENTITY");
			if (!res || res->IsEnd() || !res->IsOpen())
			{
				RunStateError("账号：%s创建角色：%s失败！", msg.account().c_str(), msg.sname().c_str());
				dbhand->Execute("delete from playerdate where guid = 0");
				sendMsg.set_ncode(netData::CreatePlayerRet::EC_CREATE);
			}
			else
			{
				sendMsg.set_ncode(netData::CreatePlayerRet::EC_SUCC);
				netData::PlayerLite *_pInfo = sendMsg.mutable_info();
				if (_pInfo)
				{
					_pInfo->set_nguid(guid);
					_pInfo->set_sname(msg.sname());
					_pInfo->set_njob(msg.njob());
					_pInfo->set_nsex(msg.nsex());
				}
			}
		}
		else
		{
			RunStateError("账号：%s创建角色：%s失败！", msg.account().c_str(), msg.sname().c_str());
			sendMsg.set_ncode(netData::CreatePlayerRet::EC_CREATE);
		}

		MessagePack pk;
		pk.Pack(&sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER_RET);
		tk->PushMsg(&pk);
	}
	else
		RunStateError("获取dbhand失败！");
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
		DataBase::CRecordset *res = dbhand->Execute(fmt::format("select * from playerdate where guid = {0}", 
			msg.nguid()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			svrData::LoadPlayerData sendMsgToGame;
			sendMsgToGame.set_account(res->GetChar("account"));
			sendMsgToGame.set_name(res->GetChar("name"));
			sendMsgToGame.set_nguid(res->GetInt64("guid"));
			sendMsgToGame.set_nsex(res->GetInt("sex"));
			sendMsgToGame.set_njob(res->GetInt("job"));
			sendMsgToGame.set_nlevel(res->GetInt("level"));
			sendMsgToGame.set_ncreatetime(res->GetInt("createtime"));
			sendMsgToGame.set_nlogintime(res->GetInt("logintime"));
			sendMsgToGame.set_nmapid(res->GetInt("mapid"));
			sendMsgToGame.set_nx(res->GetFloat("x"));
			sendMsgToGame.set_ny(res->GetFloat("y"));
			sendMsgToGame.set_nz(res->GetFloat("z"));
			sendMsgToGame.set_data(res->GetChar("data"));

			res = dbhand->Execute(fmt::format("update playerdate set logintime ={0} where guid = '{1}'", 
				CTimer::GetTime(), msg.nguid()).c_str());
			if (res)
			{
				MessagePack pk;
				pk.Pack(&sendMsgToGame, SERVER_TYPE_MAIN, SVR_SUB_PLAYERDATA);
				tk->PushMsg(&pk);
				return;
			}
			else
			{
				RunStateError("更新角色：%I64d登陆时间失败！", msg.nguid());
				sendMsg.set_ncode(netData::SelectPlayerRet::EC_LOGINTIME);
			}
		}
		else
		{
			RunStateError("查找角色：%I64d失败！", msg.nguid());
			sendMsg.set_ncode(netData::SelectPlayerRet::EC_NONE);
		}

		MessagePack pk;
		pk.Pack(&sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_SELECT_PLAYER_RET);
		tk->PushMsg(&pk);
	}
	else
		RunStateError("获取dbhand失败！");
}