/*
* Log服务管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "ServerMgr.h"
#include "sqlinterface.h"

class CLogServerMgr :public CServerMgr
{
public:
	CLogServerMgr();
	~CLogServerMgr();

	static CLogServerMgr &Instance()
	{
		static CLogServerMgr m;
		return m;
	}
	bool Init();
	void Destroy();

	void GetCurrentInfo(char *buf, size_t buflen);
	void ResetMsgNum();
	const char *GetMsgNumInfo();

	void SendMsgToServer(Msg &pMsg, int nType, int64 nClientID = 0, int nServerID = 0, bool bBroad = false) {}
	void SendMsgToServer(google::protobuf::Message &pMsg, int maintype, int subtype, int nType, int64 nClientID = 0, int nServerID = 0, bool bBroad = false) {}
	void OnConnectDisconnect(serverinfo *info, bool overtime = false);

	void ProcessMsg(serverinfo *info);

	void ProcessGameMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
	void ProcessLoginMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
	void ProcessDBMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
	void ProcessGateMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
	void ProcessCenterMsg(serverinfo *info, Msg *pMsg, msgtail *tl);
private:
	bool AddNewServer(serverinfo *info, int nServerID, int nType);
	serverinfo *FindServer(int nServerID, int nType);
	void ServerRegisterSucc(int id, int type, const char *ip, int port);

private:

	std::map<int, serverinfo *> m_GameList;
	std::map<int, serverinfo *> m_LoginList;
	std::map<int, serverinfo *> m_DBList;
	std::map<int, serverinfo *> m_GateList;
	std::map<int, serverinfo *> m_CenterList;
};

extern DataBase::CConnection g_dbhand;