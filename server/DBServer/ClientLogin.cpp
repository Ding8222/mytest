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
#include "DBCache.h"

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

void CClientLogin::ClientAuth(task *tk, Msg *pMsg)
{
	CDBCache *dbhand = tk->GetCacheDBHand();
	if (dbhand)
	{
		netData::Auth msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::AuthRet sendMsg;
		sendMsg.set_account(msg.account());

		const char *account = msg.account().c_str();
		nlohmann::json reault = dbhand->ExecuteSingle("account", account);
		if (reault.find("account") != reault.end())
		{
			// 存在账号
			nlohmann::json sql = {
				{ "account",msg.account() },
				{"logintime",fmt::format("{0}",CTimer::GetTime()) }
			};
			if(dbhand->Update("account", sql))
				sendMsg.set_ncode(netData::AuthRet::EC_SUCC);
			else
			{
				RunStateError("更新账号:%s登陆时间失败！", account);
				sendMsg.set_ncode(netData::AuthRet::EC_LOGINTIME);
			}
		}
		else
		{
			// 不存在的账号，创建
			const std::string &nowtime = fmt::format("{0}", CTimer::GetTime());
			nlohmann::json sql = {
				{ "account",msg.account() },
				{ "createtime",nowtime },
				{ "logintime",nowtime }
			};
			if (dbhand->Insert("account", sql))
				sendMsg.set_ncode(netData::AuthRet::EC_SUCC);
			else
			{
				RunStateError("创建账号:%s失败！", account);
				sendMsg.set_ncode(netData::AuthRet::EC_LOGINTIME);
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
	CDBCache *dbhand = tk->GetCacheDBHand();
	if (dbhand)
	{
		netData::PlayerList msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::PlayerListRet sendMsg;

		std::list<std::string> filed;
		filed.push_back("guid");
		filed.push_back("name");
		filed.push_back("job");
		filed.push_back("sex");

		nlohmann::json reault = dbhand->ExecuteMulti("playerdate", msg.account(), 0, &filed);
		for (nlohmann::json::iterator iter = reault.begin(); iter != reault.end(); ++iter)
		{
			netData::PlayerLite *_pInfo = sendMsg.add_list();
			if (_pInfo)
			{
				_pInfo->set_nguid(iter.value()["guid"]);
				_pInfo->set_sname(iter.value()["name"]);
				_pInfo->set_njob(iter.value()["job"]);
				_pInfo->set_nsex(iter.value()["sex"]);
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
	CDBCache *dbhand = tk->GetCacheDBHand();
	if (dbhand)
	{
		netData::CreatePlayer msg;
		_CHECK_PARSE_(pMsg, msg);

		int64 guid = CGuid::Instance().Generate();
		netData::CreatePlayerRet sendMsg;
		sendMsg.set_ncode(netData::CreatePlayerRet::EC_FAIL);

		const std::string &nowtime = fmt::format("{0}", CTimer::GetTime());
		nlohmann::json sql = {
		{ "account",fmt::format("{0}", msg.account()) },
		{ "name",fmt::format("{0}", msg.sname()) },
		{ "guid",fmt::format("{0}", guid) },
		{ "sex",fmt::format("{0}", msg.nsex()) },
		{ "job",fmt::format("{0}", msg.njob()) },
		{ "level","1" },
		{ "createtime",nowtime },
		{ "logintime",nowtime },
		{ "mapid",fmt::format("{0}",Config.GetBeginMap()) },
		{ "x","1" },
		{ "y","1" },
		{ "z","1" },
		{ "data","" },
		};
		if (dbhand->Insert("playerdate", sql))
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
	CDBCache *dbhand = tk->GetCacheDBHand();
	if (dbhand)
	{
		netData::SelectPlayer msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::SelectPlayerRet sendMsg;

		nlohmann::json reault = dbhand->ExecuteSingle("playerdate", fmt::format("{0}", msg.nguid()).c_str());
		if(!reault.empty())
		{
			svrData::LoadPlayerData sendMsgToGame;
			sendMsgToGame.set_account(reault["account"]);
			sendMsgToGame.set_name(reault["name"]);
			sendMsgToGame.set_nguid(reault["guid"]);
			sendMsgToGame.set_nsex(reault["sex"]);
			sendMsgToGame.set_njob(reault["job"]);
			sendMsgToGame.set_nlevel(reault["level"]);
			sendMsgToGame.set_ncreatetime(reault["createtime"]);
			sendMsgToGame.set_nlogintime(reault["logintime"]);
			sendMsgToGame.set_nmapid(reault["mapid"]);
			sendMsgToGame.set_nx(reault["x"]);
			sendMsgToGame.set_ny(reault["y"]);
			sendMsgToGame.set_nz(reault["z"]);
			sendMsgToGame.set_data(reault["data"]);


			nlohmann::json sql = {
				{ "guid",fmt::format("{0}", msg.nguid()) },
				{ "logintime",fmt::format("{0}",CTimer::GetTime()) }
			};
			if (dbhand->Update("playerdate", sql))
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