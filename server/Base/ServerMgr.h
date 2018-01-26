﻿/*
* 服务器连接管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <list>
#include <unordered_map>
#include "lxnet.h"

class serverinfo;
class CServerMgr
{
public:
	CServerMgr();
	~CServerMgr();

	bool Init(int serverid, int port, int overtime);
	void Run();
	void EndRun();
	virtual void Destroy();

	// 获取当前连接中的服务器数量
	virtual void GetCurrentInfo(char *buf, size_t buflen) = 0;
	// 重置消息数量
	virtual void ResetMsgNum() = 0;
	// 获取每个服务器中消息数量信息
	virtual const char *GetMsgNumInfo() = 0;

public:
	// 根据类型和SvrID发送消息给服务器
	virtual void SendMsgToServer(Msg *pMsg, int nType , int nServerID = 0) = 0;
	// 处理未注册服务器消息
	void ProcessNotRegister(serverinfo *info);
	// 处理已注册服务器消息
	virtual void ProcessMsg(serverinfo *info) = 0;
	// 发送消息
	void SendMsg(serverinfo *info, google::protobuf::Message &pMsg, int maintype, int subtype, void *adddata = nullptr, size_t addsize = 0);
	void SendMsg(serverinfo *info, Msg *pMsg, void *adddata = nullptr, size_t addsize = 0);
private:
	// 根据类型添加服务器
	virtual bool AddNewServer(serverinfo *info, int nServerID, int nType) = 0;
	// 根据类型查找服务器
	virtual serverinfo *FindServer(int nServerID, int nType) = 0;

private:
	// 停止监听
	void StopListen();
	// 尝试监听
	bool TestAndListen();
	// 接受新服务器连接
	void AcceptNewClient();
	// 处理所有连接进来的服务器
	void Process();
	// 服务器注册
	void OnServerRegister(serverinfo *info, MessagePack *pMsg);
	// 服务器断开
	virtual void OnConnectDisconnect(serverinfo *info, bool overtime = false) = 0;
	// 真正移除服务器连接
	void CheckAndRemove();
private:
	int m_ListenPort;
	int m_OverTime;
	int m_ServerID;

	lxnet::Listener *m_Listen;

	std::list<serverinfo *> m_List;
	std::list<serverinfo *> m_WaitRemove;
};
