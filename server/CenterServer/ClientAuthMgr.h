/*
* Client认证管理器
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>

struct ClientAuthInfo
{
	ClientAuthInfo()
	{
		Token.clear();
		Secret.clear();
	}
	
	std::string Token;	// 账号
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

	// Client请求认证,loginSvr调用
	void AddClientAuthInfo(Msg *pMsg, int64 clientid);
	// 移除认证信息
	void DelClientAuthInfo(int64 clientid);
	// 发送认证信息到逻辑服
	void SendAuthInfoToLogic(Msg *pMsg, int64 clientid);
private:
	// id,info
	std::unordered_map<int64, ClientAuthInfo > m_ClientInfo;
};