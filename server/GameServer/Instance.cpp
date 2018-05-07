#include "platform_config.h"
#include "Instance.h"
#include "MapConfig.h"
#include "Timer.h"

CInstance::CInstance()
{
	m_InstanceBaseID = 0;
	m_InstanceID = 0;
	m_InstanceResult = false;
	m_NowWave = 0;
	m_MaxWave = 0;
	m_CreateMonsterDelayTime = 0;
	m_MonsterCount = 0;
	m_LimitTime = 0;
	m_CreateTime = 0;
	m_RemoveTime = 0;
	m_InstanceMonster.clear();
}

CInstance::~CInstance()
{
	Destroy();
}

void CInstance::Destroy()
{
	m_InstanceMonster.clear();
	CScene::Destroy();
}

bool CInstance::Init(int32 instancebaseid)
{
	CSVData::stInstance *instanceinfo = CSVData::CInstanceDB::FindById(instancebaseid);
	if (!instanceinfo)
	{
		RunStateError("没有找到基础副本：%d", instancebaseid);
		return false;
	}

	CMapInfo* mapconfig = MapConfig.FindMapInfo(instanceinfo->nMapID);
	if (!mapconfig || mapconfig->GetMapType() != MapEnum::MapType::EMT_INSTANCE)
	{
		RunStateError("副本地图错误：%d", instanceinfo->nMapID);
		return false;
	}

	m_InstanceBaseID = instancebaseid;
	m_LimitTime = instanceinfo->nLimitTime;
	m_CreateTime = CTimer::GetTime();
	m_MaxWave = instanceinfo->nMaxWave;

	if (!CScene::Init(mapconfig))
	{
		RunStateError("副本：%d 地图：%d 初始化错误：%d", instancebaseid, instanceinfo->nMapID);
		return false;
	}

	// 初始化刷怪
	m_InstanceMonster = *CSVData::CInstanceMonsterDB::FindById(instancebaseid);

	return true;
}

void CInstance::Run()
{
	int64 nowtime = CTimer::GetTime();
	CreateMonsterRun(nowtime);
	CScene::Run();
	if (IsOverTime(nowtime))
	{
		SetWaitRemove();
	}
}

void CInstance::MonsterDie(int32 monsterid)
{
	if (m_MonsterCount == 0)
	{
		if (m_NowWave < m_MaxWave)
			++m_NowWave;
		else
			SetInstanceResult(true);
	}
}

void CInstance::CreateMonsterRun(const int64 &time)
{
	if (time > m_CreateTime + m_CreateMonsterDelayTime)
	{
		if (m_MonsterCount == 0)
		{
			CSVData::stInstanceMonster *monsterinfo;
			while (!m_InstanceMonster.empty())
			{
				monsterinfo = m_InstanceMonster.front();
				if (monsterinfo->nWave == m_NowWave)
				{
					if (AddMonster(monsterinfo->nMonsterID, monsterinfo->nX, monsterinfo->nY, monsterinfo->nZ))
					{
						++m_MonsterCount;
					}
					else
					{
						RunStateError("副本：%d 地图：%d 添加monster：%d 失败！", GetInstanceBaseID(), GetMapID(), monsterinfo->nMonsterID);
					}
					m_InstanceMonster.pop_front();
				}
			}
		}
	}
}
