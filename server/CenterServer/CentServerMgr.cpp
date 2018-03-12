#include "CentServerMgr.h"
#include "serverinfo.h"
#include "Config.h"
#include "ClientAuthMgr.h"
#include "ServerStatusMgr.h"
#include "ClientSvrMgr.h"
#include "serverlog.h"
#include "ProcessGameMsg.h"
#include "ProcessLoginMsg.h"
#include "ProcessGateMsg.h"
#include "ProcessDBMsg.h"

extern int64 g_currenttime;

CCentServerMgr::CCentServerMgr()
{
	m_GameList.clear();
	m_LoginList.clear();
	m_DBList.clear();
	m_GateList.clear();
}

CCentServerMgr::~CCentServerMgr()
{

}

bool CCentServerMgr::Init(const char *ip, int serverid, int port, int overtime)
{
	if (!CClientSvrMgr::Instance().Init())
	{
		RunStateError("初始化 ClientSvrMgr 失败!");
		return false;
	}

	if (!CClientAuthMgr::Instance().Init())
	{
		RunStateError("初始化 ClientAuthMgr 失败!");
		return false;
	}

	return CServerMgr::Init(ip, serverid, port, overtime);
}

void CCentServerMgr::Run()
{
	CClientAuthMgr::Instance().Run();
	CClientSvrMgr::Instance().Run();
	CServerMgr::Run();
}

void CCentServerMgr::Destroy()
{
	CServerMgr::Destroy();
	m_GameList.clear();
	m_LoginList.clear();
	m_DBList.clear();
	m_GateList.clear();
	CClientSvrMgr::Instance().Destroy();
}

void CCentServerMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "当前注册的服务器信息：\n网关数量：%d\n逻辑服务器数量：%d\n登陆服务器数量：%d\n数据服务器数量：%d\n",
		(int)m_GateList.size(), (int)m_GameList.size(), (int)m_LoginList.size(), (int)m_DBList.size());
}

void CCentServerMgr::ResetMsgNum()
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
}

const char *CCentServerMgr::GetMsgNumInfo()
{
	static char tempbuf[1024 * 32];
	char *buf = tempbuf;
	size_t len = sizeof(tempbuf);
	int res = 0;
	serverinfo* info = nullptr;
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

	tempbuf[sizeof(tempbuf) - len] = 0;
	tempbuf[sizeof(tempbuf) - 1] = 0;
	return tempbuf;
}

void CCentServerMgr::SendMsgToServer(Msg &pMsg, int nType, int32 nClientID, int nServerID, bool bBroad)
{
	msgtail tail;
	std::map<int, serverinfo *> *iterList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_GATE:
	{
		if (!bBroad && nServerID == 0)
		{
			ClientSvr *_pData = CClientSvrMgr::Instance().GetClientSvr(nClientID);
			if (_pData)
			{
				tail.id = _pData->nClientID;
				nServerID = _pData->nGateID;
			}
			assert(nServerID);
			assert(tail.id);
		}
		iterList = &m_GateList;
		break;
	}
	case ServerEnum::EST_GAME:
	{
		if (!bBroad && nServerID == 0)
		{
			ClientSvr *_pData = CClientSvrMgr::Instance().GetClientSvr(nClientID);
			if (_pData)
			{
				tail.id = _pData->nClientID;
				nServerID = _pData->nGameServerID;
			}
			assert(nServerID);
			assert(tail.id);
		}
		tail.id = nClientID;
		iterList = &m_GameList;
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		if (!bBroad && nServerID == 0)
		{
			ClientAuthInfo *_pAuthInfo = CClientAuthMgr::Instance().FindClientAuthInfo(nClientID);
			if (_pAuthInfo)
			{
				tail.id = _pAuthInfo->nClientID;
				nServerID = _pAuthInfo->nLoginSvrID;
			}
			assert(nServerID);
			assert(tail.id);
		}
		iterList = &m_LoginList;
		break;
	}
	case ServerEnum::EST_DB:
	{
		tail.id = nClientID;
		iterList = &m_DBList;
		bBroad = true;
		break;
	}
	default:
		RunStateError("请求发送消息到未知类型的服务器，服务器类型:[%d]", nType);
		break;
	}

	if (iterList)
	{
		if (!bBroad)
		{
			std::map<int, serverinfo *>::iterator iterFind = iterList->find(nServerID);
			if (iterFind != iterList->end())
			{
				SendMsg(iterFind->second, pMsg, &tail, sizeof(tail));
			}
			else
				RunStateError("请求发送消息到未知的服务器,服务器ID:[%d]", nServerID);
		}
		else
		{
			tail.id = nClientID;
			for (std::map<int, serverinfo *>::iterator itr = iterList->begin(); itr != iterList->end(); ++itr)
			{
				SendMsg(itr->second, pMsg, &tail, sizeof(tail));
			}
		}
	}
}

