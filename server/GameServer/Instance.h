/*
* 副本
* Copyright (C) ddl
* 2018
*/
#pragma once
#include "Scene.h"

class CInstance: public CScene
{
public:
	CInstance();
	~CInstance();

	void SetInsranceID(int id) { m_InstanceID = id; }
	int GetInsranceID() { return m_InstanceID; }
	void SetRemoveTime(int64 currenttime) { m_RemoveTime = currenttime; }
	bool IsNeedRemove() { return m_RemoveTime > 0; }
	bool CanRemove(int64 currenttime) { if (!IsNeedRemove()) return false; return currenttime >= m_RemoveTime; }
private:

	int m_InstanceID;
	int64 m_RemoveTime;
};