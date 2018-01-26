﻿#include"stdfx.h"
#include "ServerMgr.h"
#include "serverinfo.h"

extern int64 g_currenttime;

static const int s_backlog = 16;

CServerMgr::CServerMgr()
{
	m_ListenPort = 0;
	m_OverTime = 0;
	m_ServerID = 0;

	m_Listen = NULL;

	m_List.clear();
	m_WaitRemove.clear();
}

CServerMgr::~CServerMgr()
{
	Destroy();
}

bool CServerMgr::Init(int serverid, int port, int overtime)
{
	m_ListenPort = port;
	m_OverTime = overtime;
	m_ServerID = serverid;

	m_Listen = lxnet::Listener::Create();
	if (!m_Listen)
	{
		log_error("创建Listener失败!");
		return false;
	}

	if (!TestAndListen())
		return false;
	return true;
}

void CServerMgr::Run()
{
	AcceptNewClient();
	Process();

	CheckAndRemove();
}

void CServerMgr::EndRun()
{
	for (std::list<serverinfo*>::iterator itr = m_List.begin(); itr != m_List.end(); ++itr)
	{
		(*itr)->GetCon()->CheckSend();
		(*itr)->GetCon()->CheckRecv();
	}
}

void CServerMgr::Destroy()
{
	if (m_Listen)
	{
		StopListen();
		lxnet::Listener::Release(m_Listen);
		m_Listen = NULL;
	}

	for (std::list<serverinfo*>::iterator itr = m_List.begin(); itr != m_List.end(); ++itr)
	{
		serverinfo_release(*itr);
	}
	m_List.clear();

	for (std::list<serverinfo*>::iterator itr = m_WaitRemove.begin(); itr != m_WaitRemove.end(); ++itr)
	{
		serverinfo_release(*itr);
	}
	m_WaitRemove.clear();
}

void CServerMgr::GetCurrentInfo(char *buf, size_t buflen)
{

}

void CServerMgr::StopListen()
{
	if (!m_Listen->IsClose())
	{
		m_Listen->Close();
		log_error("停止监听 %d", m_ListenPort);
	}
}

bool CServerMgr::TestAndListen()
{
	if (!m_Listen->IsClose())
		return true;

	if (m_Listen->Listen(m_ListenPort, s_backlog))
	{
		log_error("监听端口 %d 成功!", m_ListenPort);
		return true;
	}

	log_error("监听端口 %d 失败!", m_ListenPort);
	return false;
}

void CServerMgr::AcceptNewClient()
{
	lxnet::Socketer *sock = NULL;
	serverinfo *newinfo = NULL;
	for (int i = 0; i < s_backlog; ++i)
	{
		if (!m_Listen->CanAccept())
			break;

		sock = m_Listen->Accept(true);
		if (!sock)
			continue;
		newinfo = serverinfo_create();
		if (!newinfo)
		{
			lxnet::Socketer::Release(sock);
			continue;
		}
		newinfo->SetConnectTime(g_currenttime);
		newinfo->SetPingTime(g_currenttime);
		newinfo->SetCon(sock);
		m_List.push_back(newinfo);

		char ip[128];
		sock->GetIP(ip, sizeof(ip) - 1);
		newinfo->SetIP(ip);
		log_error("有新的服务器连接, ip:%s", ip);
	}
}

