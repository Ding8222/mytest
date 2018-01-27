#include "stdfx.h"
#include "ServerConnect.h"
#include "Connector.h"

extern int64 g_currenttime;

CServerConnect::CServerConnect()
{
	m_IsReady = false;
	memset(m_IP, 0, MAX_IP_LEN);
	m_Port = 0;
	m_ID = 0;
	m_PingTime = 0;
	m_OverTime = 0;

	m_ServerID = 0;
	m_ServerType = 0;

	m_Con = NULL;
}

CServerConnect::~CServerConnect()
{
	Destroy();
}

bool CServerConnect::Init(const char *ip, int port, int id, int serverid, int servertype, int pingtime, int overtime)
{
	strncpy(m_IP, ip, MAX_IP_LEN - 1);
	m_IP[MAX_IP_LEN - 1] = 0;

	m_Port = port;
	m_ID = id;
	m_ServerID = serverid;
	m_PingTime = pingtime;
	m_OverTime = overtime;
	m_ServerType = servertype;

	m_Con = (connector *)malloc(sizeof(connector));
	if (!m_Con)
	{
		log_error("创建connect失败!");
		return false;
	}
	new(m_Con) connector();
	return true;
}

void CServerConnect::Run()
{
	if (!m_Con->IsAlreadyConnect())
	{
		TryConnect();
		return;
	}
	if (m_Con->NeedSendPing(g_currenttime, m_PingTime))
	{
		svrData::Ping msg;
		SendMsg(msg, SERVER_TYPE_MAIN, SVR_SUB_PING);
	}
	if (m_Con->IsOverTime(g_currenttime, m_OverTime))
	{
		log_error("连接远程服务器:[%d] 超时，准备断开重连!", m_ID);
		OnConnectDisconnect();
		return;
	}
	if (m_Con->IsClose())
	{
		log_error("远程服务器断开连接，准备断开重连!", m_ID);
		OnConnectDisconnect();
		return;
	}

	if (IsReady())
		ProcessMsg(m_Con);
	else
		ProcessRegister(m_Con);
}

void CServerConnect::Destroy()
{
	if (m_Con)
	{
		delete m_Con;
		m_Con = NULL;
	}
}

void CServerConnect::EndRun()
{
	m_Con->CheckSend();
	m_Con->CheckRecv();
}

void CServerConnect::SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int64 clientid)
{
	MessagePack pk;
	pk.Pack(&pMsg, maintype, subtype);

	assert(clientid > 0);
	if (clientid <= 0)
		return;

	msgtail tail;
	tail.id = clientid;
	m_Con->SendMsg(&pk, &tail, sizeof(tail));
}

void CServerConnect::SendMsgToServer(Msg &pMsg, int64 clientid)
{
	assert(clientid > 0);
	if (clientid <= 0)
		return;

	msgtail tail;
	tail.id = clientid;
	m_Con->SendMsg(&pMsg, &tail, sizeof(tail));
}

void CServerConnect::SendMsg(google::protobuf::Message &pMsg, int maintype, int subtype, void *adddata, size_t addsize)
{
	MessagePack pk;
	pk.Pack(&pMsg, maintype, subtype);
	m_Con->SendMsg(&pk);
}

void CServerConnect::SendMsg(Msg &pMsg, void *adddata, size_t addsize)
{
	m_Con->SendMsg(&pMsg);
}

void CServerConnect::ResetMsgNum()
{
	m_Con->ResetMsgNum();
}

int CServerConnect::GetRecvMsgNum()
{
	return m_Con->GetRecvMsgNum();
}

int CServerConnect::GetSendMsgNum()
{
	return m_Con->GetSendMsgNum();
}

void CServerConnect::TryConnect()
{
	if (m_Con->TryConnect(g_currenttime, m_IP, m_Port))
	{
		svrData::ServerRegister msg;
		msg.set_nserverid(m_ServerID);
		msg.set_nservertype(m_ServerType);
		msg.set_nconnectid(m_ID);

		SendMsg(msg, SERVER_TYPE_MAIN, SVR_SUB_SERVER_REGISTER);
		log_error("连接服务器成功!发送注册信息。服务器ID：[%d] IP:[%s]", m_ID, m_IP);
	}
}

void CServerConnect::OnConnectDisconnect()
{
	if (m_IsReady)
	{
		ConnectDisconnect();
	}

	m_Con->ResetConnect();
	m_IsReady = false;
}

void CServerConnect::ProcessRegister(connector *con)
{
	MessagePack *pMsg = (MessagePack *)con->GetMsg();

	//未注册，则一帧只处理一个消息。
	if (pMsg)
	{
		switch (pMsg->GetMainType())
		{
		case SERVER_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				con->SetRecvPingTime(g_currenttime);
				break;
			}
			case SVR_SUB_SERVER_REGISTER_RET:
			{
				svrData::ServerRegisterRet msg;
				if (pMsg->UnPack(msg))
				{
					switch (msg.nretcode())
					{
					case svrData::ServerRegisterRet::EC_SUCC:
					{
						// 认证成功
						m_IsReady = true;
						log_error("注册到远程服务器成功！");
						break;
					}
					case svrData::ServerRegisterRet::EC_SERVER_ID_EXIST:
					{
						// 已存在相同ServerID被注册
						log_error("注册到远程服务器失败！已存在相同ServerID被注册，远程服务器ID：[%d] IP:[%s]", m_ID, m_IP);
						exit(-1);
						break;
					}
					case svrData::ServerRegisterRet::EC_TO_CONNECT_ID_NOT_EQUAL:
					{
						// 请求注册的ServerID和远程ServerID不同
						log_error("注册到远程服务器失败！请求注册的ServerID和远程ServerID不同，远程服务器ID：[%d] IP:[%s]", m_ID, m_IP);
						exit(-1);
						break;
					}
					}
				}
				break;
			}
			default:
				break;
			}
			break;
		}
		}
	}
}