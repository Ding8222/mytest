#include "lxnet\base\platform_config.h"
#include "GameLevel.h"
#include "serverlog.h"
#include "RandomPool.h"
#include "objectpool.h"

extern int64 g_currenttime;

CGameLevel::CGameLevel()
{
	m_GateInfo.clear();
	m_MapPosInfo.clear();
	m_Point.resize(32);
}

CGameLevel::~CGameLevel()
{
	Destroy();
}

std::vector<stPoint> CGameLevel::m_Point;

stPoint CGameLevel::GetPos(int32 mapgrid[][m_MapGriden])
{
	m_Point.clear();
	for(auto item : m_MapPosInfo)
	{
		int32 x = item.second->nX;
		int32 y = item.second->nY;

		if (x - 1 >= 0 && mapgrid[x - 1][ y] == 0) m_Point.push_back(stPoint(x - 1, y));

		if (y - 1 >= 0 && mapgrid[x][ y - 1] == 0) m_Point.push_back(stPoint(x, y - 1));

		if (x + 1 < m_GameLevelLen && mapgrid[x + 1][ y] == 0) m_Point.push_back(stPoint(x + 1, y));

		if (y + 1 < m_GameLevelLen && mapgrid[x][ y + 1] == 0) m_Point.push_back(stPoint(x, y + 1));
	}

	int32 rate = CRandomPool::GetOneLess(m_Point.size());
	return m_Point[rate];
}

void CGameLevel::GenGate(int32 mapgrid[][m_MapGriden], int32 x, int32 y)
{
	if (x >= 0 && x < m_GameLevelLen && y >= 0 && y < m_GameLevelLen && mapgrid[x][ y] > 0)
	{
		if (x - 1 >= 0 && mapgrid[x - 1][ y] != 0)
		{
			stGateData* newdata = new stGateData();
			newdata->nGatePos = left;
			newdata->nTargetMapIndex = mapgrid[x - 1][ y];
			newdata->nTargetMapPos = right;

			stGateData* newdata2 = new stGateData();
			newdata2->nGatePos = right;
			newdata2->nTargetMapIndex = mapgrid[x][ y];
			newdata2->nTargetMapPos = left;

			m_GateInfo[mapgrid[x][ y]]->push_back(newdata);
			m_GateInfo[mapgrid[x - 1][ y]]->push_back(newdata2);
		}

		if (y - 1 >= 0 && mapgrid[x][ y - 1] != 0)
		{
			stGateData* newdata = new stGateData();
			newdata->nGatePos = down;
			newdata->nTargetMapIndex = mapgrid[x][ y - 1];
			newdata->nTargetMapPos = up;

			stGateData* newdata2 = new stGateData();
			newdata2->nGatePos = up;
			newdata2->nTargetMapIndex = mapgrid[x][ y];
			newdata2->nTargetMapPos = down;

			m_GateInfo[mapgrid[x][ y]]->push_back(newdata);
			m_GateInfo[mapgrid[x][ y - 1]]->push_back(newdata2);
		}

		if (x + 1 < m_GameLevelLen && mapgrid[x + 1][ y] != 0)
		{
			stGateData* newdata = new stGateData();
			newdata->nGatePos = right;
			newdata->nTargetMapIndex = mapgrid[x + 1][ y];
			newdata->nTargetMapPos = left;

			stGateData* newdata2 = new stGateData();
			newdata2->nGatePos = left;
			newdata2->nTargetMapIndex = mapgrid[x][ y];
			newdata2->nTargetMapPos = right;

			m_GateInfo[mapgrid[x][ y]]->push_back(newdata);
			m_GateInfo[mapgrid[x + 1][ y]]->push_back(newdata2);
		}

		if (y + 1 < m_GameLevelLen && mapgrid[x][ y + 1] != 0)
		{
			stGateData* newdata = new stGateData();
			newdata->nGatePos = up;
			newdata->nTargetMapIndex = mapgrid[x][y + 1];
			newdata->nTargetMapPos = down;

			stGateData* newdata2 = new stGateData();
			newdata2->nGatePos = down;
			newdata2->nTargetMapIndex = mapgrid[x][y];
			newdata2->nTargetMapPos = up;

			m_GateInfo[mapgrid[x][ y]]->push_back(newdata);
			m_GateInfo[mapgrid[x][ y + 1]]->push_back(newdata2);
		}
	}
}

void CGameLevel::GenMap(int32 mapgrid[][m_MapGriden], int32 n, int32 x, int32 y)
{
	if (n <= m_GameLevelLen && x >= 0 && x < m_GameLevelLen&&y >= 0 && y < m_GameLevelLen)
	{
		mapgrid[x][y] = n;
		m_MapPosInfo[n - 1] = new stPoint(x, y);
		if (m_GateInfo.find(n) == m_GateInfo.end())
		{
			std::vector<stGateData *>* list = new std::vector<stGateData *>;
			m_GateInfo.insert(std::make_pair(n, list));
		}

		GenGate(mapgrid, x, y);
		stPoint pos = GetPos(mapgrid);
		GenMap(mapgrid, n + 1, pos.nX, pos.nY);
	}
}

bool CGameLevel::Init(int32 mapgrid[][m_MapGriden])
{
	memset(mapgrid, 0, sizeof(int32) * m_MapGriden * m_MapGriden);
	GenMap(mapgrid, 1, CRandomPool::GetOneLess(m_GameLevelLen), CRandomPool::GetOneLess(m_GameLevelLen));
	return true;
}

void CGameLevel::Destroy()
{
	for (auto i : m_GateInfo)
	{
		std::vector<stGateData *> *temp = i.second;
		for (auto j : *temp)
		{
			delete j;
		}
		temp->clear();
		delete temp;
	}
	m_GateInfo.clear();

	for (auto i : m_MapPosInfo)
	{
		stPoint *temp = i.second;
		delete temp;
	}
	m_MapPosInfo.clear();
	m_Point.clear();
}
