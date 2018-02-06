#include "stdfx.h"
#include "DBCenterConnect.h"
#include "connector.h"
#include "config.h"
#include "sqlinterface.h"

#include "DBSvrType.h"
#include "LoginType.h"

#include "DBServer.pb.h"
#include "Login.pb.h"

static DataBase::CConnection g_dbhand;
extern int64 g_currenttime;

CDBCenterConnect::CDBCenterConnect()
{

}

CDBCenterConnect::~CDBCenterConnect()
{

}

bool CDBCenterConnect::Init()
{
	g_dbhand.SetLogDirectory("dbserver_log/dbhand_log");
	if (!g_dbhand.Open(CConfig::Instance().GetDBName().c_str(),
		CConfig::Instance().GetDBUser().c_str(),
		CConfig::Instance().GetDBPass().c_str(),
		CConfig::Instance().GetDBIP().c_str()))
	{
		log_error("连接Mysql失败!");
		return false;
	}

	if (!g_dbhand.SetCharacterSet("utf8"))
	{
		log_error("设置UTF-8失败!");
		return false;
	}

	if (!CConnectMgr::AddNewConnect(
		CConfig::Instance().GetCenterServerIP().c_str(),
		CConfig::Instance().GetCenterServerPort(),
		CConfig::Instance().GetCenterServerID()
		))
	{
		log_error("添加中心服务器失败!");
		return false;
	}

	return CConnectMgr::Init(
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime()
	);
}

void CDBCenterConnect::ServerRegisterSucc(int id, const char *ip, int port)
{

}

void CDBCenterConnect::ConnectDisconnect(connector *)
{
	
}

void CDBCenterConnect::ProcessMsg(connector *_con)
{
	Msg *pMsg = NULL;
	for (;;)
	{
		pMsg = _con->GetMsg();
		if (!pMsg)
			break;

		msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
		pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

		switch (pMsg->GetMainType())
		{
		case SERVER_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				_con->SetRecvPingTime(g_currenttime);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}
		default:
		{
			ProcessServerMsg(_con, pMsg, tl);
			break;
		}
		}
	}
}

void CDBCenterConnect::ProcessServerMsg(connector *_con, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		ProcessLoginMsg(_con, pMsg, tl);
		break;
	}
	default:
	{
		break;
	}
	}
}

void CDBCenterConnect::ProcessLoginMsg(connector *_con, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetSubType())
	{
	case LOGIN_SUB_AUTH:
	{
		netData::Auth msg;
		_CHECK_PARSE_(pMsg, msg);

		DataBase::CRecordset *res = g_dbhand.Execute(fmt::format("select * from account where uid = '{0}' limit 1", msg.setoken().c_str()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			// 存在的账号
			res = g_dbhand.Execute(fmt::format("update account set logintime ={0} where uid = '{1}'", time(nullptr), msg.setoken().c_str()).c_str());
		}
		else
		{
			// 不存在的账号
			res = g_dbhand.Execute(fmt::format("insert into account (uid,createtime,logintime) values ('{0}',{1},{2})", msg.setoken().c_str(), time(nullptr), time(nullptr)).c_str());
		}

		netData::AuthRet sendMsg;
		sendMsg.set_ncode(netData::AuthRet::EC_SUCC);
		sendMsg.set_nserverid(4000);
		SendMsgToServer(_con, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, tl->id);
		break;
	}
	case LOGIN_SUB_PLAYER_LIST:
	{
		netData::PlayerList msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::PlayerListRet sendMsg;

		DataBase::CRecordset *res = g_dbhand.Execute(fmt::format("select * from playerdate where uid = '{0}'", msg.account().c_str()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			// 查询到的角色信息
			while (!res->IsEnd())
			{
				netData::PlayerLite *_pInfo = sendMsg.add_list();
				if (_pInfo)
				{
					_pInfo->set_uuid(res->GetInt64("uuid"));
					_pInfo->set_sname(res->GetChar("name"));
					_pInfo->set_njob(res->GetInt("job"));
					_pInfo->set_nsex(res->GetInt("sex"));
				}
				res->NextRow();
			}
		}

		SendMsgToServer(_con, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_PLAYER_LIST_RET, tl->id);
		break;
	}
	case LOGIN_SUB_CREATE_PLAYER:
	{
		netData::CreatePlayer msg;
		_CHECK_PARSE_(pMsg, msg);

		int64 uuid = 123456;

		netData::CreatePlayerRet sendMsg;
		DataBase::CRecordset *res = g_dbhand.Execute(fmt::format("insert into playerdate (uid,name,uuid,sex,job,level,createtime,logintime,mapid) values ('{0}','{1}',{2},{3},{4},{5},{6},{7},{8})", 
			msg.account().c_str(), msg.sname(),uuid, msg.nsex(), msg.njob(), 1, time(nullptr), time(nullptr),1).c_str());
		if (res)
		{
			sendMsg.set_ncode(netData::CreatePlayerRet::EC_SUCC);
			netData::PlayerLite *_pInfo = sendMsg.mutable_info();
			if (_pInfo)
			{
				_pInfo->set_uuid(uuid);
			}
		}
		else
			sendMsg.set_ncode(netData::CreatePlayerRet::EC_FAIL);
		SendMsgToServer(_con, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER_RET, tl->id);
		break;
	}
	case LOGIN_SUB_SELECT_PLAYER:
	{
		netData::SelectPlayer msg;
		_CHECK_PARSE_(pMsg, msg);

		netData::SelectPlayerRet sendMsg;
		DataBase::CRecordset *res = g_dbhand.Execute(fmt::format("select * from playerdate where uuid = {0}", msg.uuid()).c_str());
		if (res && res->IsOpen() && !res->IsEnd())
		{
			g_dbhand.Execute(fmt::format("update playerdate set logintime ={0} where uuid = '{1}'", time(nullptr), msg.uuid()).c_str());
			sendMsg.set_ncode(netData::SelectPlayerRet::EC_SUCC);
		}
		else
			sendMsg.set_ncode(netData::SelectPlayerRet::EC_FAIL);
		SendMsgToServer(_con, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_SELECT_PLAYER_RET, tl->id);
		break;
	}
	default:
		break;
	}
}