void CServerMgr::Process()
{
	std::list<serverinfo *>::iterator itr, tempitr;
	for (itr = m_List.begin(); itr != m_List.end();)
	{
		tempitr = itr;
		++itr;

		// 超时未注册,设置待移除
		if ((*tempitr)->IsEnoughXMNotReg(g_currenttime))
		{
			(*tempitr)->SetRemove(g_currenttime);
		}

		// 实际移除
		if ((*tempitr)->IsNeedRemove())
		{
			(*tempitr)->GetCon()->CheckSend();
			m_WaitRemove.push_back(*tempitr);
			m_List.erase(tempitr);
			continue;
		}

		// 服务器连接关闭,移除
		if ((*tempitr)->GetCon()->IsClose())
		{
			OnConnectDisconnect(*tempitr);
			serverinfo_release(*tempitr);
			m_List.erase(tempitr);
			continue;
		}

		// 超时移除
		if ((*tempitr)->IsOverTime(g_currenttime, m_OverTime))
		{
			OnConnectDisconnect(*tempitr, true);
			serverinfo_release(*tempitr);
			m_List.erase(tempitr);
			continue;
		}

		if ((*tempitr)->IsAlreadyRegister())
		{
			ProcessMsg(*tempitr);
		}
		else
		{
			ProcessNotRegister(*tempitr);
		}
	}
}

void CServerMgr::ProcessNotRegister(serverinfo *info)
{
	MessagePack *pMsg = (MessagePack *)info->GetMsg();

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
				info->SendMsg(pMsg);
				info->SetPingTime(g_currenttime);
				break;
			}
			case SVR_SUB_SERVER_REGISTER:
			{
				OnServerRegister(info, pMsg);
				break;
			}
			default:
				break;
			}
			break;
		}
		break;
		}
	}
}

void CServerMgr::SendMsg(serverinfo *info, google::protobuf::Message &pMsg, int maintype, int subtype, void *adddata, size_t addsize)
{
	assert(info != nullptr);

	MessagePack pk;
	pk.Pack(&pMsg, maintype, subtype);
	info->SendMsg(&pk, adddata, addsize);
}


void CServerMgr::SendMsg(serverinfo *info, Msg *pMsg, void *adddata, size_t addsize)
{
	assert(info != nullptr);
	assert(pMsg != nullptr);

	info->SendMsg(pMsg, adddata, addsize);
}

void CServerMgr::OnServerRegister(serverinfo *info, MessagePack *pMsg)
{
	svrData::ServerRegister Msg;
	if (!pMsg->UnPack(Msg))
		return;
	
	if (Msg.nconnectid() != m_ServerID)
	{
		//要连的服务器ID不对
		svrData::ServerRegisterRet ret;
		ret.set_nretcode(svrData::ServerRegisterRet::EC_TO_CONNECT_ID_NOT_EQUAL);

		SendMsg(info, ret, SERVER_TYPE_MAIN, SVR_SUB_SERVER_REGISTER_RET);

		//延时删除
		info->SetRemove(g_currenttime);
		log_error("一个新的服务器注册失败！请求注册的ServerID和本机ServerID不同，远程服务器ID：[%d] IP:[%s]", Msg.nserverid(), info->GetIP());
		return;
	}

	if (AddNewServer(info, Msg.nserverid(), Msg.nservertype()))
	{
		//注册成功
		svrData::ServerRegisterRet ret;
		ret.set_nretcode(svrData::ServerRegisterRet::EC_SUCC);

		SendMsg(info, ret, SERVER_TYPE_MAIN, SVR_SUB_SERVER_REGISTER_RET);

		log_error("一个新的服务器注册成功，远程服务器ID：[%d] IP:[%s]", Msg.nserverid(), info->GetIP());
	}
	else
	{
		//id已存在
		svrData::ServerRegisterRet ret;
		ret.set_nretcode(svrData::ServerRegisterRet::EC_SERVER_ID_EXIST);

		SendMsg(info, ret, SERVER_TYPE_MAIN, SVR_SUB_SERVER_REGISTER_RET);

		//延时删除
		info->SetRemove(g_currenttime);
		log_error("一个新的服务器注册失败！远程服务器ID：[%d] IP:[%s]", Msg.nserverid(), info->GetIP());
	}
}

void CServerMgr::CheckAndRemove()
{
	serverinfo *info;
	for (;;)
	{
		if (m_WaitRemove.empty())
			break;
		info = m_WaitRemove.front();
		if (!info->CanRemove(g_currenttime))
			break;
		serverinfo_release(info);
		m_WaitRemove.pop_front();
	}
}