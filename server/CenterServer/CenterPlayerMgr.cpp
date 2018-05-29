#include "CenterPlayerMgr.h"
#include "objectpool.h"
#include "ServerLog.h"
#include "GlobalDefine.h"
#include "ClientAuthMgr.h"

CCenterPlayerMgr::CCenterPlayerMgr()
{
	m_PlayerMap.clear();
}

CCenterPlayerMgr::~CCenterPlayerMgr()
{
	Destroy();
}

static objectpool<stCenterPlayer> &CenterPlayerPool()
{
	static objectpool<stCenterPlayer> m(CLIENT_ID_MAX, "stCenterPlayer pools");
	return m;
}

static stCenterPlayer *centerplayer_create()
{
	stCenterPlayer *self = CenterPlayerPool().GetObject();
	if (!self)
	{
		RunStateError("创建 stCenterPlayer 失败!");
		return NULL;
	}
	new(self) stCenterPlayer();
	return self;
}

static void centerplayer_release(stCenterPlayer *self)
{
	if (!self)
		return;
	self->~stCenterPlayer();
	CenterPlayerPool().FreeObject(self);
}

void CCenterPlayerMgr::Destroy()
{
	for (auto &i : m_PlayerMap)
	{
		centerplayer_release(i.second);
	}

	m_PlayerMap.clear();
}

void CCenterPlayerMgr::AddPlayer(int64 guid, const std::string &account, int32 nClientID, int64 nGameID, int32 nGameSvrID, int32 nGateID)
{
	auto iter = m_PlayerMap.find(guid);
	assert(iter == m_PlayerMap.end());
	if(iter == m_PlayerMap.end())
	{
		stCenterPlayer *player = centerplayer_create();
		if(player)
		{
			player->Account = account;
			player->nClientID = nClientID;
			player->nGateID = nGateID;
			player->nGameID = nGameID;
			player->nGameSvrID = nGameSvrID;
			m_PlayerMap.insert(std::make_pair(guid, player));
		}
	}
}

void CCenterPlayerMgr::DelPlayer(int64 guid)
{
	auto iter = m_PlayerMap.find(guid);
	assert(iter != m_PlayerMap.end());
	if (iter != m_PlayerMap.end())
	{
		centerplayer_release(iter->second);
		m_PlayerMap.erase(iter);
	}
}

void CCenterPlayerMgr::UpdatePlayerGameSvr(int64 guid, int32 nGameSvrID)
{
	auto iter = m_PlayerMap.find(guid);
	assert(iter != m_PlayerMap.end());
	if (iter != m_PlayerMap.end())
	{
		iter->second->nGameSvrID = nGameSvrID;
	}
}

void CCenterPlayerMgr::AsGateServerDisconnect(int32 gateserverid)
{
	auto iterB = m_PlayerMap.begin();
	for (; iterB != m_PlayerMap.end();)
	{
		stCenterPlayer *player = iterB->second;
		if (player->nGateID == gateserverid)
		{
			ClientAuthMgr.SetPlayerOffline(player->Account);
			centerplayer_release(player);
			iterB = m_PlayerMap.erase(iterB);
		}
		else
			++iterB;
	}
}

void CCenterPlayerMgr::AsGameServerDisconnect(int32 gameserverid)
{
	auto iterB = m_PlayerMap.begin();
	for (; iterB != m_PlayerMap.end();)
	{
		stCenterPlayer *player = iterB->second;
		if (player->nGameSvrID == gameserverid)
		{
			ClientAuthMgr.SetPlayerOffline(player->Account);
			centerplayer_release(player);
			iterB = m_PlayerMap.erase(iterB);
		}
		else
			++iterB;
	}
}

stCenterPlayer *CCenterPlayerMgr::FindPlayerByGuid(int64 guid)
{
	auto iter = m_PlayerMap.find(guid);
	if (iter != m_PlayerMap.end())
		return iter->second;

	return nullptr;
}
