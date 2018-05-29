/*
* Client认证
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include "ServerMsg.pb.h"

struct ClientAuthInfo
{
	ClientAuthInfo()
	{
		ClientID = 0;
		GameServerID = 0;
		Account.clear();
		Secret.clear();
	}
	
	int32 ClientID; // 连接进来的client分配的id
	int32 GameServerID; // 所在game服务器id
	std::string Account; // 账号
	std::string Secret; // 秘钥
	svrData::LoadPlayerData Data; //玩家数据
};

class CClient;
#define ClientAuth CClientAuth::Instance()
class CClientAuth
{
public:
	CClientAuth();
	~CClientAuth();

	static CClientAuth &Instance()
	{
		static CClientAuth m;
		return m;
	}

	bool Init();
	void Destroy();
	void AsLogicServerDisconnect(int logicserverid);

	// 中心服务器通知添加Account
	void AddAccountInfo(Msg *pMsg);
	// 更新Client所在GameSvr
	void UpdateGameSvrID(int32 clientid, int32 gameid);
	// 踢下Client，默认断开的时候需要通知移除client连接，在client断开连接的时候，不能重复断开
	void KickClient(int32 clientid, bool closeclient = true);
	// client请求登陆
	void QueryLogin(Msg *pMsg, CClient *cl);
	// 返回当前Client数量
	int GetClientSize() { return m_ClientSecretInfo.size(); }
	ClientAuthInfo *FindAuthInfo(int32 clientid);
private:
	// account,authinfo
	std::unordered_map<std::string, ClientAuthInfo *> m_ClientSecretInfo;
	// authinfo
	std::vector<ClientAuthInfo *> m_ClientAuthInfo;
};