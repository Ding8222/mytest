#include "lxnet\base\platform_config.h"
#include "PlayerGameLevel.h"
#include "serverlog.h"
#include "RandomPool.h"
#include "GameLevelMgr.h"

extern int64 g_currenttime;

CPlayerGameLevel::CPlayerGameLevel()
{
	m_BigGameLevelID = 0;
	m_MidGameLevelIndex = 0;
	m_SubGameLevelIndex = 0;
	m_MidGameLevelList.clear();
	m_SubGameLevelList.clear();
	m_GameLevelInfo.clear();
	GenGameLevel(1);
}

CPlayerGameLevel::~CPlayerGameLevel()
{
	Destroy();
}

bool CPlayerGameLevel::Init()
{
	return true;
}

void CPlayerGameLevel::Destroy()
{
	m_BigGameLevelID = 0;
	m_MidGameLevelIndex = 0;
	m_SubGameLevelIndex = 0;
	m_MidGameLevelList.clear();
	m_SubGameLevelList.clear();

	for (auto i : m_GameLevelInfo)
	{
		delete i;
	}
	m_GameLevelInfo.clear();
}

bool CPlayerGameLevel::GenGameLevel(int32 biglevelid)
{
	m_BigGameLevelID = biglevelid;
	m_MidGameLevelIndex = 0;
	m_SubGameLevelIndex = 0;
	m_CurrentMapIndex = 0;
	return GameLevelMgr.InitGameLevel(m_BigGameLevelID, m_MidGameLevelList, m_SubGameLevelList, m_GameLevelInfo);
}

bool CPlayerGameLevel::GenNextGameLevel()
{
	m_SubGameLevelIndex++;
	m_CurrentMapIndex = 0;
	if (m_SubGameLevelIndex < (int32)m_SubGameLevelList.size())
	{
		// 当前大关卡没有通关
		return GameLevelMgr.GenGameLevel(m_BigGameLevelID, m_MidGameLevelList[m_MidGameLevelIndex], m_SubGameLevelList[m_SubGameLevelIndex], m_GameLevelInfo);
	}
	else
	{
		m_MidGameLevelIndex++;
		m_SubGameLevelIndex = 0;
		if (m_MidGameLevelIndex < (int32)m_MidGameLevelList.size())
		{
			// 还有大关卡可以进行,生成下一个大关卡
			return GameLevelMgr.GenGameLevel(m_BigGameLevelID, m_MidGameLevelList[m_MidGameLevelIndex], m_SubGameLevelList[m_SubGameLevelIndex], m_GameLevelInfo);
		}
		else
		{
			// m_BigGameLevelID
			m_BigGameLevelID++;
			return GenGameLevel(m_BigGameLevelID);
		}
	}
}