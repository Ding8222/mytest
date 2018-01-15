#include "stdfx.h"
#include "client.h"
#include "clientmgr.h"
#include "player.h"

extern int64 g_currenttime;

CClient::CClient()
{
	m_ConnectTime = 0;
	m_PingTime = 0;
	m_Socket = nullptr;
	m_Ip.clear();
}

CClient::~CClient()
{
	m_ConnectTime = 0;
	m_PingTime = 0;
	if (m_Socket)
	{
		lxnet::Socketer::Release(m_Socket);
		m_Socket = nullptr;
	}
	m_Ip.clear();
}

bool CClient::bOverTime(int64 currenttime, int clientovertime)
{
	if (clientovertime > 0)
	{
		if (currenttime >= m_PingTime + clientovertime)
		{
			return true;
		}
	}

	return false;
}

void CClient::SetSocket(lxnet::Socketer *socket)
{
	m_Socket = socket;
}

void CClient::SendMsg(Msg *pMsg)
{
	assert(pMsg != nullptr);
	if (!pMsg)
		return;
	assert(m_Socket != nullptr);
	if (!m_Socket)
		return ;
	m_Socket->SendMsg(pMsg);
}

Msg *CClient::GetMsg()
{
	assert(m_Socket != nullptr);
	if (!m_Socket)
		return nullptr;
	Msg *msg = m_Socket->GetMsg();
	if (!msg)
		return nullptr;

	return msg;
}

// ������Ϣ
void CClient::ProcessMsg()
{
	Msg *pMsg = NULL;
	while (1)
	{
		pMsg = GetMsg();
		if (!pMsg)
			break;
		switch (pMsg->GetType())
		{
		case MSG_PING:
		{
			//�յ�ping��Ϣ��ʱ��Ҳ����ping��Ϣ��ǰ�ˣ�Ȼ������ping��Ϣ�Ľ���ʱ��
			MsgPing* msg = (MsgPing*)pMsg;
			if (msg)
			{
				msg->m_servertime = g_currenttime;
				SendMsg(pMsg);
				SetPingTime(g_currenttime);
			}
			break;
		}
		case MSG_END:
		{
			log_error("get end msg: %s", GetIP().c_str());
			break;
		}
		case MSG_CHAT:
		{
			//���յ���chat��Ϣ�����������ߵ�client
			MessagePack * msg = (MessagePack *)pMsg;
			char stBuff[521] = { 0 };
			msg->Begin();
			msg->GetString(stBuff, 512);
			log_error("%s : %s", GetPlayer()->GetName().c_str(), stBuff);
			CClientMgr::Instance().SendMsgToAll(msg);
			break;
		}
		case MSG_LOAD:
		{
			//����
			MessagePack * msg = (MessagePack *)pMsg;
			msg->Begin();
			char sName[48];
			msg->GetString(sName, 48);
			CPlayer * player = new CPlayer(this);
			SetPlayer(player);
			if (!player->LoadData())
			{
				log_error("Player %s Load Faild", sName);
			}
			break;
		}
		case MSG_MOVE:
		{
			//�ƶ�
			MessagePack * msg = (MessagePack *)pMsg;
			msg->Begin();
			float _Pos[3] = { 0 };
			_Pos[0] = msg->GetFloat();
			_Pos[1] = msg->GetFloat();
			_Pos[2] = msg->GetFloat();
			CPlayer * player = GetPlayer();
			player->MoveTo(_Pos[0], _Pos[1], _Pos[2]);
			break;
		}
		default:
		{
			log_error("msg type error: %s", GetIP().c_str());
		}
		}
	}
}