#include "Team.h"
#include "TeamMgr.h"
#include "objectpool.h"
#include "idmgr.h"
#include "ServerLog.h"
#include "CenterPlayerMgr.h"
#include "Utilities.h"

#include "TeamType.h"
#include "Team.pb.h"

#define TEAM_ID_MAX 10000
#define TEAM_ID_DELAY_TIME 30000

static objectpool<CTeam> &TeamPool()
{
	static objectpool<CTeam> m(TEAM_ID_MAX, "CTeam pools");
	return m;
}

static CTeam *team_create()
{
	CTeam *self = TeamPool().GetObject();
	if (!self)
	{
		RunStateError("创建 CTeam 失败!");
		return NULL;
	}
	new(self) CTeam();
	return self;
}

static void team_release(CTeam *self)
{
	if (!self)
		return;
	self->~CTeam();
	TeamPool().FreeObject(self);
}

CTeamMgr::CTeamMgr()
{
	m_PlayerTeam.clear();
	m_TeamSet.clear();
	m_IDPool = nullptr;
}

CTeamMgr::~CTeamMgr()
{
	Destroy();
}

bool CTeamMgr::Init()
{
	m_IDPool = idmgr_create(TEAM_ID_MAX + 1, TEAM_ID_DELAY_TIME);
	if (!m_IDPool)
	{
		log_error("创建IDMgr失败!");
		return false;
	}

	m_TeamSet.resize(TEAM_ID_MAX + 1, nullptr);
	return true;
}

void CTeamMgr::Destroy()
{
	for (auto &i : m_TeamSet)
	{
		ReleaseTeamAndID(i);
	}
	m_TeamSet.clear();
	m_PlayerTeam.clear();

	if (m_IDPool)
	{
		idmgr_release(m_IDPool);
		m_IDPool = nullptr;
	}
}

// 发送队伍信息
void CTeamMgr::SendTeamInfo(int64 playerguid)
{
	auto iter = m_PlayerTeam.find(playerguid);
	if (iter != m_PlayerTeam.end())
	{
		CTeam *team = iter->second;

		netData::TeamInfo SendMsg;
		SendMsg.set_teamid(team->GetTeamID());
		SendMsg.set_leaderguid(team->GetLeaderGuid());
		std::unordered_map<int64, TeamPlayer> *membermap = team->GetAllMemeber();
		for (auto &i : *membermap)
		{
			TeamPlayer &player = i.second;
			netData::TeamMemberInfo *info = SendMsg.add_info();
			if (info)
			{
				info->set_guid(player.Guid);
				info->set_name(player.Name);
				stCenterPlayer *playerinfo = CCenterPlayerMgr::Instance().FindPlayerByGuid(player.Guid);
				info->set_online(playerinfo == nullptr ? false : true);
			}
		}
		FuncUti::SendMsgToGame(playerguid, SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_TEAM_INFO);
	}
}

void CTeamMgr::QueryCreateTeam(int64 playerguid)
{
	netData::CreateTeamRet SendMsg;
	// 检查玩家是否已经在队伍中了
	auto iter = m_PlayerTeam.find(playerguid);
	if (iter == m_PlayerTeam.end())
	{
		CTeam *team = NewTeam(playerguid);
		if (team)
		{
			if (JoinTeam(team, playerguid))
			{
				SendTeamInfo(playerguid);
				SendMsg.set_ncode(netData::CreateTeamRet::EC_SUCC);
			}
			else
				ReleaseTeamAndID(team);
		}
	}
	else
		SendMsg.set_ncode(netData::CreateTeamRet::EC_TEAM);

	FuncUti::SendMsgToGame(playerguid, SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_CREATE_RET);
}

void CTeamMgr::QueryApplyTeam(int64 playerguid, int32 teamid)
{
	netData::ApplyTeamRet SendMsg;
	// 检查玩家是否已经在队伍中了
	auto iter = m_PlayerTeam.find(playerguid);
	if (iter == m_PlayerTeam.end())
	{
		CTeam *team = FindTeam(teamid);
		if (team)
		{
			// 通知队长有新玩家请求加入
			AddToApplyList(team->GetLeaderGuid(), playerguid);
			SendMsg.set_ncode(netData::ApplyTeamRet::EC_TEAM);
		}
		SendMsg.set_ncode(netData::ApplyTeamRet::EC_FIND);
	}
	else
		SendMsg.set_ncode(netData::ApplyTeamRet::EC_TEAM);

	FuncUti::SendMsgToGame(playerguid, SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_APPLY_RET);
}

