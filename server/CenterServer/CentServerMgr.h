/*
* 中心服务管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ServerMgr.h"

class CCentServerMgr :public CServerMgr
{
public:
	CCentServerMgr();
	~CCentServerMgr();

	static CCentServerMgr &Instance()
	{
		static CCentServerMgr m;
		return m;
	}
	void Destroy();

	void GetCurrentInfo(char *buf, size_t buflen);
	void ResetMsgNum();
	const char *GetMsgNumInfo();

	void SendMsgToServer(Msg &pMsg, int nType, int nServerID = 0, int64 nClientID = 0);
	void SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int nType, int nServerID = 0, int64 nClientID = 0);
	void OnConnectDisconnect(serverinfo *info, bool overtime = false);

	void ProcessMsg(serverinfo *info);

private:
	bool AddNewServer(serverinfo *info, int nServerID, int nType);
	serverinfo *FindServer(int nServerID, int nType);

	// 有新的client连接进来
	bool AddNewClient(int servertype, int serverid, int64 clientid);
	ClientSvr *FindClientSvr(int64 clientid);
private:

	std::map<int, serverinfo *> m_GameList;
	std::map<int, serverinfo *> m_LoginList;
	std::map<int, serverinfo *> m_DBList;

	std::unordered_map<int64, ClientSvr> m_ClientSvr;
};

