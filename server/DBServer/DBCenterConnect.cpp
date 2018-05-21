#include "connector.h"
#include "config.h"
#include "sqlinterface.h"
#include "serverlog.h"
#include "msgbase.h"
#include "task.h"
#include "dotask.h"
#include "datahand.h"
#include "DBCenterConnect.h"
#include "DBCache.h"

#include "ServerType.h"
#include "DBSvrType.h"

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

				DBCenterConnect.SendMsgToServer(d->GetServerID(), *pMsg, d->GetClientID());
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

	if (!DBCache.Init(Config.GetDBName(),
		Config.GetDBUser(),
		Config.GetDBPass(),
		Config.GetDBIP(),
		Config.GetIsOpenSQLLog()))
	{
		RunStateError("加载数据库缓存失败!");
		return false;
	}

	if (!CConnectMgr::AddNewConnect(
		Config.GetCenterServerIP(),
		Config.GetCenterServerPort(),
		Config.GetCenterServerID(),
		Config.GetCenterServerName()
		))
	{
		RunStateError("添加中心服务器失败!");
		return false;
	}

	return CConnectMgr::Init(
		Config.GetServerID(),
		Config.GetServerType(),
		Config.GetServerName(),
		Config.GetPingTime(),
		Config.GetOverTime()
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

	DBCache.Destroy();
}

void CDBCenterConnect::GetDataHandInfo(char *buf, size_t buflen)
{
	if(m_Hand)
		m_Hand->GetCurrentInfo(buf, buflen);
	else
	{
		snprintf(buf, buflen - 1, "datahand为空\n");
	}
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
				AddNewTask(pMsg, _con->GetConnectID(), task::tasktype_process);
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

	if (!tk->PushMsg(pMsg))
	{
		RunStateError("添加消息至Task失败!");
		task_release(tk);
		return;
	}

	tk->SetInfo(&DBCache, serverid, tl->id);
	tk->SetSendToAll(sendtoall);
	tk->SetTaskType(tasktype);

	if (!m_Hand->PushTask(tk))
	{
		RunStateError("添加任务至datahand失败!");
		task_release(tk);
		return;
	}
}
