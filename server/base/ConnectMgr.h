#pragma once
/*
* 连接器管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include "lxnet.h"
#include <google/protobuf/message.h>
#include "platform_config.h"
#include "crosslib.h"

class connector;
class CConnectMgr
{
public:
	CConnectMgr();
	~CConnectMgr();

	bool Init(int serverid, int servertype, int pingtime, int overtime, int listenport = 0);
	void Run();
	void EndRun();
	void Destroy();

	bool IsAlreadyRegister(int id);

	// 获取当前连接中的服务器数量
	void GetCurrentInfo(char *buf, size_t buflen);
	// 重置消息数量
	void ResetMsgNum();
	// 获取每个服务器中消息数量信息
	const char *GetMsgNumInfo();

public:
	// 发送消息
	bool SendMsgToServer(int nServerID, google::protobuf::Message &pMsg, int maintype, int subtype, int64 nClientID = 0);
	bool SendMsgToServer(int nServerID, Msg &pMsg, int64 nClientID = 0);

	bool SendMsgToServer(connector *con, google::protobuf::Message &pMsg, int maintype, int subtype, int64 nClientID = 0);
	bool SendMsgToServer(connector *con, Msg &pMsg, int64 nClientID = 0);

	bool SendMsg(connector *info, google::protobuf::Message &pMsg, int maintype, int subtype, void *adddata = nullptr, size_t addsize = 0);
	bool SendMsg(connector *info, Msg &pMsg, void *adddata = nullptr, size_t addsize = 0);

public:
	// 根据类型添加连接器
	bool AddNewConnect(const char *ip, int port, int id);
	// 根据类型查找连接器
	connector *FindConnect(int nID);
private:
	// 尝试连接并请求注册
	void TryConnect(connector *);
	// 断开连接
	void OnConnectDisconnect(connector *);
	// 连接认证通过后，连接断开会被OnConnectDisconnect调用
	virtual void ConnectDisconnect(connector *) = 0;
	// 注册前处理的消息
	void ProcessRegister(connector *);
	// 注册后处理的消息
	virtual void ProcessMsg(connector *) = 0;
	// 注册成功后的回调
	virtual void ServerRegisterSucc(int id, const char *ip, int port) = 0;
private:
	int m_ServerID;
	int m_ServerType;
	int m_OverTime;
	int m_PingTime;
	int m_ListenPort;

	std::unordered_map<int, connector *> m_List;
};

