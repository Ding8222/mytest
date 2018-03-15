#include <map>
#include "ServerMsg.pb.h"

#include "serverinfo.h"
#include "Config.h"
#include "serverlog.h"
#include "GlobalDefine.h"
#include "sqlinterface.h"
#include "task.h"
#include "dotask.h"
#include "datahand.h"
#include "LogServerMgr.h"

#include "LoginType.h"
#include "ServerType.h"

DataBase::CConnection g_dbhand;
extern int64 g_currenttime;

CLogServerMgr::CLogServerMgr()
{
	m_GameList.clear();
	m_LoginList.clear();
	m_DBList.clear();
	m_GateList.clear();
	m_CenterList.clear();
	m_Hand = nullptr;
}

CLogServerMgr::~CLogServerMgr()
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
	ffunc(taskresult);
}

bool CLogServerMgr::Init(const char *ip, int serverid, int port, int overtime)
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

	if (!m_Hand->Init(10, nullptr, task_release, DoTask, DoTaskResult, nullptr))
	{
		RunStateError("初始化DataHand失败!");
		return false;
	}

	g_dbhand.SetLogDirectory("log_log/LogServer_Log/dbhand_log");
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

	return CServerMgr::Init(ip, serverid, port, overtime);
}

void CLogServerMgr::Destroy()
{
	CServerMgr::Destroy();
	m_GameList.clear();
	m_LoginList.clear();
	m_DBList.clear();
	m_GateList.clear();
	m_CenterList.clear();

	if (m_Hand)
	{
		m_Hand->Destroy();
		datahand_release(m_Hand);
		m_Hand = nullptr;
	}

	//切记放到最后
	task::DestroyPools();
}

void CLogServerMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "当前注册的服务器信息：\n网关数量：%d\n逻辑服务器数量：%d\n登陆服务器数量：%d\n数据服务器数量：%d\n中心服务器数量：%d\n",
		(int)m_GateList.size(), (int)m_GameList.size(), (int)m_LoginList.size(), (int)m_DBList.size(), (int)m_CenterList.size());
}

void CLogServerMgr::ResetMsgNum()
{
	for (std::map<int, serverinfo*>::iterator itr = m_GateList.begin(); itr != m_GateList.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}

	for (std::map<int, serverinfo*>::iterator itr = m_GameList.begin(); itr != m_GameList.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}

	for (std::map<int, serverinfo*>::iterator itr = m_LoginList.begin(); itr != m_LoginList.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}

	for (std::map<int, serverinfo*>::iterator itr = m_DBList.begin(); itr != m_DBList.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}

	for (std::map<int, serverinfo*>::iterator itr = m_CenterList.begin(); itr != m_CenterList.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}
}

