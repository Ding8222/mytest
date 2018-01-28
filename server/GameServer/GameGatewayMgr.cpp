#include"stdfx.h"
#include "GameGatewayMgr.h"
#include "serverinfo.h"
#include "Config.h"

#include "LoginType.h"
#include "Login.pb.h"

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

void CGameGatewayMgr::SendMsgToServer(Msg &pMsg, int nType, int nServerID, int64 nClientID)
{
	if (nServerID > 0)
	{
		serverinfo *info = FindServer(nServerID, nType);
		if (info)
		{
			msgtail tail;
			tail.id = nClientID;
			SendMsg(info, pMsg, &tail, sizeof(tail));
		}
		else
			log_error("请求发送消息到未知的网关,，网关ID:[%d]", nServerID);
	}
	else
	{
		msgtail tail;
		tail.id = nClientID;
		for (std::map<int, serverinfo *>::iterator itr = m_GateList.begin(); itr != m_GateList.end(); ++itr)
		{
			SendMsg(itr->second, pMsg, &tail, sizeof(tail));
		}
	}
}

void CGameGatewayMgr::SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int nType, int nServerID, int64 nClientID)
{
	if (nServerID > 0)
	{
		serverinfo *info = FindServer(nServerID, nType);
		if (info)
		{
			msgtail tail;
			tail.id = nClientID;
			SendMsg(info, pMsg, maintype, subtype, &tail, sizeof(tail));
		}
		else
			log_error("请求发送消息到未知的网关,，网关ID:[%d]", nServerID);
	}
	else
	{
		msgtail tail;
		tail.id = nClientID;
		for (std::map<int, serverinfo *>::iterator itr = m_GateList.begin(); itr != m_GateList.end(); ++itr)
		{
			SendMsg(itr->second, pMsg, maintype, subtype, &tail, sizeof(tail));
		}
	}
}

void CGameGatewayMgr::SendMsgToClient(Msg &pMsg, int64 nClientID)
{
	ClientSvr *cl = FindClientSvr(nClientID);
	if(cl)
		SendMsgToServer(pMsg, ServerEnum::EST_GATE, cl->ServerID, nClientID);
}

void CGameGatewayMgr::SendMsgToClient(google::protobuf::Message &pMsg, int maintype, int subtype, int64 nClientID)
{
	ClientSvr *cl = FindClientSvr(nClientID);
	if (cl)
		SendMsgToServer(pMsg, maintype, subtype, ServerEnum::EST_GATE, cl->ServerID, nClientID);
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
			case SVR_SUB_NEW_CLIENT:
			{
				msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
				pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));
				
				AddNewClientSvr(info->GetServerType(), info->GetServerID(), tl->id);

				//通知CLient登录成功
				netData::LoginRet sendMsg;
				sendMsg.set_ncode(netData::LoginRet::EC_SUCC);

				SendMsgToClient(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_LOGIN_RET, tl->id);
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
				ProcessGameMsg(info, pMsg);
				break;
			}
			}
		}
		}
	}
}

void CGameGatewayMgr::ProcessGameMsg(serverinfo *info, Msg *pMsg)
{
	//获取尾巴，看看是从哪个客户端来的消息
	msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
	pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

	ProcessClientMsg(info->GetServerID(), tl->id, pMsg);
}

void CGameGatewayMgr::ProcessClientMsg(int gateid, int64 clientid, Msg *pMsg)
{
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_PLAYER_LIST:
		{

			break;
		}
		default:
		{
		}
		}
		break;
	}
	default:
		break;
	}
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

bool CGameGatewayMgr::AddNewClientSvr(int servertype, int serverid, int64 clientid)
{
	auto iter = m_ClientSvr.find(clientid);
	if (iter == m_ClientSvr.end())
	{
		m_ClientSvr.insert(std::make_pair(clientid, ClientSvr(servertype, serverid, clientid)));
	}
	else
	{
		// todo
		// T下现有的
	}

	return true;
}

ClientSvr *CGameGatewayMgr::FindClientSvr(int64 clientid)
{
	auto iter = m_ClientSvr.find(clientid);
	if (iter != m_ClientSvr.end())
	{
		return &(iter->second);
	}

	return nullptr;
}