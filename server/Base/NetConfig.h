/*
* 网络配置
* 
*/
#pragma once

class CNetConfig
{
public:
	CNetConfig ();
	~CNetConfig ();

	static CNetConfig &Instance()
	{
		static CNetConfig m;
		return m;
	}

	bool Init ();

	int GetBigBufSize () { return m_BigBufSize; }
	int GetBigBufNum () { return m_BigBufNum; }
	int GetSmallBufSize () { return m_SmallBufSize; }
	int GetSmallBufNum () { return m_SmallBufNum; }
	int GetListenerNum () { return m_ListenerNum; }
	int GetSocketerNum () { return m_SocketerNum; }
	int GetThreadNum () { return m_ThreadNum; }
private:
	int m_BigBufSize;
	int m_BigBufNum;
	int m_SmallBufSize;
	int m_SmallBufNum;
	int m_ListenerNum;
	int m_SocketerNum;
	int m_ThreadNum;
};
