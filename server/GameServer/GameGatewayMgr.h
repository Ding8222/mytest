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

	void SendMsgToServer(Msg *pMsg, int nGateID, int nClientID = 0);
	void OnConnectDisconnect(serverinfo *info, bool overtime = false);

	// 处理服务器发来的消息
	void ProcessMsg(serverinfo *info);
	// 处理Client发来的消息
	void ProcessClientMsg(int gateid, int64 clientid, Msg *pMsg);

private:
	bool AddNewServer(serverinfo *info, int nServerID, int nType);
	serverinfo *FindServer(int nServerID, int nType);

private:
	std::map<int, serverinfo *> m_GateList;
};