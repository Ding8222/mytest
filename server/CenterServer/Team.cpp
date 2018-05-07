#include "Team.h"
#include "CenterPlayerMgr.h"

#define TEAM_MEMBER_MAX 5

CTeam::CTeam()
{
	m_TeamID = 0;
	m_LeaderGuid = 0;
	m_Member.clear();
	m_Member.reserve(TEAM_MEMBER_MAX);
}

CTeam::~CTeam()
{
	Destroy();
}

bool CTeam::Init(int32 temid, int64 guid)
{
	m_TeamID = temid;
	m_LeaderGuid = guid;
	return true;
}

void CTeam::Destroy()
{
	m_Member.clear();
}

void CTeam::SetLeader(int64 guid)
{
	// 检查要设置的对象是否在队伍中
	auto iter = m_Member.find(guid);
	if (iter != m_Member.end())
	{
		m_LeaderGuid = guid;
	}
}

bool CTeam::AddMember(int64 guid)
{
	if (m_Member.size() < TEAM_MEMBER_MAX)
	{
		auto iter = m_Member.find(guid);
		if (iter == m_Member.end())
		{
			stCenterPlayer *player = CenterPlayerMgr.FindPlayerByGuid(guid);
			if (player)
			{
				m_Member.insert(std::make_pair(guid, TeamPlayer(guid, player->Name)));
			}
		}
	}
	return false;
}

bool CTeam::DelMember(int64 guid)
{
	auto iter = m_Member.find(guid);
	if (iter != m_Member.end())
	{
		m_Member.erase(iter);
		return true;
	}
	return false;
}
