/*
* 玩家管理器
* Copyright (C) ddl
* 2018
*/

#pragma once
#include <unordered_map>
#include "Player.h"

struct idmgr;
static int32 player_max = 2000;

#define PlayerMgr CPlayerMgr::Instance()
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
	void GetCurrentInfo(char *buf, size_t buflen);
	void AsGateServerDisconnect(int32 gateserverid);

	void Run();
	void ProcessAllPlayer();

	bool AddPlayer(serverinfo * info,int32 clientid);
	void DelPlayer(int64 gameid);
	void DelAllPlayer();
	void ReleasePlayer(CPlayer *player);
	int32 GetPlayerSize() { return m_PlayerList.size(); }
	CPlayer *FindPlayerByGameID(int64 gameid);
	std::list<CPlayer *> &GetPlayerList() { return m_PlayerList; }
private:
	void CheckAndRemove();
	bool InitIdMgrAndPlayerSet();
private:

	std::list<CPlayer *> m_PlayerList;
	std::list<CPlayer *> m_WaitRemove;
	std::unordered_map<int64, CPlayer *>m_PlayerMap;
};