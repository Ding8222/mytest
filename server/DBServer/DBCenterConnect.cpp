#include "stdfx.h"
#include "DBCenterConnect.h"
#include "connector.h"
#include "config.h"
#include "sqlinterface.h"
#include "json.hpp"

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

	return CServerConnect::Init(CConfig::Instance().GetCenterServerIP().c_str(),
		CConfig::Instance().GetCenterServerPort(),
		CConfig::Instance().GetCenterServerID(),
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime());
}

void CDBCenterConnect::ConnectDisconnect()
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
			}
			}
			break;
		}
		case DBSVR_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case DBSVR_SUB_EXECUTE:
			{
				SvrData::Execute msg;
				_CHECK_PARSE_(pMsg, msg);

				DataBase::CRecordset *res = g_dbhand.Execute(msg.sql().c_str());
				if (res && res->IsOpen())
				{
					while (!res->IsEnd())
					{
						std::string ret = res->Get("createtime");
						res->NextRow();
					}
				}


				break;
			}
			default:
			{
			}
			}
			break;
		}
		case LOGIN_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case LOGIN_SUB_AUTH:
			{
				msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
				pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));
				if (msgtail::enum_type_from_client == tl->type)
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
					SendMsgToClient(&sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, tl->id);
				}
				break;
			}
			default:
				break;
			}
		}
		default:
		{
		}
		}
	}
}