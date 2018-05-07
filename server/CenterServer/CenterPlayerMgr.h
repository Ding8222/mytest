#pragma once
#include <unordered_map>
#include "platform_config.h"

struct stCenterPlayer
{
	stCenterPlayer()
	{
		nClientID = 0;
		nGateID = 0;
		nGameID = 0;
		Account.clear();
		Name.clear();
	}
	int32 nClientID;
	int32 nGateID;
	int32 nGameID;
	std::string Account;
	std::string Name;
};

#define CenterPlayerMgr CCenterPlayerMgr::Instance()
class CCenterPlayerMgr
{
public:
	CCenterPlayerMgr();
	~CCenterPlayerMgr();

	static CCenterPlayerMgr &Instance()
	{
		static CCenterPlayerMgr m;
		return m;
	}

	void Destroy();

	void AddPlayer(int64 guid, const std::string &account, int32 nClientID, int32 nGameID, int32 nGateID);
	void DelPlayer(int64 guid);
	void UpdatePlayerGameSvr(int64 guid, int32 nGameID);

	void AsGateServerDisconnect(int32 gateserverid);
	void AsGameServerDisconnect(int32 gateserverid);

	stCenterPlayer *FindPlayerByGuid(int64 guid);
private:

	std::unordered_map<int64, stCenterPlayer *> m_PlayerMap;
};