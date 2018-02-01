/*
* 网关连接管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ServerMgr.h"

class CGameGatewayMgr :public CServerMgr
{
public:
	CGameGatewayMgr();
	~CGameGatewayMgr();

	static CGameGatewayMgr &Instance()
	{
		static CGameGatewayMgr m;
		return m;
	}
	void Destroy();

	void GetCurrentInfo(char *buf, size_t buflen);
	void ResetMsgNum();
	const char *GetMsgNumInfo();

	void SendMsgToServer(Msg &pMsg, int nType, int nServerID = 0, int64 nClientID = 0);
	void SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int nType, int nServerID = 0, int64 nClientID = 0);

	void SendMsgToClient(Msg &pMsg, int64 nClientID = 0);
	void SendMsgToClient(google::protobuf::Message &pMsg, int maintype, int subtype, int64 nClientID = 0);

	void ServerRegisterSucc(int id, int type, const char *ip, int port);
	void OnConnectDisconnect(serverinfo *info, bool overtime = false);

	// 处理服务器发来的消息
	void ProcessMsg(serverinfo *info);
	// 处理Client发来的消息，此时Client已经验证成功了
	void ProcessClientMsg(int gateid, int64 clientid, Msg *pMsg);

private:
	// 添加新的Server
	bool AddNewServer(serverinfo *info, int nServerID, int nType);

public:
	serverinfo *FindServer(int nServerID, int nType);

public:
	// 获取连接上的网关id
	int GetGateID() { return m_GateID; }

private:
	// 所属网关
	int m_GateID;

	std::map<int, serverinfo *> m_GateList;
};