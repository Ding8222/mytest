#include"stdfx.h"
#include "PlayerMgr.h"
#include "GameCenterConnect.h"
#include "Config.h"
#include "idmgr.c"

extern int64 g_currenttime;

static objectpool<CPlayer> &PlayerPool()
{
	static objectpool<CPlayer> m(2000, "CPlayer pools");
	return m;
}

static CPlayer *player_create()
{
	CPlayer *self = PlayerPool().GetObject();
	if (!self)
	{
		log_error("创建 CPlayer 失败!");
		return NULL;
	}
	new(self) CPlayer();
	return self;
}

static void player_release(CPlayer *self)
{
	if (!self)
		return;
	self->~CPlayer();
	PlayerPool().FreeObject(self);
}

static objectpool<stPlayerInfo> &PlayerInfoPool()
{
	static objectpool<stPlayerInfo> m(CLIENT_ID_MAX, "stPlayerInfo pools");
	return m;
}

static stPlayerInfo *playerinfo_create()
{
	stPlayerInfo *self = PlayerInfoPool().GetObject();
	if (!self)
	{
		log_error("创建 stPlayerInfo 失败!");
		return NULL;
	}
	new(self) stPlayerInfo();
	return self;
}

static void playerinfo_release(stPlayerInfo *self)
{
	if (!self)
		return;
	self->~stPlayerInfo();
	PlayerInfoPool().FreeObject(self);
}

CPlayerMgr::CPlayerMgr()
{
	m_PlayerList.clear();
	m_WaitRemove.clear();
	m_PlayerInfoSet.clear();
}

CPlayerMgr::~CPlayerMgr()
{
	Destroy();
}

bool CPlayerMgr::init()
{
	if (!InitIdMgrAndPlayerSet())
		return false;

	return true;
}

void CPlayerMgr::Destroy()
{
	DelAllPlayer();
}

void CPlayerMgr::Run()
{
	ProcessAllPlayer();

	CheckAndRemove();
}

void CPlayerMgr::ProcessAllPlayer()
{
	std::list<stPlayerInfo*>::iterator iter, tempiter;
	for (iter = m_PlayerList.begin(); iter != m_PlayerList.end();)
	{
		tempiter = iter;
		++iter;

		if ((*tempiter)->pPlayer && (*tempiter)->pPlayer->IsWaitRemove())
		{
			m_WaitRemove.push_back(*tempiter);
			m_PlayerList.erase(tempiter);
			continue;
		}

		(*tempiter)->pPlayer->Run();
	}
}

bool CPlayerMgr::AddPlayer(int clientid, int gateid)
{
	assert(m_PlayerInfoSet[clientid] == nullptr);
	if (m_PlayerInfoSet[clientid] == nullptr)
	{
		stPlayerInfo *newInfo = nullptr;
		CPlayer *newplayer = nullptr;

		newInfo = playerinfo_create();
		if (!newInfo)
		{
			log_error("创建stPlayerInfo失败!");
			return false;
		}

		newplayer = player_create();
		if (!newplayer)
		{
			log_error("创建CPlayer失败!");
			playerinfo_release(newInfo);
			return false;
		}

		newInfo->nClientID = clientid;
		newInfo->nGateID = gateid;
		newInfo->nGameServerID = CConfig::Instance().GetServerID();
		newInfo->pPlayer = newplayer;

		m_PlayerList.push_back(newInfo);
		m_PlayerInfoSet[clientid] = newInfo;
		return true;
	}
	return false;
}

void CPlayerMgr::DelPlayer(int clientid)
{
	assert(m_PlayerInfoSet[clientid]);
	if (m_PlayerInfoSet[clientid])
	{
		CPlayer *pPlayer = m_PlayerInfoSet[clientid]->pPlayer;
		assert(pPlayer);
		if (pPlayer)
		{
			assert(!pPlayer->IsWaitRemove());
			if (!pPlayer->IsWaitRemove())
			{
				pPlayer->SetWaitRemove();
				pPlayer->OffLine();
			}
		}
	}
}

void CPlayerMgr::DelAllPlayer()
{
	for (std::list<stPlayerInfo *>::iterator itr = m_PlayerList.begin(); itr != m_PlayerList.end(); ++itr)
	{
		if((*itr)->pPlayer)
			(*itr)->pPlayer->OffLine();
		ReleasePlayerAndID(*itr);
	}
	m_PlayerList.clear();

	for (std::list<stPlayerInfo*>::iterator itr = m_WaitRemove.begin(); itr != m_WaitRemove.end(); ++itr)
	{
		if((*itr) && (*itr)->pPlayer)
		ReleasePlayerAndID(*itr);
	}
	m_WaitRemove.clear();
}

void CPlayerMgr::CheckAndRemove()
{
	stPlayerInfo *playerinfo;
	while (!m_WaitRemove.empty())
	{
		playerinfo = m_WaitRemove.front();
		if (playerinfo && playerinfo->pPlayer && !playerinfo->pPlayer->CanRemove(g_currenttime))
			break;
		ReleasePlayerAndID(playerinfo);
		m_WaitRemove.pop_front();
	}
}

void CPlayerMgr::ReleasePlayerAndID(stPlayerInfo * playerinfo)
{
	if (!playerinfo)
		return;

	if (playerinfo->nClientID <= 0 || playerinfo->nClientID >= static_cast<int>(m_PlayerInfoSet.size()))
	{
		log_error("要释放的PlayerInfo的ClientID错误!");
		return;
	}

	m_PlayerInfoSet[playerinfo->nClientID] = NULL;

	player_release(playerinfo->pPlayer);
	playerinfo_release(playerinfo);
}

int CPlayerMgr::FindPlayerGateID(int clientid)
{
	assert(m_PlayerInfoSet[clientid]);
	if(m_PlayerInfoSet[clientid])
		return m_PlayerInfoSet[clientid]->nGateID;

	return 0;
}

CPlayer *CPlayerMgr::FindPlayerByClientID(int clientid)
{
	assert(m_PlayerInfoSet[clientid]);
	if (m_PlayerInfoSet[clientid])
		return m_PlayerInfoSet[clientid]->pPlayer;

	return nullptr;
}

bool CPlayerMgr::InitIdMgrAndPlayerSet()
{
	m_PlayerInfoSet.resize(CLIENT_ID_MAX + 1, NULL);
	return true;
}