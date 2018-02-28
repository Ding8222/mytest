#include <map>
#include "ServerMsg.pb.h"

#include "LogServerMgr.h"
#include "serverinfo.h"
#include "Config.h"
#include "serverlog.h"
#include "GlobalDefine.h"
#include "sqlinterface.h"

#include "MainType.h"
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
}

CLogServerMgr::~CLogServerMgr()
{

}

bool CLogServerMgr::Init()
{
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

	return CServerMgr::Init(
		CConfig::Instance().GetServerIP(),
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetListenPort(),
		CConfig::Instance().GetOverTime());
}

void CLogServerMgr::Destroy()
{
	CServerMgr::Destroy();
	m_GameList.clear();
	m_LoginList.clear();
	m_DBList.clear();
	m_GateList.clear();
	m_CenterList.clear();
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
	for (std::map<int, serverinfo*>::iterator itr = m_GateList.begin(); itr != m_GateList.end(); ++itr)
	{
		snprintf(buf, len - 1, "网关服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			itr->second->GetServerID(), itr->second->GetRecvMsgNum(), itr->second->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_GameList.begin(); itr != m_GameList.end(); ++itr)
	{
		snprintf(buf, len - 1, "逻辑服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			itr->second->GetServerID(), itr->second->GetRecvMsgNum(), itr->second->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_LoginList.begin(); itr != m_LoginList.end(); ++itr)
	{
		snprintf(buf, len - 1, "登陆服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			itr->second->GetServerID(), itr->second->GetRecvMsgNum(), itr->second->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_DBList.begin(); itr != m_DBList.end(); ++itr)
	{
		snprintf(buf, len - 1, "数据服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			itr->second->GetServerID(), itr->second->GetRecvMsgNum(), itr->second->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_CenterList.begin(); itr != m_CenterList.end(); ++itr)
	{
		snprintf(buf, len - 1, "中心服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			itr->second->GetServerID(), itr->second->GetRecvMsgNum(), itr->second->GetSendMsgNum());

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
			case SVR_SUB_SERVERLOG:
			{
				svrData::ServerLog msg;
				_CHECK_PARSE_(pMsg, msg);

				//处理收到的log
			}
			default:
				break;
			}
			break;
		}
		default:
		{
			switch (info->GetServerType())
			{
			case ServerEnum::EST_GAME:
			{
				// 来自GameSvr的消息
				ProcessGameMsg(info, pMsg, tl);
				break;
			}
			case ServerEnum::EST_LOGIN:
			{
				// 来自LoginSvr的消息
				ProcessLoginMsg(info, pMsg, tl);
				break;
			}
			case ServerEnum::EST_DB:
			{
				// 来自DBSvr的消息
				ProcessDBMsg(info, pMsg, tl);
				break;
			}
			case ServerEnum::EST_GATE:
			{
				// 来自GateSvr的消息
				ProcessGateMsg(info, pMsg, tl);
				break;
			}
			case ServerEnum::EST_CENTER:
			{
				// 来自CenterSvr的消息
				ProcessCenterMsg(info, pMsg, tl);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}
		}
	}
}

void CLogServerMgr::ProcessGameMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case 1:
	{
		break;
	}
	default:
		break;
	}
}

void CLogServerMgr::ProcessLoginMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_AUTH:
		{
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

void CLogServerMgr::ProcessDBMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_AUTH_RET:
		{
			break;
		}
		case LOGIN_SUB_PLAYER_LIST_RET:
		case LOGIN_SUB_CREATE_PLAYER_RET:
		case LOGIN_SUB_SELECT_PLAYER_RET:
		{
			CLogServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_GATE, tl->id);
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

void CLogServerMgr::ProcessGateMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_PLAYER_LIST:
		case LOGIN_SUB_CREATE_PLAYER:
		case LOGIN_SUB_SELECT_PLAYER:
		{
			CLogServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, tl->id);
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
}

void CLogServerMgr::ProcessCenterMsg(serverinfo *info, Msg *pMsg, msgtail *tl)
{
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_PLAYER_LIST:
		case LOGIN_SUB_CREATE_PLAYER:
		case LOGIN_SUB_SELECT_PLAYER:
		{
			CLogServerMgr::Instance().SendMsgToServer(*pMsg, ServerEnum::EST_DB, tl->id);
			break;
		}
		default:
			break;
		}
		break;
	}
	default:
		break;
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