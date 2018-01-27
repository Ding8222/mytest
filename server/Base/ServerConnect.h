/*
* 服务器连接器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <string>

class connector;
class CServerConnect
{
public:
	CServerConnect();
	~CServerConnect();

	// 目标地址、端口、远程服务器id、本地id、本地类型、ping时间、超时重试时间
	bool Init(const char *ip, int port, int id, int serverid, int servertype, int pingtime, int overtime);

	void Run();

	void Destroy();

	void EndRun();

	bool IsReady() { return m_IsReady; }

	void SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int64 clientid);
	void SendMsgToServer(Msg &pMsg, int64 clientid);

	void SendMsg(google::protobuf::Message &pMsg, int maintype, int subtype, void *adddata = 0, size_t addsize = 0);
	void SendMsg(Msg &pMsg, void *adddata = 0, size_t addsize = 0);

	void ResetMsgNum();
	int GetRecvMsgNum();
	int GetSendMsgNum();
private:
	// 尝试连接并请求注册
	void TryConnect();
	// 断开连接
	void OnConnectDisconnect();
	// 连接认证通过后，连接断开会被OnConnectDisconnect调用
	virtual void ConnectDisconnect() = 0;
	// 注册前处理的消息
	void ProcessRegister(connector *);
	// 注册后处理的消息
	virtual void ProcessMsg(connector *) = 0;
private:
	bool m_IsReady;

	char m_IP[MAX_IP_LEN];
	int m_Port;
	int m_ID;
	int m_PingTime;
	int m_OverTime;

	int m_ServerID;
	int m_ServerType;

	connector *m_Con;
};