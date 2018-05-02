/*
* 组队管理器Center
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include <set>
#include "platform_config.h"

class CTeam;
struct idmgr;
class CTeamMgr
{
public:
	CTeamMgr();
	~CTeamMgr();
	static CTeamMgr &Instance()
	{
		static CTeamMgr m;
		return m;
	}

	bool Init();
	void Destroy();
	
	// 发送队伍信息
	void SendTeamInfo(int64 playerguid);

	// 请求创建队伍
	void QueryCreateTeam(int64 playerguid);
	// 申请加入队伍
	void QueryApplyTeam(int64 playerguid, int32 teamid);
	// 回应队伍申请
	void ReasponseApplyTeam(int64 playerguid, int64 applyguid, bool agree);
	// 邀请加入队伍
	void QueryInviteTeam(int64 playerguid, int64 inviteguid);
	// 回应队伍邀请
	void ReasponseInviteTeam(int64 playerguid, int64 inviteguid, bool agree);
	// 请求退出队伍
	void QueryExitTeam(int64 playerguid);
	// 请求踢出玩家
	void QueryKickPlayer(int64 playerguid, int64 kickguid);

private:
	// 讲玩家添加到申请列表
	void AddToApplyList(int64 leaderguid, int64 playerguid);
	// 创建一个新队伍，将玩家添加到队伍并设置为队长
	CTeam * NewTeam(const int64 &playerguid);
	// 删除一个队伍
	void ReleaseTeamAndID(CTeam *team);
	// 将玩家添加到队伍
	bool JoinTeam(CTeam *team, int64 playerguid);
	// 退出队伍
	bool ExitTeam(CTeam *team, int64 playerguid);
	// 解散队伍
	bool DissolutionTeam(CTeam *team);
	// 根据teamid查找队伍
	CTeam *FindTeam(int32 teamid);
private:
	// 申请列表 队长id，申请玩家id列表
	std::unordered_map<int64, std::set<int64>> m_ApplyList;
	// 玩家所在队伍
	std::unordered_map<int64, CTeam *> m_PlayerTeam;
	// 所有队伍
	std::vector<CTeam *> m_TeamSet;
	idmgr *m_IDPool;
};