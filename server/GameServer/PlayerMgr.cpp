#include "PlayerMgr.h"
#include "GameCenterConnect.h"
#include "Config.h"
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
	m_PlayerMap.clear();
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

void CPlayerMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "当前玩家数量:%d\n", (int32)m_PlayerList.size());
}

void CPlayerMgr::AsGateServerDisconnect(int32 gateserverid)
{
	std::list<CPlayer*>::iterator iter, tempiter;
	for (iter = m_PlayerList.begin(); iter != m_PlayerList.end();)
	{
		tempiter = iter;
		++iter;

		serverinfo *gate = (*tempiter)->GetGateInfo();
		if (gate)
		{
			if (gate->GetServerID() == gateserverid)
			{
				(*tempiter)->OffLine();
			}
		}
	}
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

bool CPlayerMgr::AddPlayer(serverinfo * info, int32 clientid)
{
	int64 gameid = info->GetServerID();
	gameid = gameid << 32 | clientid;
	if (FindPlayerByGameID(gameid))
	{
		RunStateError("添加Player失败!已经存在的GameIDGameID错误:%I64d,ServerID:%d,clientid:%d", gameid, info->GetServerID(), clientid);
		return false;
	}

	CPlayer *newplayer = player_create();
	if (!newplayer)
	{
		RunStateError("创建CPlayer失败!");
		return false;
	}

	newplayer->SetGateInfo(info);
	newplayer->SetClientID(clientid);
	newplayer->SetGameID(gameid);

	m_PlayerList.push_back(newplayer);
	m_PlayerMap.insert(std::make_pair(gameid, newplayer));
	return true;
}

void CPlayerMgr::DelPlayer(int64 gameid)
{
	auto iter = m_PlayerMap.find(gameid);
	assert(iter != m_PlayerMap.end());
	if (iter != m_PlayerMap.end())
	{
		CPlayer *pPlayer = iter->second;
		pPlayer->OffLine();
		m_PlayerMap.erase(iter);
	}
}

void CPlayerMgr::DelAllPlayer()
{
	for (std::list<CPlayer *>::iterator itr = m_PlayerList.begin(); itr != m_PlayerList.end(); ++itr)
	{
		(*itr)->OffLine();
		ReleasePlayer(*itr);
	}
	m_PlayerList.clear();

	for (std::list<CPlayer*>::iterator itr = m_WaitRemove.begin(); itr != m_WaitRemove.end(); ++itr)
	{
		ReleasePlayer(*itr);
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
		ReleasePlayer(player);
		m_WaitRemove.pop_front();
	}
}

void CPlayerMgr::ReleasePlayer(CPlayer * player)
{
	if (!player)
		return;

	int64 gameid = player->GetGameID();
	m_PlayerMap.erase(gameid);
	player_release(player);
}

CPlayer *CPlayerMgr::FindPlayerByGameID(int64 gameid)
{
	auto iter = m_PlayerMap.find(gameid);
	if (iter != m_PlayerMap.end())
	{
		return iter->second;
	}

	return nullptr;
}

bool CPlayerMgr::InitIdMgrAndPlayerSet()
{
	m_PlayerMap.reserve(player_max);
	return true;
}