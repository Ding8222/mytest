﻿/*
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
	bool Init(const char *ip, int serverid, int port, int overtime);
	void Destroy();

	void GetCurrentInfo(char *buf, size_t buflen);
	void ResetMsgNum();
	const char *GetMsgNumInfo();

	void SendMsgToServer(Msg &pMsg, int nType, int64 nClientID = 0, int nServerID = 0, bool bBroad = false);
	void SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int nType, int64 nClientID = 0, int nServerID = 0, bool bBroad = false);
	void OnConnectDisconnect(serverinfo *info, bool overtime = false);

	void ProcessMsg(serverinfo *info);

	void ProcessGameMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
	void ProcessLoginMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
	void ProcessDBMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
	void ProcessGateMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
private:
	bool AddNewServer(serverinfo *info, int nServerID, int nType);
	serverinfo *FindServer(int nServerID, int nType);
	void ServerRegisterSucc(int id, int type, const char *ip, int port);

private:

	std::map<int, serverinfo *> m_GameList;
	std::map<int, serverinfo *> m_LoginList;
	std::map<int, serverinfo *> m_DBList;
	std::map<int, serverinfo *> m_GateList;
};

