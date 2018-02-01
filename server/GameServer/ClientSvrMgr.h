#pragma once
#include <unordered_map>

class CClientSvrMgr
{
public:
	CClientSvrMgr();
	~CClientSvrMgr();

	static CClientSvrMgr &Instance()
	{
		static CClientSvrMgr m;
		return m;
	}

	void AddClientSvr(int64 clientid, int serverid);
	void DelClientSvr(int64 clientid);
	void DelAllClientSvr();
	int FindClientSvr(int64 clientid);
	int GetClientSvrSize() { return m_ClientSvrInfo.size(); }
private:

	// clientid,serverid
	// client所在网关服务器
	std::unordered_map<int64, int> m_ClientSvrInfo;
};