#pragma once
#include <unordered_map>
#include "osrng.h"

struct ClientAuthInfo
{
	ClientAuthInfo()
	{
		SelectSucc = false;
		ClientID = 0;
		Account.clear();
		Secret.clear();
	}

	// 选角是否成功
	bool SelectSucc;
	int32 ClientID;
	std::string Account;	// 账号
	std::string Secret;	// 秘钥
};

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
	void GetCurrentInfo(char *buf, size_t buflen);

	// 握手
	void HandShake(CClient *cl, Msg *pMsg);
	// 挑战
	void Challenge(CClient *cl, Msg *pMsg);
	// 认证
	void Auth(CClient *cl, Msg *pMsg);
	// 将认证成功的账号和clientid绑定
	bool AddAccount(int32 clientid, const std::string &account);
	// 请求角色列表
	void GetPlayerList(CClient *cl, Msg *pMsg);
	// 请求创建角色
	void CreatePlayer(CClient *cl, Msg *pMsg);
	// 请求选择角色
	void SelectPlayer(CClient *cl, Msg *pMsg);
	// 设置选角成功
	void SetSelectPlayerSucc(int32 clientid);
	// 根据clientid获取认证信息
	ClientAuthInfo *GetClientAuthInfo(int32 clientid);
	// Client断开连接
	void OnClientDisconnect(CClient *cl);
private:
	// 添加Secret
	bool AddSecret(int32 clientid, const std::string &secret);
	// 删除认证信息
	void DelAuthInfo(int32 clientid);
	// 验证Secret是否正确
	bool CheckSecret(int32 clientid, const std::string &secret);
	// 获取Secret
	std::string GetSecret(int32 clientid);
	std::string GetAccount(int32 clientid);
private:

	// clientid,Secret
	std::unordered_map<int32, std::string > m_Secret;

	std::vector<ClientAuthInfo *> m_ClientAuthInfoSet;
	static CryptoPP::AutoSeededRandomPool prng;
};