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

	bool Init(int32 serverid, int32 servertype, const char *servername, int32 pingtime, int32 overtime);
	void Run();
	void EndRun();
	virtual void Destroy();

	bool IsAlreadyRegister(int32 id);

	// 获取当前连接中的服务器数量
	void GetCurrentInfo(char *buf, size_t buflen);
	// 重置消息数量
	void ResetMsgNum();
	// 获取每个服务器中消息数量信息
	const char *GetMsgNumInfo();

public:
	// 发送消息
	bool SendMsgToServer(int32 nServerID, google::protobuf::Message &pMsg, int32 maintype, int32 subtype, int64 nClientID = 0);
	bool SendMsgToServer(int32 nServerID, Msg &pMsg, int64 nClientID = 0);

	bool SendMsgToServer(connector *con, google::protobuf::Message &pMsg, int32 maintype, int32 subtype, int64 nClientID = 0);
	bool SendMsgToServer(connector *con, Msg &pMsg, int64 nClientID = 0);

	bool SendMsg(connector *info, google::protobuf::Message &pMsg, int32 maintype, int32 subtype, void *adddata = nullptr, size_t addsize = 0);
	bool SendMsg(connector *info, Msg &pMsg, void *adddata = nullptr, size_t addsize = 0);

public:
	// 根据类型添加连接器
	bool AddNewConnect(const char *ip, int32 port, int32 id, const char *name);
	// 根据类型查找连接器
	connector *FindConnect(int32 nID);
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
	virtual void ServerRegisterSucc(connector *) {}
private:
	int32 m_ServerID;
	int32 m_ServerType;
	int32 m_OverTime;
	int32 m_PingTime;
	std::string m_ServerName;

	std::unordered_map<int32, connector *> m_List;
};

