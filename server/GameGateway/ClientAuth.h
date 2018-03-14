#pragma once
#include <unordered_map>
#include "ServerMsg.pb.h"

struct ClientAuthInfo
{
	ClientAuthInfo()
	{
		ClientID = 0;
		GameServerID = 0;
		Token.clear();
		Secret.clear();
	}
	
	int32 ClientID;
	int32 GameServerID;
	std::string Token;	// 账号
	std::string Secret;	// 秘钥
	svrData::LoadPlayerData Data;//玩家数据
};

class CClient;
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

	void Destroy();

	// 中心服务器通知添加token
	void AddAuthInfo(Msg *pMsg);
	// 更新Client所在GameSvr
	void UpdateGameSvrID(int32 clientid, int32 gameid);
	// 踢下Client
	void KickClient(int32 clientid, bool closeclient = true);
	// client请求登陆
	void QueryLogin(Msg *pMsg, CClient *cl);
	// 返回当前Client数量
	int GetClientSize() { return m_ClientSecretInfo.size(); }
	ClientAuthInfo *FindAuthInfo(int32 clientid);
private:
	// Token,authinfo
	std::unordered_map<std::string, ClientAuthInfo *> m_ClientSecretInfo;
	// clientid,authinfo
	std::unordered_map<int32, ClientAuthInfo *> m_ClientAuthInfo;
};