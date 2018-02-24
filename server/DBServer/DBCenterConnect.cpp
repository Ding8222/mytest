#include "stdfx.h"
#include "DBCenterConnect.h"
#include "connector.h"
#include "config.h"
#include "sqlinterface.h"
#include "ClientLogin.h"
#include "serverlog.h"

#include "DBSvrType.h"

DataBase::CConnection g_dbhand;
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
		RunStateError("连接Mysql失败!");
		return false;
	}

	if (!g_dbhand.SetCharacterSet("utf8"))
	{
		RunStateError("设置UTF-8失败!");
		return false;
	}

	if (!CConnectMgr::AddNewConnect(
		CConfig::Instance().GetCenterServerIP().c_str(),
		CConfig::Instance().GetCenterServerPort(),
		CConfig::Instance().GetCenterServerID()
		))
	{
		RunStateError("添加中心服务器失败!");
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
		CClientLogin::Instance().ProcessLoginMsg(_con, pMsg, tl);
		break;
	}
	default:
	{
		break;
	}
	}
}
