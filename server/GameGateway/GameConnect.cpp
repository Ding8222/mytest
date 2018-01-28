#include "stdfx.h"
#include "GameConnect.h"
#include "connector.h"
#include "config.h"

extern int64 g_currenttime;

CGameConnect::CGameConnect()
{

}

CGameConnect::~CGameConnect()
{

}

bool CGameConnect::Init()
{
	if (!CConnectMgr::AddNewConnect(
		CConfig::Instance().GetGameServerIP().c_str(),
		CConfig::Instance().GetGameServerPort(),
		CConfig::Instance().GetGameServerID()
	))
	{
		log_error("添加逻辑服务器失败!");
		return false;
	}

	return CConnectMgr::Init(
		CConfig::Instance().GetServerID(),
		CConfig::Instance().GetServerType(),
		CConfig::Instance().GetPingTime(),
		CConfig::Instance().GetOverTime()
	);
}

void CGameConnect::ConnectDisconnect(connector *)
{

}

void CGameConnect::ProcessMsg(connector *_con)
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
			case SVR_SUB_CLIENT_TOKEN:
			{

				msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
				pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

				svrData::ClientToken msg;
				_CHECK_PARSE_(pMsg, msg);

				ClientSvr *cl = new ClientSvr;
				cl->Token = msg.setoken();
				cl->Secret = msg.ssecret();
				AddNewClientSvrToken(cl);
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
		}
		}
	}
}

void CGameConnect::AddNewClientSvrToken(ClientSvr *cl)
{
	auto iter = m_ClientSvrToken.find(cl->Token);
	if (iter == m_ClientSvrToken.end())
	{
		m_ClientSvrToken[cl->Token] = cl;
		log_error("新的客户端待登录，token：%s", cl->Token.c_str());
	}
	else
	{
		// 已经存在
	}
}

ClientSvr *CGameConnect::FindClientSvr(std::string t)
{
	auto iter = m_ClientSvrToken.find(t);
	if (iter != m_ClientSvrToken.end())
	{
		return iter->second;
	}
	return nullptr;
}

bool CGameConnect::AddNewClientSvrID(std::string token, int servertype, int serverid, int64 clientid)
{
	auto iter = m_ClientSvrToken.find(token);
	if (iter == m_ClientSvrToken.end())
	{
		// 没有收到center的认证信息
		return false;
	}
	else
	{
		// 存在
		if (FindClientSvr(clientid) == nullptr)
		{
			ClientSvr *cl = iter->second;
			cl->ClientID = clientid;
			cl->ServerType = servertype;
			cl->ServerID = serverid;

			m_ClientSvrID[clientid] = iter->second;
			return true;
		}
		else
		{
			//存在已经认证过的

			return true;
		}
	}
}

ClientSvr *CGameConnect::FindClientSvr(int64 clientid)
{
	auto iter = m_ClientSvrID.find(clientid);
	if (iter != m_ClientSvrID.end())
	{
		return iter->second;
	}

	return nullptr;
}