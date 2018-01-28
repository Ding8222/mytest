#include"stdfx.h"
#include "CentServerMgr.h"
#include "serverinfo.h"
#include "Config.h"

#include "LoginType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"

extern int64 g_currenttime;

static const int s_backlog = 16;

CCentServerMgr::CCentServerMgr()
{
	m_GameList.clear();
	m_LoginList.clear();
	m_DBList.clear();
}

CCentServerMgr::~CCentServerMgr()
{

}

void CCentServerMgr::Destroy()
{
	CServerMgr::Destroy();
	m_GameList.clear();
	m_LoginList.clear();
	m_DBList.clear();
}

void CCentServerMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "当前注册的服务器信息：\n逻辑服务器数量：%d\n登陆服务器数量：%d\n数据服务器数量：%d\n",
		(int)m_GameList.size(), (int)m_LoginList.size(), (int)m_DBList.size());
}

void CCentServerMgr::ResetMsgNum()
{
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

	tempbuf[sizeof(tempbuf) - len] = 0;
	tempbuf[sizeof(tempbuf) - 1] = 0;
	return tempbuf;
}

void CCentServerMgr::SendMsgToServer(Msg &pMsg, int nType, int nServerID, int64 nClientID)
{
	std::map<int, serverinfo *> *iterList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_GAME:
	{
		iterList = &m_GameList;
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		iterList = &m_LoginList;
		break;
	}
	case ServerEnum::EST_DB:
	{
		iterList = &m_DBList;
		break;
	}
	default:
		log_error("请求发送消息到未知类型的服务器，服务器类型:[%d]", nType);
		break;
	}

	if (iterList)
	{
		if (nServerID > 0)
		{
			std::map<int, serverinfo *>::iterator iterFind = iterList->find(nServerID);
			if (iterFind != iterList->end())
			{
				msgtail tail;
				tail.id = nClientID;
				SendMsg(iterFind->second, pMsg, &tail, sizeof(tail));
			}
			else
				log_error("请求发送消息到未知的服务器,，服务器ID:[%d]", nServerID);
		}
		else
		{
			msgtail tail;
			tail.id = nClientID;
			for (std::map<int, serverinfo *>::iterator itr = iterList->begin(); itr != iterList->end(); ++itr)
			{
				SendMsg(itr->second, pMsg, &tail, sizeof(tail));
			}
		}
	}
}

void CCentServerMgr::SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int nType, int nServerID, int64 nClientID)
{
	std::map<int, serverinfo *> *iterList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_GAME:
	{
		iterList = &m_GameList;
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		iterList = &m_LoginList;
		break;
	}
	case ServerEnum::EST_DB:
	{
		iterList = &m_DBList;
		break;
	}
	default:
		log_error("请求发送消息到未知类型的服务器，服务器类型:[%d]", nType);
		break;
	}

	if (iterList)
	{
		if (nServerID > 0)
		{
			std::map<int, serverinfo *>::iterator iterFind = iterList->find(nServerID);
			if (iterFind != iterList->end())
			{
				msgtail tail;
				tail.id = nClientID;
				SendMsg(iterFind->second, pMsg, maintype, subtype, &tail, sizeof(tail));
			}
			else
				log_error("请求发送消息到未知的服务器，服务器ID:[%d]", nServerID);
		}
		else
		{
			msgtail tail;
			tail.id = nClientID;
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
	case ServerEnum::EST_GAME:
	{
		m_GameList.erase(info->GetServerID());
		if (overtime)
			log_error("逻辑服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			log_error("逻辑服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		break;
	}
	case ServerEnum::EST_LOGIN:
	{
		m_LoginList.erase(info->GetServerID());
		if (overtime)
			log_error("登陆服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			log_error("登陆服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());

		break;
	}
	case ServerEnum::EST_DB:
	{
		m_DBList.erase(info->GetServerID());
		if (overtime)
			log_error("数据服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			log_error("数据服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());

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

void CCentServerMgr::ProcessMsg(serverinfo *info)
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
			// 来自LoginSvr的消息
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
				AddNewClient(info->GetServerType(), info->GetServerID(), tl->id);
				break;
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
				ProcessGameMsg(info, pMsg);
				break;
			}
			case ServerEnum::EST_LOGIN:
			{
				// 来自LoginSvr的消息
				ProcessLoginMsg(info, pMsg);
				break;
			}
			case ServerEnum::EST_DB:
			{
				// 来自DBSvr的消息
				ProcessDBMsg(info, pMsg);
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

void CCentServerMgr::ProcessGameMsg(serverinfo *info, Msg *pMsg)
{
	msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
	pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));
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

void CCentServerMgr::ProcessLoginMsg(serverinfo *info, Msg *pMsg)
{
	msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
	pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_AUTH:
		{
			ClientSvr *cl = FindClientSvr(tl->id);
			if (cl)
			{
				netData::Auth msg;
				_CHECK_PARSE_(pMsg, msg);
				cl->Token = msg.setoken();
				cl->Secret = msg.ssecret();
				SendMsgToServer(*pMsg, ServerEnum::EST_DB, 0, cl->ClientID);
			}
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

void CCentServerMgr::ProcessDBMsg(serverinfo *info, Msg *pMsg)
{
	msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
	pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case LOGIN_SUB_AUTH_RET:
		{
			ClientSvr *cl = FindClientSvr(tl->id);
			if (cl)
			{
				netData::AuthRet msg;
				_CHECK_PARSE_(pMsg, msg);
				msg.set_nserverid(3000);
				msg.set_ip("127.0.0.1");
				msg.set_port(30012);

				SendMsgToServer(msg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET, cl->ServerType, cl->ServerID, cl->ClientID);

				svrData::ClientToken sendMsg;
				sendMsg.set_setoken(cl->Token);
				sendMsg.set_ssecret(cl->Secret);
				SendMsgToServer(sendMsg, SERVER_TYPE_MAIN, SVR_SUB_CLIENT_TOKEN, ServerEnum::EST_GAME, 4000, cl->ClientID);
			}
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

bool CCentServerMgr::AddNewServer(serverinfo *info, int nServerID, int nType)
{
	if (FindServer(nServerID, nType))
	{
		log_error("添加服务器失败！已经存在的服务器，远程服务器ID：[%d] IP:[%s]", nServerID, info->GetIP());
		return false;
	}
	
	std::map<int, serverinfo*> *_pList = nullptr;
	switch (nType)
	{
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

serverinfo *CCentServerMgr::FindServer(int nServerID, int nType)
{
	std::map<int, serverinfo*> *_pList = nullptr;
	switch (nType)
	{
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

bool CCentServerMgr::AddNewClient(int servertype, int serverid, int64 clientid)
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

ClientSvr *CCentServerMgr::FindClientSvr(int64 clientid)
{
	auto iter = m_ClientSvr.find(clientid);
	if (iter != m_ClientSvr.end())
	{
		return &(iter->second);
	}

	return nullptr;
}