#include "connector.h"
#include "config.h"
#include "sqlinterface.h"
#include "serverlog.h"
#include "msgbase.h"
#include "task.h"
#include "dotask.h"
#include "datahand.h"
#include "DBCenterConnect.h"

#include "MainType.h"
#include "ServerType.h"
#include "DBSvrType.h"

DataBase::CConnection g_dbhand;
extern int64 g_currenttime;

CDBCenterConnect::CDBCenterConnect()
{
	m_Hand = nullptr;
}

CDBCenterConnect::~CDBCenterConnect()
{
	Destroy();
}

//执行逻辑任务
static bool DoTask(void *tk)
{
	OnDoTask(tk);
	return false;
}

//对执行逻辑任务的结果的处理
static void DoTaskResult(void *taskresult, freetask ffunc)
{
	task *d = (task *)taskresult;

	Msg *pMsg;

	//需要回馈时，才发送
	if (d->IsNeedSend())
	{
		if (d->IsSendToAll())
		{
			for (;;)
			{
				pMsg = d->GetMsg();
				if (!pMsg)
					break;
			}
		}
		else
		{
			for (;;)
			{
				pMsg = d->GetMsg();
				if (!pMsg)
					break;

				CDBCenterConnect::Instance().SendMsgToServer(d->GetServerID(), *pMsg, d->GetClientID());
			}
		}
	}
	ffunc(taskresult);
}

bool CDBCenterConnect::Init()
{
	if (!task::InitPools())
	{
		RunStateError("初始化task pool失败!");
		return false;
	}

	m_Hand = datahand_create();
	if (!m_Hand)
	{
		RunStateError("创建DataHand失败!");
		return false;
	}

	if (!m_Hand->Init(20, nullptr, task_release, DoTask, DoTaskResult, nullptr))
	{
		RunStateError("初始化DataHand失败!");
		return false;
	}

	g_dbhand.SetLogDirectory("log_log/DBServer_Log/dbhand_log");
	g_dbhand.SetEnableLog(CConfig::Instance().GetIsOpenSQLLog());
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

void CDBCenterConnect::Run()
{
	m_Hand->RunOnce();
	CConnectMgr::Run();
}

void CDBCenterConnect::Destroy()
{
	CConnectMgr::Destroy();

	if (m_Hand)
	{
		m_Hand->Destroy();
		datahand_release(m_Hand);
		m_Hand = nullptr;
	}

	task::DestroyPools();
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
			AddNewTask(pMsg, _con->GetConnectID(), task::tasktype_process);
			break;
		}
		}
	}
}

void CDBCenterConnect::AddNewTask(Msg *pMsg, int serverid, int tasktype, bool sendtoall)
{
	if (!pMsg)
		return;

	msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
	pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

	task *tk = task_create();
	if (!tk)
	{
		RunStateError("创建任务失败!");
		return;
	}

	tk->SetInfo(&g_dbhand, serverid, tl->id);
	tk->PushMsg(pMsg);

	tk->SetSendToAll(sendtoall);
	tk->SetTaskType(tasktype);

	if (!m_Hand->PushTask(tk))
	{
		RunStateError("添加任务至datahand失败!");
		task_release(tk);
		return;
	}
}
