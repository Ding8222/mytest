#include "lxnet\base\platform_config.h"
#include "GameLevelMgr.h"
#include "GameLevel.h"
#include "serverlog.h"
#include "CSVLoad.h"
#include "RandomPool.h"
#include "Timer.h"

// 预生成的地图形状个数
#define LEVEL_LIST_MAX 10

CGameLevelMgr::CGameLevelMgr()
{
	memset(mapgrid, 0, sizeof(mapgrid));
	m_GameLevelList.clear();
}

CGameLevelMgr::~CGameLevelMgr()
{
	Destroy();
}

bool CGameLevelMgr::Init()
{
	CTimer::UpdateTime();
	int64 time1 = CTimer::GetTime64();
	// 预先生成关卡信息
	for (int32 i = 0; i < LEVEL_LIST_MAX; i++)
	{
		CGameLevel *level = new CGameLevel();
		if (level->Init(mapgrid))
		{
			m_GameLevelList.push_back(level);
		}
		else
			delete level;
	}
	CTimer::UpdateTime();
	RunStateLog("初始化生成关卡耗时：%d ms", CTimer::GetTime64() - time1);
	return true;
}

void CGameLevelMgr::Destroy()
{
	for (auto &i : m_GameLevelList)
	{
		delete i;
	}
	m_GameLevelList.clear();
}

bool CGameLevelMgr::InitGameLevel(int32 biglevelid, std::vector<int32> &midlevellist, std::vector<int32> &sublevellist, std::vector<stMapData *> &gamelevelinfo)
{
	midlevellist.clear();
	sublevellist.clear();
	// 大关顺序
	std::unordered_map<int32, int32>* data = CSVData::CGameLevelDB::FindInfoById(biglevelid);
	if (data)
	{
		for (auto &i : *data)
		{
			midlevellist.push_back(i.first);
		}
		if (midlevellist.size() > 0)
		{
			std::random_shuffle(midlevellist.begin(), midlevellist.end());
			// 小关顺序
			int32 sublevelcount = data->find(midlevellist[0])->second;
			if (sublevelcount > 0)
			{
				for (int32 i = 0; i < sublevelcount - 1; i++)
				{
					sublevellist.push_back(i + 1);
				}
				if (sublevellist.size() > 0)
					std::random_shuffle(sublevellist.begin(), sublevellist.end());
				// 最后一关boss关一定在最后
				sublevellist.push_back(sublevelcount);
				// 生成具体关卡
				return GenGameLevel(biglevelid, midlevellist[0], sublevellist[0], gamelevelinfo);
			}
		}
	}
	return false;
}

bool CGameLevelMgr::GenGameLevel(int32 biglevelid, int32 midlevelid, int32 sublevelid, std::vector<stMapData *> &gamelevelinfo)
{
	int32 levelid = biglevelid * 10000 + midlevelid * 100 + sublevelid;
	CSVData::stGameLevel* data = CSVData::CGameLevelDB::FindById(levelid);
	if (data)
	{
		CGameLevel *gameleveldata = m_GameLevelList[CRandomPool::GetOneLess(m_GameLevelList.size())];
		size_t maplistindex = 0;
		std::random_shuffle(data->vMapList.begin(), data->vMapList.end());
		for (size_t i = 0; i < CGameLevel::m_GameLevelLen && 
			i < gameleveldata->m_MapPosInfo.size() &&
			maplistindex < data->vMapList.size(); ++i)
		{
			stMapData *mapdata = new stMapData();
			mapdata->nMapIndex = i;
			mapdata->nMapPosX = gameleveldata->m_MapPosInfo[i]->nX;
			mapdata->nMapPosY = gameleveldata->m_MapPosInfo[i]->nY;
			mapdata->nMapID = data->vMapList[maplistindex];
			mapdata->mGataInfo = gameleveldata->m_GateInfo[i + 1];
			if (maplistindex >= data->vMapList.size())
			{
				maplistindex = 0;
				std::random_shuffle(data->vMapList.begin(), data->vMapList.end());
			}
			gamelevelinfo.push_back(mapdata);
		}
		return true;
	}
	return false;
}
