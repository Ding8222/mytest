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

	void OnConnectDisconnect(serverinfo *info, bool overtime = false);

	// 处理服务器发来的消息
	void ProcessMsg(serverinfo *info);
	// 处理逻辑服发来的消息
	void ProcessGameMsg(serverinfo *info, Msg *pMsg);
	// 处理Client发来的消息
	void ProcessClientMsg(int gateid, int64 clientid, Msg *pMsg);

private:
	bool AddNewServer(serverinfo *info, int nServerID, int nType);
	serverinfo *FindServer(int nServerID, int nType);

	// 有新的client连接进来
	bool AddNewClientSvr(int servertype, int serverid, int64 clientid);
	ClientSvr *FindClientSvr(int64 clientid);
private:
	std::map<int, serverinfo *> m_GateList;

	std::unordered_map<int64, ClientSvr> m_ClientSvr;
};