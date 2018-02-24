#include"stdfx.h"
#include "GameCenterConnect.h"
#include "GameGatewayMgr.h"
#include "serverinfo.h"
#include "Config.h"
#include "PlayerMgr.h"

#include "ClientType.h"
#include "LoginType.h"
#include "Login.pb.h"
#include "ClientMsg.pb.h"

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
			log_error("请求发送消息到未知的网关,网关ID:[%d]", nServerID);
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
			log_error("请求发送消息到未知的网关,网关ID:[%d]", nServerID);
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
	int serverid = CPlayerMgr::Instance().FindPlayerGateID(nClientID);
	if(serverid > 0)
		SendMsgToServer(pMsg, ServerEnum::EST_GATE,  nClientID, serverid);
}

void CGameGatewayMgr::SendMsgToClient(google::protobuf::Message &pMsg, int maintype, int subtype, int64 nClientID)
{
	int serverid = CPlayerMgr::Instance().FindPlayerGateID(nClientID);
	if (serverid > 0)
		SendMsgToServer(pMsg, maintype, subtype, ServerEnum::EST_GATE, nClientID, serverid);
}

void CGameGatewayMgr::ServerRegisterSucc(int id, int type, const char *ip, int port)
{

}

void CGameGatewayMgr::OnConnectDisconnect(serverinfo *info, bool overtime)
{
	switch (info->GetServerType())
	{
	case ServerEnum::EST_GATE:
	{
		CPlayerMgr::Instance().DelAllPlayer();
		m_GateList.erase(info->GetServerID());
		if (overtime)
			log_error("网关服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			log_error("网关服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
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
			case SVR_SUB_NEW_CLIENT:
			{
				CPlayerMgr::Instance().AddPlayer(tl->id, info->GetServerID());

				//通知CLient登录成功
				netData::LoginRet sendMsg;
				sendMsg.set_ncode(netData::LoginRet::EC_SUCC);

				SendMsgToClient(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_LOGIN_RET, tl->id);
				CGameCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), *pMsg, tl->id);
				break;
			}
			case SVR_SUB_DEL_CLIENT:
			{
				svrData::DelClient msg;
				_CHECK_PARSE_(pMsg, msg);

				CPlayerMgr::Instance().DelPlayer(msg.nclientid());
				CGameCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), *pMsg, tl->id);
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
				ProcessClientMsg(info->GetServerID(), tl->id, pMsg);
				break;
			}
			}
		}
		}
	}
}

void CGameGatewayMgr::ProcessClientMsg(int gateid, int64 clientid, Msg *pMsg)
{
	switch (pMsg->GetMainType())
	{
	case LOGIN_TYPE_MAIN:
	{
// 		switch (pMsg->GetSubType())
// 		{
// 		default:
// 		{
// 		}
// 		}
// 		break;
	}
	case CLIENT_TYPE_MAIN:
	{
		switch (pMsg->GetSubType())
		{
		case CLIENT_SUB_MOVE:
		{
			netData::PlayerMove msg;
			_CHECK_PARSE_(pMsg, msg);
			CPlayer *player = CPlayerMgr::Instance().FindPlayerByClientID(clientid);
			if (player)
			{
				netData::PlayerMoveRet sendMsg;
				if (player->MoveTo(msg.x(), msg.y(), msg.z()))
				{
					sendMsg.set_x(msg.x());
					sendMsg.set_y(msg.y());
					sendMsg.set_z(msg.z());
				}
				else
				{
					float _Pos[EPP_MAX] = { 0 };
					player->GetNowPos(_Pos[EPP_X], _Pos[EPP_Y], _Pos[EPP_Z]);
					sendMsg.set_x(_Pos[EPP_X]);
					sendMsg.set_y(_Pos[EPP_Y]);
					sendMsg.set_z(_Pos[EPP_Z]);
				}
				SendMsgToClient(sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_MOVE_RET, clientid);
			}
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