const char *CLogServerMgr::GetMsgNumInfo()
{
	static char tempbuf[1024 * 32];
	char *buf = tempbuf;
	size_t len = sizeof(tempbuf);
	int res = 0;
	serverinfo *info = nullptr;
	for (std::map<int, serverinfo*>::iterator itr = m_GateList.begin(); itr != m_GateList.end(); ++itr)
	{
		info = itr->second;
		snprintf(buf, len - 1, "网关服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			info->GetServerID(), info->GetRecvMsgNum(), info->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_GameList.begin(); itr != m_GameList.end(); ++itr)
	{
		info = itr->second;
		snprintf(buf, len - 1, "逻辑服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			info->GetServerID(), info->GetRecvMsgNum(), info->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_LoginList.begin(); itr != m_LoginList.end(); ++itr)
	{
		info = itr->second;
		snprintf(buf, len - 1, "登陆服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			info->GetServerID(), info->GetRecvMsgNum(), info->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_DBList.begin(); itr != m_DBList.end(); ++itr)
	{
		info = itr->second;
		snprintf(buf, len - 1, "数据服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			info->GetServerID(), info->GetRecvMsgNum(), info->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_CenterList.begin(); itr != m_CenterList.end(); ++itr)
	{
		info = itr->second;
		snprintf(buf, len - 1, "中心服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			info->GetServerID(), info->GetRecvMsgNum(), info->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	tempbuf[sizeof(tempbuf) - len] = 0;
	tempbuf[sizeof(tempbuf) - 1] = 0;
	return tempbuf;
}

void CLogServerMgr::OnConnectDisconnect(serverinfo *info, bool overtime)
{
	switch (info->GetServerType())
	{
	case ServerEnum::EST_GATE:
	{
		m_GateList.erase(info->GetServerID());
		if (overtime)
			RunStateError("网关服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("网关服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		break;
	}
	case ServerEnum::EST_GAME:
	{
		m_GameList.erase(info->GetServerID());
		if (overtime)
			RunStateError("逻辑服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("逻辑服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		m_LoginList.erase(info->GetServerID());
		if (overtime)
			RunStateError("登陆服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("登陆服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());

		break;
	}
	case ServerEnum::EST_DB:
	{
		m_DBList.erase(info->GetServerID());
		if (overtime)
			RunStateError("数据服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("数据服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());

		break;
	}
	case ServerEnum::EST_CENTER:
	{
		m_CenterList.erase(info->GetServerID());
		if (overtime)
			RunStateError("中心服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("中心服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());

		break;
	}
	default:
	{
		if (overtime)
			RunStateError("未注册的服务器超时移除, ip:[%s]", info->GetIP());
		else
			RunStateError("未注册的服务器关闭移除, ip:[%s]", info->GetIP());
	}
	}
}

void CLogServerMgr::ProcessMsg(serverinfo *info)
{
	Msg *pMsg;
	for (;;)
	{
		pMsg = info->GetMsg();
		if (!pMsg)
			return;

		msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
		pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

		switch (pMsg->GetMainType())
		{
		case SERVER_TYPE_MAIN:
		{
			// 来自LoginSvr的消息
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				info->SendMsg(pMsg);
				info->SetPingTime(g_currenttime);
				break;
			}
			default:
				break;
			}
			break;
		}
		default:
		{
		}
		}
	}
}

bool CLogServerMgr::AddNewServer(serverinfo *info, int nServerID, int nType)
{
	if (FindServer(nServerID, nType))
	{
		RunStateError("添加服务器失败！已经存在的服务器，远程服务器ID：[%d] IP:[%s]", nServerID, info->GetIP());
		return false;
	}

	std::map<int, serverinfo*> *_pList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_GATE:
	{
		_pList = &m_GateList;
		break;
	}
	case ServerEnum::EST_GAME:
	{
		_pList = &m_GameList;
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		_pList = &m_LoginList;
		break;
	}
	case ServerEnum::EST_DB:
	{
		_pList = &m_DBList;
		break;
	}
	case ServerEnum::EST_CENTER:
	{
		_pList = &m_CenterList;
		break;
	}
	default:
	{
		RunStateError("添加服务器失败！不存在的服务器类型，远程服务器ID：[%d] 类型：[%d] IP:[%s]", nServerID, nType, info->GetIP());
		return false;
	}
	}

	info->SetAlreadyRegister();
	info->SetServerID(nServerID);
	info->SetServerType(nType);
	_pList->insert(std::make_pair(nServerID, info));
	return true;
}

serverinfo *CLogServerMgr::FindServer(int nServerID, int nType)
{
	std::map<int, serverinfo*> *_pList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_GATE:
	{
		_pList = &m_GateList;
		break;
	}
	case ServerEnum::EST_GAME:
	{
		_pList = &m_GameList;
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		_pList = &m_LoginList;
		break;
	}
	case ServerEnum::EST_DB:
	{
		_pList = &m_DBList;
		break;
	}
	case ServerEnum::EST_CENTER:
	{
		_pList = &m_CenterList;
		break;
	}
	default:
		return nullptr;
	}
	std::map<int, serverinfo*>::iterator itr = _pList->find(nServerID);
	if (itr != _pList->end())
		return itr->second;
	return nullptr;
}

void CLogServerMgr::ServerRegisterSucc(int id, int type, const char *ip, int port)
{

}


void CLogServerMgr::AddNewTask(Msg *pMsg, int serverid)
{
	if (!pMsg)
		return;
	
	task *tk = task_create();
	if (!tk)
	{
		RunStateError("创建任务失败!");
		return;
	}

	tk->SetInfo(&g_dbhand, serverid);
	tk->PushMsg(pMsg);
	
	if (!m_Hand->PushTask(tk))
	{
		RunStateError("添加任务至datahand失败!");
		task_release(tk);
		return;
	}
}