void CTeamMgr::ReasponseApplyTeam(int64 playerguid, int64 applyguid, bool agree)
{
	// 查找Team
	auto iterT = m_PlayerTeam.find(playerguid);
	if (iterT != m_PlayerTeam.end())
	{
		// 是否队长
		CTeam *team = iterT->second;
		if (team->GetLeaderGuid() == playerguid)
		{
			// 查找申请列表
			auto iterA = m_ApplyList.find(playerguid);
			if (iterA != m_ApplyList.end())
			{
				std::set<int64> &list = iterA->second;
				// 对象是否在申请列表中
				auto iterF = list.find(applyguid);
				if (iterF != list.end())
				{
					// 添加到队伍
					JoinTeam(team, applyguid);
					list.erase(iterF);
				}
			}
		}
	}

	// 通知申请人结果
	netData::ResponseApplyTeamRet SendMsg;
	stCenterPlayer *player = CCenterPlayerMgr::Instance().FindPlayerByGuid(playerguid);
	SendMsg.set_agree(agree);
	if (player)
		SendMsg.set_name(player->Name);
	FuncUti::SendMsgToGame(applyguid, SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_APPLY_RESPONSE_RET);
}

void CTeamMgr::QueryInviteTeam(int64 playerguid, int64 inviteguid)
{
	netData::InviteTeamRet SendMsg;
	// 检查玩家是否已经在队伍中了
	auto iter = m_PlayerTeam.find(inviteguid);
	if (iter == m_PlayerTeam.end())
	{
		auto iterT = m_PlayerTeam.find(playerguid);
		if (iterT != m_PlayerTeam.end())
		{
			// 是否队长
			CTeam *team = iterT->second;
			if (team->GetLeaderGuid() == playerguid)
			{
				// 通知玩家有新的邀请
				stCenterPlayer *player = CCenterPlayerMgr::Instance().FindPlayerByGuid(playerguid);
				if (player)
				{
					netData::InviteTeamTips SendTipsMsg;
					SendTipsMsg.set_playerguid(playerguid);
					SendTipsMsg.set_name(player->Name);
					FuncUti::SendMsgToGame(inviteguid, SendTipsMsg, TEAM_TYPE_MAIN, TEAM_SUB_INVITE_TIPS);
				}
			}
			else
				SendMsg.set_ncode(netData::InviteTeamRet::EC_LEADER);
		}
	}
	else
		SendMsg.set_ncode(netData::InviteTeamRet::EC_TEAM);

	FuncUti::SendMsgToGame(playerguid, SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_INVITE_RET);
}

void CTeamMgr::ReasponseInviteTeam(int64 playerguid, int64 inviteguid, bool agree)
{

	// 查找Team
	auto iterT = m_PlayerTeam.find(inviteguid);
	if (iterT != m_PlayerTeam.end())
	{
		// 是否队长
		CTeam *team = iterT->second;
		if (team->GetLeaderGuid() == inviteguid)
		{
			// 添加到队伍
			JoinTeam(team, playerguid);
		}
	}

	// 通知邀请人结果
	netData::ResponseInviteTeamRet SendMsg;
	stCenterPlayer *player = CCenterPlayerMgr::Instance().FindPlayerByGuid(playerguid);
	SendMsg.set_agree(agree);
	if (player)
		SendMsg.set_name(player->Name);
	FuncUti::SendMsgToGame(inviteguid, SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_INVITE_RESPONSE_RET);
}

void CTeamMgr::QueryExitTeam(int64 playerguid)
{
	netData::ExitTeamRet SendMsg;
	// 检查玩家是否已经在队伍中了
	auto iter = m_PlayerTeam.find(playerguid);
	if (iter != m_PlayerTeam.end())
	{
		CTeam *team = iter->second;
		ExitTeam(team, playerguid);
		SendMsg.set_ncode(netData::ExitTeamRet::EC_SUCC);
	}
	else
		SendMsg.set_ncode(netData::ExitTeamRet::EC_TEAM);

	FuncUti::SendMsgToGame(playerguid, SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_EXIT_RET);
}

