#include"stdfx.h"
#include "PlayerMgr.h"
#include "GameCenterConnect.h"
#include "Config.h"
#include "idmgr.c"

extern int64 g_currenttime;

static objectpool<CPlayer> &PlayerPool()
{
	static objectpool<CPlayer> m(player_max, "Player pools");
	return m;
}

static CPlayer *player_create()
{
	CPlayer *self = PlayerPool().GetObject();
	if (!self)
	{
		log_error("创建 CClient 失败!");
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
	m_PlayerInfo.clear();
}

CPlayerMgr::~CPlayerMgr()
{
	m_PlayerInfo.clear();
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
	idmgr_run(m_IDPool);

	ProcessAllPlayer();

	CheckAndRemove();
}

void CPlayerMgr::ProcessAllPlayer()
{
	for (std::unordered_map<int64, CPlayer *>::iterator itr = m_PlayerInfo.begin(); itr != m_PlayerInfo.end(); ++itr)
	{
		itr->second->Run();
	}
}

void CPlayerMgr::AddPlayer(int64 clientid, int gateid)
{
	auto iter = m_PlayerInfo.find(clientid);
	assert(iter == m_PlayerInfo.end());
	if (iter == m_PlayerInfo.end())
	{
		CPlayer *newplayer = nullptr;
		int id = idmgr_allocid(m_IDPool);
		if (id <= 0)
		{
			log_error("为新Player分配ID失败!, id:%d", id);
			return;
		}

		newplayer = player_create();
		if (!newplayer)
		{
			log_error("创建Player失败!");
			if (id > 0)
			{
				if (!idmgr_freeid(m_IDPool, id))
					log_error("释放ID失败!, ID:%d", id);
			}
			return;
		}

		newplayer->SetTempID(id);
		newplayer->SetClientID(clientid);
		newplayer->SetGameServerID(CConfig::Instance().GetServerID());
		newplayer->SetGateID(gateid);

		m_PlayerInfo.insert(std::make_pair(clientid, newplayer));
	}
}

void CPlayerMgr::DelPlayer(int64 clientid)
{
	auto iter = m_PlayerInfo.find(clientid);
	assert(iter != m_PlayerInfo.end());
	if (iter != m_PlayerInfo.end())
	{
		if (!iter->second->IsWaitRemove())
		{
			iter->second->SetWaitRemove();
			iter->second->OffLine();
			m_WaitRemove.push_back(iter->second);
			m_PlayerInfo.erase(clientid);
		}
	}
}

void CPlayerMgr::DelAllPlayer()
{
	for (std::unordered_map<int64, CPlayer *>::iterator itr = m_PlayerInfo.begin(); itr != m_PlayerInfo.end(); ++itr)
	{
		itr->second->OffLine();
		ReleasePlayerAndID(itr->second);
	}
	m_PlayerInfo.clear();

	for (std::list<CPlayer*>::iterator itr = m_WaitRemove.begin(); itr != m_WaitRemove.end(); ++itr)
	{
		(*itr)->OffLine();
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
		if (!player->CanRemove(g_currenttime))
			break;
		ReleasePlayerAndID(player);
		m_WaitRemove.pop_front();
	}
}

void CPlayerMgr::ReleasePlayerAndID(CPlayer * player)
{
	if (!player)
		return;

	int id = player->GetTempID();
	if (id <= 0 || id >= (int)m_PlayerSet.size())
	{
		log_error("要释放的Client的ID错误!");
		return;
	}
	m_PlayerSet[id] = NULL;

	if (!idmgr_freeid(m_IDPool, id))
	{
		log_error("释放ID错误, ID:%d", id);
	}

	player->SetTempID(0);
	player_release(player);
}

int CPlayerMgr::FindPlayerGateID(int64 clientid)
{
	auto iter = m_PlayerInfo.find(clientid);
	assert(iter != m_PlayerInfo.end());
	if (iter != m_PlayerInfo.end())
		return iter->second->GetGateID();
	return 0;
}

CPlayer *CPlayerMgr::FindPlayerByClientID(int64 clientid)
{
	auto iter = m_PlayerInfo.find(clientid);
	assert(iter != m_PlayerInfo.end());
	if (iter != m_PlayerInfo.end())
		return iter->second;

	return nullptr;
}

CPlayer *CPlayerMgr::FindPlayerByTempID(uint32 tempid)
{
	if (tempid > m_PlayerSet.size())
		return nullptr;
	
	return m_PlayerSet[tempid];
}

bool CPlayerMgr::InitIdMgrAndPlayerSet()
{
	m_IDPool = idmgr_create(player_max + 1, player_delay_time);
	if (!m_IDPool)
	{
		log_error("创建IDMgr失败!");
		return false;
	}
	m_PlayerSet.resize(player_max + 1, NULL);
	return true;
}