#include "ObjStatus.h"
#include "Timer.h"
#include "CSVLoad.h"
#include "Status.h"
#include "objectpool.h"

static objectpool<CStatus> &StatusPool()
{
	static objectpool<CStatus> m(100000, "CStatus pools");
	return m;
}

static CStatus *status_create()
{
	CStatus *self = StatusPool().GetObject();
	if (!self)
	{
		RunStateError("创建 CStatus 失败!");
		return NULL;
	}
	new(self) CStatus();
	return self;
}

static void status_release(CStatus *self)
{
	if (!self)
		return;
	self->~CStatus();
	StatusPool().FreeObject(self);
}

CObjStatus::CObjStatus()
{
	m_StatusMap.clear();
	m_WaitRemove.clear();
}

CObjStatus::~CObjStatus()
{
	for (auto &i : m_StatusMap)
	{
		status_release(i.second);
	}
	m_StatusMap.clear();

	for (auto &i : m_WaitRemove)
	{
		status_release(i);
	}
	m_WaitRemove.clear();
}

void CObjStatus::StatusRun()
{
	int64 nNowTime = CTimer::GetTime64();
	UpdateStatus(nNowTime);
	CheckAndRemove();
}

bool CObjStatus::AddStatus(int32 id, int32 lev)
{
	CStatus *status = status_create();
	if (status)
	{
		if (status->Init(id, lev))
		{
			status->AddStatusEffect(GetObj());
			m_StatusMap[id] = status;
			return true;
		}
	}
	return false;
}

void CObjStatus::DelStatus(int32 id)
{
	auto iter = m_StatusMap.find(id);
	if (iter != m_StatusMap.end())
	{
		iter->second->DelStatusEffect(GetObj());
		iter->second->SetEndTime(CTimer::GetTime());
	}
}

CStatus *CObjStatus::FindStatus(int32 id)
{
	auto iter = m_StatusMap.find(id);
	if (iter != m_StatusMap.end())
	{
		return iter->second;
	}

	return nullptr;
}

void CObjStatus::UpdateStatus(const int64 &time)
{
	std::unordered_map<int32, CStatus *>::iterator iter = m_StatusMap.begin();
	for (; iter != m_StatusMap.end();)
	{
		CStatus *status = iter->second;
		if (status->isEnd(time))
		{
			status->DelStatusEffect(GetObj());
			iter = m_StatusMap.erase(iter);
			continue;
		}

		status->Run();
		++iter;
	}
}

void CObjStatus::CheckAndRemove()
{
	int64 nowtime = CTimer::GetTime();
	CStatus *status;
	while (!m_WaitRemove.empty())
	{
		status = m_WaitRemove.front();
		if (status && !status->CanRemove(nowtime))
			break;
		status_release(status);
		m_WaitRemove.pop_front();
	}
}
