#include "ConnectMgr.h"
#include "Connector.h"
#include "log.h"
#include "msgbase.h"
#include "GlobalDefine.h"
#include "BaseConfig.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

extern int64 g_currenttime;

static const int32 s_backlog = 16;

CConnectMgr::CConnectMgr()
{
	m_ServerID = 0;
	m_ServerType = 0;
	m_OverTime = 0;
	m_PingTime = 0;
	
	m_List.clear();
	m_ServerName.clear();
}

CConnectMgr::~CConnectMgr()
{
	Destroy();
}

bool CConnectMgr::Init(int32 serverid, int32 servertype, const char *servername, int32 pingtime, int32 overtime)
{
	m_ServerID = serverid;
	m_ServerType = servertype;
	m_PingTime = pingtime;
	m_OverTime = overtime;
	m_ServerName = servername;

	return true;
}

void CConnectMgr::Run()
{
	std::unordered_map<int32, connector *>::iterator itr;
	for (itr = m_List.begin(); itr != m_List.end(); ++itr)
	{
		connector *con = itr->second;
		if (!con->IsAlreadyConnect())
		{
			TryConnect(con);
			continue;
		}
		if (con->NeedSendPing(g_currenttime, m_PingTime))
		{
			svrData::Ping msg;
			msgtail tail;
			tail.id = m_ServerID;
			SendMsg(con, msg, SERVER_TYPE_MAIN, SVR_SUB_PING, &tail, sizeof(tail));
		}
		if (con->IsOverTime(g_currenttime, m_OverTime))
		{
			log_error("连接远程服务器[%s], ID:[%d] 超时，准备断开重连!", con->GetConnectName(), con->GetConnectID());
			OnConnectDisconnect(con);
			continue;
		}
		if (con->IsClose())
		{
			log_error("远程服务器[%s], ID:[%d] 断开连接，准备断开重连!", con->GetConnectName(), con->GetConnectID());
			OnConnectDisconnect(con);
			continue;
		}

		if (con->IsAlreadyRegister())
			ProcessMsg(con);
		else
			ProcessRegister(con);
	}
}

void CConnectMgr::EndRun()
{
	for (std::unordered_map<int32, connector *>::iterator itr = m_List.begin(); itr != m_List.end(); ++itr)
	{
		itr->second->CheckSend();
		itr->second->CheckRecv();
	}
}

void CConnectMgr::Destroy()
{
	for (std::unordered_map<int32, connector *>::iterator itr = m_List.begin(); itr != m_List.end(); ++itr)
	{
		ConnectorRelease(itr->second);
	}
	m_List.clear();
	m_ServerName.clear();
}

bool CConnectMgr::IsAlreadyRegister(int32 id)
{
	connector *con= FindConnect(id);
	if (con)
	{
		return con->IsAlreadyRegister();
	}
	return false;
}

void CConnectMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "当前连接的服务器数量：%d\n",(int32)m_List.size());
}

void CConnectMgr::ResetMsgNum()
{
	for (std::unordered_map<int32, connector *>::iterator itr = m_List.begin(); itr != m_List.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}
}

