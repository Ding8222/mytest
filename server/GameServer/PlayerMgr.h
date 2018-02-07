#pragma once
#include <unordered_map>
#include "Player.h"

static int player_max = 2000;
static int player_delay_time = 30000;

struct idmgr;
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

	void AddPlayer(int64 clientid, int gateid);
	void DelPlayer(int64 clientid);
	void DelAllPlayer();
	void ReleasePlayerAndID(CPlayer *player);
	int FindPlayerGateID(int64 clientid);
	int GetPlayerSize() { return m_PlayerInfo.size(); }

	CPlayer *FindPlayerByClientID(int64 clientid);
	CPlayer *FindPlayerByTempID(uint32 tempid);
private:
	void CheckAndRemove();
	bool InitIdMgrAndPlayerSet();
private:

	// clientid,serverid
	// client所在网关服务器
	std::unordered_map<int64, CPlayer *> m_PlayerInfo;
	std::list<CPlayer *> m_WaitRemove;

	std::vector<CPlayer *> m_PlayerSet;
	idmgr *m_IDPool;
};