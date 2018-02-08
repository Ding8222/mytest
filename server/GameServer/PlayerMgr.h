#pragma once
#include <unordered_map>
#include "Player.h"

static int player_max = 2000;
static int player_delay_time = 30000;

struct stPlayerInfo
{
	stPlayerInfo()
	{
		nClientID = 0;
		nGateID = 0;
		nGameServerID = 0;
		pPlayer = nullptr;
	}

	~stPlayerInfo()
	{
		nClientID = 0;
		nGateID = 0;
		nGameServerID = 0;
		pPlayer = nullptr;
	}

	int nClientID;
	int nGateID;
	int nGameServerID;
	CPlayer *pPlayer;
};

class CPlayerMgr
{
public:
	CPlayerMgr();
	~CPlayerMgr();

	static CPlayerMgr &Instance()
	{
		static CPlayerMgr m;
		return m;
	}

	bool init();
	void Destroy();

	void Run();

	void ProcessAllPlayer();

	bool AddPlayer(int clientid, int gateid);
	void DelPlayer(int clientid);
	void DelAllPlayer();
	void ReleasePlayerAndID(stPlayerInfo *player);
	int FindPlayerGateID(int clientid);
	int GetPlayerSize() { return m_PlayerList.size(); }

	CPlayer *FindPlayerByClientID(int clientid);
private:
	void CheckAndRemove();
	bool InitIdMgrAndPlayerSet();
private:

	std::list<stPlayerInfo *> m_PlayerList;
	std::list<stPlayerInfo *> m_WaitRemove;
	std::vector<stPlayerInfo *> m_PlayerInfoSet;
};