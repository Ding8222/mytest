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
	void Destroy(bool bLoginDisconnect = false);
	void AsLoginServerDisconnect();

	// Client请求认证,loginSvr调用
	void QueryAuth(Msg *pMsg, int32 clientid, int32 serverid);

	void DelClientAuthInfo(int32 clientid);
	ClientAuthInfo *FindClientAuthInfo(int32 clientid);
	int32 GetClientLoginSvr(int32 clientid);
	void SetCenterClientID(const std::string &account,int32 id);
	void ClientOffline(const std::string &account);
private:
	// id,info
	std::vector<ClientAuthInfo *> m_ClientInfoSet;
	// account,centerclientid
	std::unordered_map<std::string, int32> m_PlayerOnlineMap;
	// account,clientid
	std::unordered_map<std::string, int32> m_PlayerLoginMap;
};