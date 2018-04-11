#include "platform_config.h"
#include "Instance.h"
#include "MapConfig.h"
#include "CSVLoad.h"
#include "Timer.h"

CInstance::CInstance()
{
	m_InstanceID = 0;
	m_LimitTime = 0;
	m_CreateTime = 0;
	m_RemoveTime = 0;
}

CInstance::~CInstance()
{
	Destroy();
}

void CInstance::Destroy()
{
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

	CMapInfo* mapconfig = CMapConfig::Instance().FindMapInfo(instanceinfo->nMapID);
	if (!mapconfig || mapconfig->GetMapType() != MapEnum::MapType::EMT_INSTANCE)
	{
		RunStateError("副本地图错误：%d", instanceinfo->nMapID);
		return false;
	}

	m_LimitTime = instanceinfo->nLimitTime;
	m_CreateTime = CTimer::GetTime();

	if (!CScene::Init(mapconfig))
	{
		RunStateError("副本：%d 地图：%d 初始化错误：%d", instancebaseid, instanceinfo->nMapID);
		return false;
	}

	// 初始化刷怪
	std::vector<CSVData::stInstanceMonster *> *monsterset = CSVData::CInstanceMonsterDB::FindById(instancebaseid);
	if (monsterset)
	{
		std::vector<CSVData::stInstanceMonster *>::iterator iter = monsterset->begin();
		for (; iter != monsterset->end(); ++iter)
		{
			CSVData::stInstanceMonster *monster = *iter;
			if (!AddMonster(monster->nMonsterID, monster->nX, monster->nY, monster->nZ, monster->bCanRelive, monster->nReliveCD))
			{
				RunStateError("副本：%d 地图：%d 添加monster：%d 失败！", instancebaseid, instanceinfo->nMapID, monster->nMonsterID);
				return false;
			}
		}
	}

	return true;
}

void CInstance::Run()
{
	CScene::Run();
	if (IsOverTime(CTimer::GetTime()))
	{
		SetWaitRemove();
	}
}