#include "Status.h"
#include "CSVLoad.h"
#include "Timer.h"

CStatus::CStatus()
{
	nID = 0;
	nLev = 0;
	nEndTime = 0;
}

CStatus::~CStatus()
{

}

bool CStatus::Init(int32 id, int32 lev)
{
	int64 key = MAKE_STATUS_KEY(id, lev);
	CSVData::stStatus *statusinfo = CSVData::CStatusDB::FindById(key);
	if (statusinfo)
	{
		nID = id;
		nLev = lev;
		nEndTime = CTimer::GetTime64() + statusinfo->nTime;
		return true;
	}

	return false;
}

void CStatus::Run()
{

}


// 添加状态效果
void CStatus::AddStatusEffect(CBaseObj *obj)
{

}

// 移除状态效果
void CStatus::DelStatusEffect(CBaseObj *obj)
{

}