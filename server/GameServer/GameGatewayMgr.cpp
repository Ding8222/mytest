#include "GameCenterConnect.h"
#include "GameGatewayMgr.h"
#include "serverinfo.h"
#include "Config.h"
#include "PlayerMgr.h"
#include "serverlog.h"
#include "Utilities.h"
#include "PlayerOperate.h"

#include "ServerType.h"
#include "ClientType.h"
#include "LoginType.h"
#include "ServerMsg.pb.h"
#include "Login.pb.h"
#include "ClientMsg.pb.h"

extern int64 g_currenttime;

static const int s_backlog = 16;

CGameGatewayMgr::CGameGatewayMgr()
{
	m_GateID = 0;
	m_GateList.clear();
}

CGameGatewayMgr::~CGameGatewayMgr()
{
	Destroy();
}

void CGameGatewayMgr::Destroy()
{
	m_GateID = 0;
	m_GateList.clear();
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

	tempbuf[sizeof(tempbuf) - len] = 0;
	tempbuf[sizeof(tempbuf) - 1] = 0;
	return tempbuf;
}

void CGameGatewayMgr::SendMsgToServer(Msg &pMsg, int nType, int64 nClientID, int nServerID, bool bBroad)
{
	if (!bBroad)
	{
		serverinfo *info = FindServer(nServerID, nType);
		if (info)
		{
			msgtail tail;
			tail.id = nClientID;
			SendMsg(info, pMsg, &tail, sizeof(tail));
		}
		else
			RunStateError("请求发送消息到未知的网关,网关ID:[%d]", nServerID);
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

void CGameGatewayMgr::SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int nType, int64 nClientID, int nServerID, bool bBroad)
{
	if (!bBroad)
	{
		serverinfo *info = FindServer(nServerID, nType);
		if (info)
		{
			msgtail tail;
			tail.id = nClientID;
			SendMsg(info, pMsg, maintype, subtype, &tail, sizeof(tail));
		}
		else
			RunStateError("请求发送消息到未知的网关,网关ID:[%d]", nServerID);
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

void CGameGatewayMgr::OnConnectDisconnect(serverinfo *info, bool overtime)
{
	switch (info->GetServerType())
	{
	case ServerEnum::EST_GATE:
	{
		CPlayerMgr::Instance().AsGateServerDisconnect(info->GetServerID());
		m_GateList.erase(info->GetServerID());
		if (overtime)
			RunStateError("网关服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("网关服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		break;
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
				info->SendMsg(pMsg);
				info->SetPingTime(g_currenttime);
				break;
			}
			case SVR_SUB_DEL_CLIENT:
			{
				svrData::DelClient msg;
				_CHECK_PARSE_(pMsg, msg);

				CPlayer *player = CPlayerMgr::Instance().FindPlayerByClientID(tl->id);
				if (FuncUti::isValidCret(player))
				{
					msg.set_account(player->GetAccount());
					FuncUti::SendMsgToCenter(player, msg, SERVER_TYPE_MAIN, SVR_SUB_DEL_CLIENT);
					RunStateLog("玩家连接断开！账号：%s，角色名称：%s", player->GetAccount().c_str(), player->GetName());
					CPlayerMgr::Instance().DelPlayer(tl->id);
				}
				break;
			}
			case SVR_SUB_PLAYERDATA:
			{
				CPlayerMgr::Instance().AddPlayer(info, tl->id);
				CPlayer *player = CPlayerMgr::Instance().FindPlayerByClientID(tl->id);
				netData::LoginRet sendMsg;
				if (FuncUti::isValidCret(player))
				{
					if (player->LoadData(pMsg))
					{
						sendMsg.set_ntempid(player->GetTempID());
						sendMsg.set_ncode(netData::LoginRet::EC_SUCC);
					}
					else
						sendMsg.set_ncode(netData::LoginRet::EC_LOADDATA);
				}
				else
					sendMsg.set_ncode(netData::LoginRet::EC_ADDPLAYER);

				FuncUti::SendPBNoLoop(player, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_LOGIN_RET);
				break;
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
				ProcessClientMsg(tl->id, pMsg);
				break;
			}
			}
		}
		}
	}
}

void CGameGatewayMgr::ProcessClientMsg(int32 clientid, Msg *pMsg)
{
	CPlayer *player = CPlayerMgr::Instance().FindPlayerByClientID(clientid);
	if (FuncUti::isValidCret(player))
	{
		Operate(player, pMsg);
	}
}

bool CGameGatewayMgr::AddNewServer(serverinfo *info, int nServerID, int nType)
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