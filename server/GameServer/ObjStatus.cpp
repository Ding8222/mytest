#include "ObjStatus.h"
#include "Timer.h"
#include "CSVLoad.h"

CObjStatus::CObjStatus()
{
	m_StatusMap.clear();
}

CObjStatus::~CObjStatus()
{
	for (auto &i : m_StatusMap)
	{
		delete i.second;
	}
	m_StatusMap.clear();
}

void CObjStatus::StatusRun()
{
	int64 nNowTime = CTimer::GetTime64();
	UpdateStatus(nNowTime);
}

bool CObjStatus::AddStatus(int32 id, int32 lev)
{
	int64 key = MAKE_STATUS_KEY(id, lev);
	CSVData::stStatus *statusinfo = CSVData::CStatusDB::FindById(key);
	if (statusinfo)
	{
		stStatus *status = new stStatus;
		status->nID = id;
		status->nLev = lev;
		status->nEndTime = CTimer::GetTime64() + 10000;

		m_StatusMap[id] = status;
		return true;
	}

	return false;
}

void CObjStatus::DelStatus(int32 id)
{
	auto iter = m_StatusMap.find(id);
	if (iter != m_StatusMap.end())
	{
		iter->second->SetRemove(CTimer::GetTime());
	}
}

stStatus *CObjStatus::FindStatus(int32 id)
{
	auto iter = m_StatusMap.find(id);
	if (iter != m_StatusMap.end())
	{
		return iter->second;
	}

	return nullptr;
}

void CObjStatus::UpdateStatus(const int64 time)
{
	std::unordered_map<int32, stStatus *>::iterator iter = m_StatusMap.begin();
	for (; iter != m_StatusMap.end();)
	{
		stStatus *status = iter->second;
		if (status->isNeedRemove(time))
		{
			delete status;
			iter = m_StatusMap.erase(iter);
			continue;
		}
		++iter;
	}
}
