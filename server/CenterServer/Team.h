/*
* 组队Center
* Copyright (C) ddl
* 2018
*/
#pragma once
#include <unordered_map>
#include "lxnet\base\platform_config.h"

struct TeamPlayer
{
	TeamPlayer(int64 guid,const std::string &name):Guid(guid), Name(name)
	{

	}

	int64 Guid;
	std::string Name;
};

class CTeam
{
public:
	CTeam();
	~CTeam();

	bool Init(int32 temid, int64 guid);
	void Destroy();

	int32 GetTeamID() { return m_TeamID; }

	void SetLeader(int64 id);
	int64 GetLeaderGuid() { return m_LeaderGuid; }
	bool AddMember(int64 guid);
	bool DelMember(int64 guid);
	std::unordered_map<int64, TeamPlayer> *GetAllMemeber() { return &m_Member; }
private:
	// 队伍ID
	int32 m_TeamID;
	// 队长guid
	int64 m_LeaderGuid;
	// 成员
	std::unordered_map<int64, TeamPlayer> m_Member;
};
