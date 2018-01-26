#include"stdfx.h"
#include "GameGatewayMgr.h"
#include "serverinfo.h"
#include "Config.h"

extern int64 g_currenttime;

static const int s_backlog = 16;

CGameGatewayMgr::CGameGatewayMgr()
{
	m_GateList.clear();
}

CGameGatewayMgr::~CGameGatewayMgr()
{
	m_GateList.clear();
}

void CGameGatewayMgr::Destroy()
{
	CServerMgr::Destroy();
}

void CGameGatewayMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "当前注册的网关服务器数量：%d\n", (int)m_GateList.size());
}

void CGameGatewayMgr::ResetMsgNum()
{
	for (std::map<int, serverinfo*>::iterator itr = m_GateList.begin(); itr != m_GateList.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}
}

const char *CGameGatewayMgr::GetMsgNumInfo()
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

	tempbuf[sizeof(tempbuf) - len] = 0;
	tempbuf[sizeof(tempbuf) - 1] = 0;
	return tempbuf;
}

void CGameGatewayMgr::SendMsgToServer(Msg *pMsg, int nGateID, int nClientID)
{
	if (nGateID > 0)
	{
		serverinfo *info = FindServer(nGateID, ServerEnum::EST_GATE);
		if (info)
		{
			msgtail tail;
			tail.type = msgtail::enum_type_to_client;
			tail.id = nClientID;
			info->SendMsg(pMsg, &tail, sizeof(tail));
		}
		else
			log_error("请求发送消息到未知的网关,，网关ID:[%d]", nGateID);
	}
	else
	{
		msgtail tail;
		tail.type = msgtail::enum_type_to_client;
		tail.id = nClientID;
		for (std::map<int, serverinfo *>::iterator itr = m_GateList.begin(); itr != m_GateList.end(); ++itr)
		{
			itr->second->SendMsg(pMsg, &tail, sizeof(tail));
		}
	}
}

void CGameGatewayMgr::OnConnectDisconnect(serverinfo *info, bool overtime)
{
	switch (info->GetServerType())
	{
	case ServerEnum::EST_GATE:
	{
		m_GateList.erase(info->GetServerID());
		if (overtime)
			log_error("逻辑服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			log_error("逻辑服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		break;
	}
	default:
	{
		if (overtime)
			log_error("未注册的服务器超时移除, ip:[%s]", info->GetIP());
		else
			log_error("未注册的服务器关闭移除, ip:[%s]", info->GetIP());
	}
	}
}

void CGameGatewayMgr::ProcessMsg(serverinfo *info)
{
	Msg *pMsg;
	for (;;)
	{
		pMsg = info->GetMsg();
		if (!pMsg)
			return;
		switch (pMsg->GetMainType())
		{
		case SERVER_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				info->SendMsg(pMsg);
				info->SetPingTime(g_currenttime);
				break;
			}
			default:
			{
			}
			}
			break;
		}
		default:
		{
			switch (info->GetServerType())
			{
			case ServerEnum::EST_GATE:
			{
				//获取尾巴，看看是从哪个客户端来的消息
				msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
				pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));
				if (msgtail::enum_type_from_client == tl->type)
				{
					ProcessClientMsg(info->GetServerID(), tl->id, pMsg);
				}
				break;
			}
			}
		}
		}
	}
}

void CGameGatewayMgr::ProcessClientMsg(int gateid, int64 clientid, Msg *pMsg)
{
	
}

bool CGameGatewayMgr::AddNewServer(serverinfo *info, int nServerID, int nType)
{
	if (FindServer(nServerID, nType))
	{
		log_error("添加服务器失败！已经存在的服务器，远程服务器ID：[%d] IP:[%s]", nServerID, info->GetIP());
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
	default:
	{
		log_error("添加服务器失败！不存在的服务器类型，远程服务器ID：[%d] 类型：[%d] IP:[%s]", nServerID, nType, info->GetIP());
		return false;
	}
	}

	info->SetAlreadyRegister();
	info->SetServerID(nServerID);
	info->SetServerType(nType);
	_pList->insert(std::make_pair(nServerID, info));
	return true;
}

serverinfo *CGameGatewayMgr::FindServer(int nServerID, int nType)
{
	std::map<int, serverinfo*> *_pList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_GATE:
	{
		_pList = &m_GateList;
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