#pragma once
#include <unordered_map>
#include "osrng.h"

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

	// 握手
	void HandShake(CClient *cl, Msg *pMsg);
	// 挑战
	void Challenge(CClient *cl, Msg *pMsg);
	// 认证
	void Auth(CClient *cl, Msg *pMsg);
	// Client断开连接
	void OnClientDisconnect(CClient *cl);
private:
	// 添加Secret
	void AddSecret(int32 clientid, std::string secret);
	// 删除Secret
	void DelSecret(int32 clientid);
	// 验证Secret是否正确
	bool CheckSecret(int32 clientid, std::string &secret);
	// 获取Secret
	std::string GetSecret(int32 clientid);
private:

	// clientid,Secret
	std::unordered_map<int32, std::string > m_Secret;
	
	static CryptoPP::AutoSeededRandomPool prng;
};