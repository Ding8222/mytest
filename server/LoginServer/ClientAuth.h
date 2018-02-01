#pragma once
#include <unordered_map>

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

	// 握手
	void HandShake(CClient *cl, Msg *pMsg);
	// 挑战
	void Challenge(CClient *cl, Msg *pMsg);
	// 认证
	void Auth(CClient *cl, Msg *pMsg);

private:
	// 添加Secret
	void AddSecret(int64 clientid, std::string secret);	
	// 验证Secret是否正确
	bool CheckSecret(int64 clientid, std::string &secret);
	// 获取Secret
	std::string GetSecret(int64 clientid);
private:

	// clientid,Secret
	std::unordered_map<int64, std::string > m_Secret;
};