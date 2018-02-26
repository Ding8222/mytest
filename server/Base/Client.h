/*
* Client对象
* Copyright (C) ddl
* 2018
*/
#pragma once

#include "lxnet.h"
#include "platform_config.h"
#include <string>
struct Msg;
class CClient
{
public:
	CClient();
	~CClient();
	enum
	{
		enum_inlist_state_unknow = 1,
		enum_inlist_state_normal,				//正常列中
		enum_inlist_state_wait_remove,			//等待移除列中

		enum_remove_delay_time = 5000,			//延时* ms再移除
		enum_not_authsucceed_max_time = 60000,	//连接上来，若经过* ms还未认证成功，则移除它
	};
	void SetClientID(int id);
	int GetClientID() { return m_ClientID; }
	void SetCon(lxnet::Socketer *con);
	lxnet::Socketer *GetCon() { return m_con; }
	void SendMsg(Msg *pMsg);
	Msg *GetMsg();
	void SetRemove(int64 currenttime);
	bool IsNeedRemove() { return (m_RemoveTime != 0); }
	bool CanRemove(int64 currenttime);
	void SetPingTime(int64 currenttime) { m_PingTime = currenttime; }
	bool IsOverTime(int64 currenttime, int clientovertime);
	void SetInNormal() { m_InlistState = enum_inlist_state_normal; }
	void SetInWaitRemove() { m_InlistState = enum_inlist_state_wait_remove; }
	bool IsInNormal() { return (enum_inlist_state_normal == m_InlistState); }
	void SetConnectTime(int64 currenttime);
	bool IsEnoughXMNotAlreadyAuth(int64 currenttime);
	void SetNotAuth() { m_AlreadyAuth = false; }
	void SetAlreadyAuth() { m_AlreadyAuth = true; }
	bool IsAlreadyAuth() { return m_AlreadyAuth; }
	void SetNotLogin() { m_AlreadyLogin = false; }
	void SetAlreadyLogin() { m_AlreadyLogin = true; }
	bool IsAlreadyLogin() { return m_AlreadyLogin; }
	void SetInLogicServerID(int64 id) { m_PlayerOnlyID = id; }
	int64 GetInLogicServerID() { return m_PlayerOnlyID; }
	void SetLogicServerID(int serverid) { m_GameServerID = serverid; }
	int GetLogicServer() { return m_GameServerID; }
	void SetSecret(const char * str) { s_Secret = str; }
	std::string GetSecret() { return s_Secret ; }
private:
	bool m_AlreadyLogin;	//登录成功标记
	bool m_AlreadyAuth;		//认证成功标记
	int64 m_ConnectTime;	//何时连接上来
	std::string s_Secret;	//秘钥

	short m_InlistState;
	int m_ClientID;
	int64 m_PingTime;
	int64 m_RemoveTime;
	lxnet::Socketer *m_con;

	int64 m_PlayerOnlyID;//在逻辑服中的角色id
	int m_GameServerID;	//逻辑服的服务器id
};

extern int g_SendToClientNum;
extern int g_RecvFromClientNum;