/*
* 玩家管理器
* Copyright (C) ddl
* 2018
*/

#pragma once
#include <unordered_map>
#include "Player.h"

static int player_max = 2000;

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
	void AsGateServerDisconnect(int gateserverid);

	void Run();
	void ProcessAllPlayer();

	bool AddPlayer(serverinfo * info,int clientid);
	void DelPlayer(int clientid);
	void DelAllPlayer();
	void ReleasePlayer(CPlayer *player);
	int GetPlayerSize() { return m_PlayerList.size(); }
	CPlayer *FindPlayerByClientID(int clientid);
	std::list<CPlayer *> &GetPlayerList() { return m_PlayerList; }
private:
	void CheckAndRemove();
	bool InitIdMgrAndPlayerSet();
private:

	std::list<CPlayer *> m_PlayerList;
	std::list<CPlayer *> m_WaitRemove;
	std::vector<CPlayer *> m_PlayerSet;
};