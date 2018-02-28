#pragma once
#include <unordered_map>

struct ClientAuthInfo
{
	ClientAuthInfo()
	{
		ClientID = 0;
		Token.clear();
		Secret.clear();
	}
	
	int32 ClientID;
	std::string Token;	// 账号
	std::string Secret;	// 秘钥
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
	// 移除Client
	void DelClient(int32 clientid);
	// 尝试踢下Client
	void KickClient(int32 clientid);
	// client请求认证
	void AddNewClient(Msg *pMsg, CClient *cl);
	// 请求角色列表
	void GetPlayerList(Msg *pMsg, CClient *cl);
	// 请求创建角色
	void CreatePlayer(Msg *pMsg, CClient *cl);
	// 请求选择角色
	void SelectPlayer(Msg *pMsg, CClient *cl);
	// client主动退出
	void Offline(int32 clientid);
	// 返回当前Client数量
	int GetClientSize() { return m_ClientSecretInfo.size(); }
private:
	ClientAuthInfo * FindAuthInfo(int32 clientid);
	ClientAuthInfo * FindAuthInfo(std::string token);
private:
	// Token,authinfo
	std::unordered_map<std::string, ClientAuthInfo *> m_ClientSecretInfo;
	// clientid,authinfo
	std::unordered_map<int32, ClientAuthInfo *> m_ClientAuthInfo;
};