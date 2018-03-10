#include "PlayerMgr.h"
#include "GameCenterConnect.h"
#include "Config.h"
#include "idmgr.c"
#include "serverlog.h"
#include "objectpool.h"
#include "serverinfo.h"

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
		RunStateError("创建 CPlayer 失败!");
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

CPlayerMgr::CPlayerMgr()
{
	m_PlayerList.clear();
	m_WaitRemove.clear();
	m_PlayerSet.clear();
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
	std::list<CPlayer*>::iterator iter, tempiter;
	for (iter = m_PlayerList.begin(); iter != m_PlayerList.end();)
	{
		tempiter = iter;
		++iter;

		if ((*tempiter)->IsWaitRemove())
		{
			m_WaitRemove.push_back(*tempiter);
			m_PlayerList.erase(tempiter);
			continue;
		}

		(*tempiter)->Run();
	}
}

bool CPlayerMgr::AddPlayer(serverinfo * info, int clientid)
{
	if (clientid <= 0 || clientid >= m_PlayerSet.size())
	{
		RunStateError("AddPlayer的ClientID错误!");
		return false;
	}

	assert(m_PlayerSet[clientid] == nullptr);
	if (m_PlayerSet[clientid] == nullptr)
	{
		CPlayer *newplayer = player_create();
		if (!newplayer)
		{
			RunStateError("创建CPlayer失败!");
			return false;
		}

		newplayer->SetGateInfo(info);
		newplayer->SetClientID(clientid);

		m_PlayerList.push_back(newplayer);
		m_PlayerSet[clientid] = newplayer;
		return true;
	}
	return false;
}

void CPlayerMgr::DelPlayer(int clientid)
{
	if (clientid <= 0 || clientid >= m_PlayerSet.size())
	{
		RunStateError("DelPlayer的ClientID错误!");
		return;
	}

	assert(m_PlayerSet[clientid]);
	if (m_PlayerSet[clientid])
	{
		CPlayer *pPlayer = m_PlayerSet[clientid];
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
	for (std::list<CPlayer *>::iterator itr = m_PlayerList.begin(); itr != m_PlayerList.end(); ++itr)
	{
		(*itr)->OffLine();
		ReleasePlayerAndID(*itr);
	}
	m_PlayerList.clear();

	for (std::list<CPlayer*>::iterator itr = m_WaitRemove.begin(); itr != m_WaitRemove.end(); ++itr)
	{
		ReleasePlayerAndID(*itr);
	}
	m_WaitRemove.clear();
}

void CPlayerMgr::CheckAndRemove()
{
	CPlayer *player;
	while (!m_WaitRemove.empty())
	{
		player = m_WaitRemove.front();
		if (player && !player->CanRemove(g_currenttime))
			break;
		ReleasePlayerAndID(player);
		m_WaitRemove.pop_front();
	}
}

void CPlayerMgr::ReleasePlayerAndID(CPlayer * player)
{
	if (!player)
		return;

	if (player->GetClientID() <= 0 || player->GetClientID() >= static_cast<int>(m_PlayerSet.size()))
	{
		RunStateError("要释放的PlayerInfo的ClientID错误!");
		return;
	}

	m_PlayerSet[player->GetClientID()] = NULL;

	player_release(player);
}

CPlayer *CPlayerMgr::FindPlayerByClientID(int clientid)
{
	assert(m_PlayerSet[clientid]);
	if (m_PlayerSet[clientid])
		return m_PlayerSet[clientid];

	return nullptr;
}

bool CPlayerMgr::InitIdMgrAndPlayerSet()
{
	m_PlayerSet.resize(CLIENT_ID_MAX + 1, NULL);
	return true;
}