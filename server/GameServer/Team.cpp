#include "Team.h"
#include "PlayerMgr.h"

CTeam::CTeam()
{
	m_TeamID = 0;
	m_TeamLeaderGuid = 0;
	m_TeamMemberOl.clear();
	m_TeamMember.clear();
}

CTeam::~CTeam()
{
	DestroyTeam();
}

void CTeam::InitTeam()
{
	m_TeamID = 0;
	m_TeamMember.clear();
}

void CTeam::DestroyTeam()
{
	m_TeamMember.clear();
}

void CTeam::AddTeamMember(int64 guid, const std::string &name, bool online)
{
	m_TeamMember.insert(std::make_pair(guid,TeamPlayer(guid, name, online)));
	CPlayer *player = CPlayerMgr::Instance().FindPlayerByClientID(guid);
	if (player)
	{
		m_TeamMemberOl.insert(std::make_pair(guid, player));
	}
}

void CTeam::DelTeamMember(int64 guid)
{
	m_TeamMember.erase(guid);
	m_TeamMemberOl.erase(guid);
}

void CTeam::ExitTeam()
{
	if (m_TeamID > 0)
	{
		m_TeamID = 0;
		m_TeamMember.clear();
	}
}