const char *CConnectMgr::GetMsgNumInfo()
{
	static char tempbuf[1024 * 32];
	char *buf = tempbuf;
	size_t len = sizeof(tempbuf);
	int32 res = 0;
	connector *co = nullptr;
	for (std::unordered_map<int32, connector *>::iterator itr = m_List.begin(); itr != m_List.end(); ++itr)
	{
		co = itr->second;
		snprintf(buf, len - 1, "连接服务器[%s], ID:%d, 收到消息数量:%d, 发送消息数量:%d\n", \
			co->GetConnectName(), co->GetConnectID(), co->GetRecvMsgNum(), co->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	tempbuf[sizeof(tempbuf) - len] = 0;
	tempbuf[sizeof(tempbuf) - 1] = 0;
	return tempbuf;
}

bool CConnectMgr::SendMsgToServer(int32 nServerID, google::protobuf::Message &pMsg, int32 maintype, int32 subtype, int64 nClientID)
{
	connector *con = FindConnect(nServerID);
	if (con)
	{
		return SendMsgToServer(con, pMsg, maintype, subtype, nClientID);
	}
	return false;
}

bool CConnectMgr::SendMsgToServer(int32 nServerID, Msg &pMsg, int64 nClientID)
{
	connector *con = FindConnect(nServerID);
	if (con)
	{
		return SendMsgToServer(con, pMsg, nClientID);
	}
	return false;
}

bool CConnectMgr::SendMsgToServer(connector *con, google::protobuf::Message &pMsg, int32 maintype, int32 subtype, int64 clientid)
{
	assert(con);
	assert(clientid >= 0);
	if (clientid < 0)
		return false;

	msgtail tail;
	tail.id = clientid;
	return SendMsg(con, pMsg, maintype, subtype, &tail, sizeof(tail));
}

bool CConnectMgr::SendMsgToServer(connector *con, Msg &pMsg, int64 clientid)
{
	assert(con);
	assert(clientid >= 0);
	if (clientid < 0)
		return false;

	msgtail tail;
	tail.id = clientid;
	return SendMsg(con, pMsg, &tail, sizeof(tail));
}

bool CConnectMgr::SendMsg(connector *con, google::protobuf::Message &pMsg, int32 maintype, int32 subtype, void *adddata, size_t addsize)
{
	assert(con != nullptr);

	MessagePack pk;
	pk.Pack(&pMsg, maintype, subtype);
	return con->SendMsg(&pk, adddata, addsize);
}


bool CConnectMgr::SendMsg(connector *con, Msg &pMsg, void *adddata, size_t addsize)
{
	assert(con != nullptr);

	return con->SendMsg(&pMsg, adddata, addsize);
}

bool CConnectMgr::AddNewConnect(const char *ip, int32 port, int32 id, const char *name)
{
	if (FindConnect(id))
	{
		log_error("创建[%s]连接失败, 已经存在的服务器id，ip:%s port:%d id:%d", name, ip, port, id);
		return false;
	}
	connector *newcon = ConnectorCreate();
	if (!newcon)
	{
		log_error("创建connector失败！", ip, port, id);
		return false;
	}
	newcon->SetConnectInfo(ip, port, id);
	newcon->SetConnectName(name);
	m_List.insert(std::make_pair(id,newcon));
	log_writelog("创建[%s]连接成功, ip:%s port:%d id:%d", name, ip, port, id);
	return true;
}

connector *CConnectMgr::FindConnect(int32 nID)
{
	std::unordered_map<int32, connector *>::iterator iter = m_List.find(nID);
	if (iter != m_List.end())
	{
		return iter->second;
	}
	return nullptr;
}

void CConnectMgr::TryConnect(connector *con)
{
	if (con->TryConnect(g_currenttime))
	{
		svrData::ServerRegister sendMsg;
		sendMsg.set_nserverid(m_ServerID);
		sendMsg.set_nservertype(m_ServerType);
		sendMsg.set_nconnectid(con->GetConnectID());
		sendMsg.set_name(m_ServerName);

		if(SendMsg(con, sendMsg, SERVER_TYPE_MAIN, SVR_SUB_SERVER_REGISTER))
			log_writelog("[%s]连接服务器成功!发送注册信息成功！服务器ID：[%d] IP:[%s]", con->GetConnectName(), con->GetConnectID(), con->GetConnectIP());
		else
			log_error("[%s]连接服务器成功!发送注册信息失败！服务器ID：[%d] IP:[%s]", con->GetConnectName(), con->GetConnectID(), con->GetConnectIP());
	}
}

void CConnectMgr::OnConnectDisconnect(connector *con)
{
	if (con->IsAlreadyRegister())
	{
		ConnectDisconnect(con);
	}

	con->ResetConnect();
}

void CConnectMgr::ProcessRegister(connector *con)
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
				_CHECK_PARSE_(pMsg, msg);

				switch (msg.nretcode())
				{
				case svrData::ServerRegisterRet::EC_SUCC:
				{
					// 认证成功
					con->SetAlreadyRegister(true);
					ServerRegisterSucc(con);
					log_writelog("[%s]注册到远程服务器成功！", con->GetConnectName());
					break;
				}
				case svrData::ServerRegisterRet::EC_SERVER_ID_EXIST:
				{
					// 已存在相同ServerID被注册
					log_error("[%s]注册到远程服务器失败！已存在相同ServerID被注册，远程服务器ID：[%d] IP:[%s]", con->GetConnectName(), con->GetConnectID(), con->GetConnectIP());
					exit(-1);
					break;
				}
				case svrData::ServerRegisterRet::EC_TO_CONNECT_ID_NOT_EQUAL:
				{
					// 请求注册的ServerID和远程ServerID不同
					log_error("[%s]注册到远程服务器失败！请求注册的ServerID和远程ServerID不同，远程服务器ID：[%d] IP:[%s]", con->GetConnectName(), con->GetConnectID(), con->GetConnectIP());
					exit(-1);
					break;
				}
				}
				break;
			}
			}
			break;
		}
		}
	}
}
