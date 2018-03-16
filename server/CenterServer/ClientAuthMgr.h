/*
* Client认证管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <vector>
#include <unordered_map>
#include "msgbase.h"

struct ClientAuthInfo
{
	ClientAuthInfo()
	{
		nLoginSvrID = 0;
		Account.clear();
		Secret.clear();
	}
	
	int32 nLoginSvrID;
	std::string Account;	// 账号
	std::string Secret;	// 秘钥
};

class CClientAuthMgr
{
public:
	CClientAuthMgr();
	~CClientAuthMgr();

	static CClientAuthMgr &Instance()
	{
		static CClientAuthMgr m;
		return m;
	}

	bool Init();
	void Run();
	void Destroy();
	void AsLoginServerDisconnect();

	// Client请求认证,loginSvr调用
	void QueryAuth(Msg *pMsg, int32 clientid, int32 serverid);

	void DelClientAuthInfo(int32 clientid);
	ClientAuthInfo *FindClientAuthInfo(int32 clientid);
	int32 GetClientLoginSvr(int32 clientid);

	void SetPlayerOnline(const std::string &account, int64 guid);
	void SetGuid(const std::string &account,int64 guid);
	void SetPlayerOffline(const std::string &account);

	void SetDBSvrReadyStatus(bool bReady) { m_bDBSvrReady = bReady; }
	bool GetIsDBSvrReady() { return m_bDBSvrReady; }
private:
	// id,info
	std::vector<ClientAuthInfo *> m_ClientInfoSet;
	// account,guid
	std::unordered_map<std::string, int64> m_PlayerOnlineMap;
	// account,guid
	std::unordered_map<std::string, int64> m_PlayerLoginMap;

	bool m_bDBSvrReady;
};