void CTeamMgr::QueryKickPlayer(int64 playerguid, int64 kickguid)
{
	auto iter = m_PlayerTeam.find(playerguid);
	if(iter!= m_PlayerTeam.end())
	{
		CTeam *team = iter->second;
		if (team->GetLeaderGuid() == playerguid)
		{
			ExitTeam(team, kickguid);
			// 通知玩家被T
			netData::KickTeamMember SendMsg;
			FuncUti::SendMsgToGame(kickguid, SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_TICK);
		}
	}
}

void CTeamMgr::AddToApplyList(int64 leaderguid, int64 playerguid)
{
	stCenterPlayer *player = CCenterPlayerMgr::Instance().FindPlayerByGuid(playerguid);
	if (player)
	{
		netData::ApplyTeamTips SendTipsMsg;
		SendTipsMsg.set_playerguid(playerguid);
		SendTipsMsg.set_name(player->Name);
		FuncUti::SendMsgToGame(leaderguid, SendTipsMsg, TEAM_TYPE_MAIN, TEAM_SUB_APPLY_TIPS);

	}
	auto iter = m_ApplyList.find(leaderguid);
	if (iter != m_ApplyList.end())
	{
		std::set<int64> &list = iter->second;
		list.insert(playerguid);
	}
	else
	{
		std::set<int64> list;
		list.insert(playerguid);
		m_ApplyList.insert(std::make_pair(leaderguid, list));
	}
}

CTeam *CTeamMgr::NewTeam(const int64 &playerguid)
{
	int32 id = idmgr_allocid(m_IDPool);
	if (id <= 0)
	{
		RunStateError("为新Team分配ID失败!, id:%d", id);
		return nullptr;
	}

	CTeam *newteam = team_create();
	if (!newteam)
	{
		RunStateError("创建Team失败!, PlayerGuid:%I64d", playerguid);
		if (!idmgr_freeid(m_IDPool, id))
			log_error("释放ID失败!, ID:%d", id);
		return nullptr;
	}

	if (!newteam->Init(id, playerguid))
	{
		RunStateError("初始化Team失败!, PlayerGuid:%I64d", playerguid);
		ReleaseTeamAndID(newteam);
		return nullptr;
	}

	return newteam;
}

void CTeamMgr::ReleaseTeamAndID(CTeam *team)
{
	if (!team)
		return;
	int32 id = team->GetTeamID();
	if (id <= 0 || id >= (int32)m_TeamSet.size())
	{
		RunStateError("要释放的Team的ID错误!");
		return;
	}
	m_TeamSet[id] = NULL;

	if (!idmgr_freeid(m_IDPool, id))
	{
		RunStateError("释放ID错误, ID:%d", id);
	}

	team_release(team);
}

bool CTeamMgr::JoinTeam(CTeam *team, int64 playerguid)
{
	if (team)
	{
		if (team->AddMember(playerguid))
		{
			m_PlayerTeam.insert(std::make_pair(playerguid, team));
			return true;
		}
	}
	return false;
}

bool CTeamMgr::ExitTeam(CTeam *team, int64 playerguid)
{
	if (team->DelMember(playerguid))
	{
		m_PlayerTeam.erase(playerguid);
		return true;
	}

	return false;
}

// 解散队伍
bool CTeamMgr::DissolutionTeam(CTeam *team)
{
	std::unordered_map<int64, TeamPlayer> *membermap = team->GetAllMemeber();
	netData::DissolutionTeam SendMsg;
	MessagePack pk;
	pk.Pack(&SendMsg, TEAM_TYPE_MAIN, TEAM_SUB_DISSOLUTION);

	for (auto &i : *membermap)
	{
		TeamPlayer &player = i.second;
		FuncUti::SendMsgToGame(player.Guid, pk);
	}

	auto iter = m_ApplyList.find(team->GetLeaderGuid());
	if (iter != m_ApplyList.end())
		iter->second.clear();

	ReleaseTeamAndID(team);
	return true;
}

CTeam *CTeamMgr::FindTeam(int32 teamid)
{
	int32 id = teamid;
	if (id <= 0 || id >= (int32)m_TeamSet.size())
	{
		RunStateError("要查找的Team的ID错误!");
		return nullptr;
	}

	return m_TeamSet[teamid];
}