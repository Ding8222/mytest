#pragma once
#include <unordered_map>
#include "Player.h"

static int player_max = 2000;
static int player_delay_time = 30000;

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

	bool AddPlayer(serverinfo * info,int clientid);
	void DelPlayer(int clientid);
	void DelAllPlayer();
	void ReleasePlayerAndID(CPlayer *player);
	int GetPlayerSize() { return m_PlayerList.size(); }

	CPlayer *FindPlayerByClientID(int clientid);
private:
	void CheckAndRemove();
	bool InitIdMgrAndPlayerSet();
private:

	std::list<CPlayer *> m_PlayerList;
	std::list<CPlayer *> m_WaitRemove;
	std::vector<CPlayer *> m_PlayerSet;
};