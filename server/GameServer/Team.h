/*
* 组队
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include "lxnet\base\platform_config.h"

struct TeamPlayer
{
	TeamPlayer(int64 guid, const std::string &name, bool online) :Guid(guid), Name(name), Online(online)
	{

	}

	int64 Guid;
	std::string Name;
	bool Online;
};

class CPlayer;
class CTeam
{
public:
	CTeam();
	~CTeam();

	void InitTeam();
	void DestroyTeam();

	virtual CPlayer *GetPlayer() = 0;

	void SetTeamID(int32 teamid) { m_TeamID = teamid; }
	int32 GetTeamID() { return m_TeamID; }
	void SetTeamLeaderGuid(int64 guid) { m_TeamLeaderGuid = guid; }
	int64 GetTeamLeaderGuid() { return m_TeamLeaderGuid; }
	// 添加成员
	void AddTeamMember(int64 guid, const std::string &name, bool online);
	// 删除成员
	void DelTeamMember(int64 guid);
	// 退出队伍
	void ExitTeam();

private:
	// 队伍ID
	int32 m_TeamID;
	// 队长ID
	int64 m_TeamLeaderGuid;
	// 在线的队友,在本服的
	std::unordered_map<int64, CPlayer *> m_TeamMemberOl;
	// 队友列表
	std::unordered_map<int64, TeamPlayer> m_TeamMember;
};