void CCentServerMgr::SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int nType, int32 nClientID, int nServerID, bool bBroad)
{
	msgtail tail;
	tail.id = 0;
	std::map<int, serverinfo *> *iterList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_GATE:
	{
		if (!bBroad && nServerID == 0)
		{
			ClientSvr *_pData = CClientSvrMgr::Instance().GetClientSvr(nClientID);
			if (_pData)
			{
				tail.id = _pData->nClientID;
				nServerID = _pData->nGateID;
			}
			assert(nServerID);
			assert(tail.id);
		}
		iterList = &m_GateList;
		break;
	}
	case ServerEnum::EST_GAME:
	{
		if (!bBroad && nServerID == 0)
		{
			ClientSvr *_pData = CClientSvrMgr::Instance().GetClientSvr(nClientID);
			if (_pData)
			{
				tail.id = _pData->nClientID;
				nServerID = _pData->nGameServerID;
			}
			assert(nServerID);
			assert(tail.id);
		}
		iterList = &m_GameList;
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		if (!bBroad && nServerID == 0)
		{
			ClientAuthInfo *_pAuthInfo = CClientAuthMgr::Instance().FindClientAuthInfo(nClientID);
			if (_pAuthInfo)
			{
				tail.id = _pAuthInfo->nClientID;
				nServerID = _pAuthInfo->nLoginSvrID;
			}
			assert(nServerID);
			assert(tail.id);
		}
		iterList = &m_LoginList;
		break;
	}
	case ServerEnum::EST_DB:
	{
		tail.id = nClientID;
		iterList = &m_DBList;
		bBroad = true;
		break;
	}
	default:
		RunStateError("请求发送消息到未知类型的服务器，服务器类型:[%d]", nType);
		break;
	}

	if (iterList)
	{
		if (!bBroad)
		{
			std::map<int, serverinfo *>::iterator iterFind = iterList->find(nServerID);
			if (iterFind != iterList->end())
			{
				SendMsg(iterFind->second, pMsg, maintype, subtype, &tail, sizeof(tail));
			}
			else
				RunStateError("请求发送消息到未知的服务器，服务器ID:[%d]", nServerID);
		}
		else
		{
			for (std::map<int, serverinfo *>::iterator itr = iterList->begin(); itr != iterList->end(); ++itr)
			{
				SendMsg(itr->second, pMsg, maintype, subtype, &tail, sizeof(tail));
			}
		}
	}
}

void CCentServerMgr::OnConnectDisconnect(serverinfo *info, bool overtime)
{
	switch (info->GetServerType())
	{
	case ServerEnum::EST_GATE:
	{
		CServerStatusMgr::Instance().DelServerID(info->GetServerID());
		m_GateList.erase(info->GetServerID());
		if (overtime)
			RunStateError("网关服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("网关服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		break;
	}
	case ServerEnum::EST_GAME:
	{
		CServerStatusMgr::Instance().DelServerID(info->GetServerID());
		m_GameList.erase(info->GetServerID());
		if (overtime)
			RunStateError("逻辑服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("逻辑服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		CClientAuthMgr::Instance().Destroy(true);
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
	default:
	{
		if (overtime)
			RunStateError("未注册的服务器超时移除, ip:[%s]", info->GetIP());
		else
			RunStateError("未注册的服务器关闭移除, ip:[%s]", info->GetIP());
	}
	}
}

void CCentServerMgr::ProcessMsg(serverinfo *info)
{
	Msg *pMsg;
	for (;;)
	{
		pMsg = info->GetMsg();
		if (!pMsg)
			return;

		msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
		pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

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
		}
		break;
	}
}

bool CCentServerMgr::AddNewServer(serverinfo *info, int nServerID, int nType)
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

serverinfo *CCentServerMgr::FindServer(int nServerID, int nType)
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
	default:
		return nullptr;
	}
	std::map<int, serverinfo*>::iterator itr = _pList->find(nServerID);
	if (itr != _pList->end())
		return itr->second;
	return nullptr;
}

void CCentServerMgr::ServerRegisterSucc(int id, int type, const char *ip, int port)
{

}