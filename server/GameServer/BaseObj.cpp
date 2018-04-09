#include <assert.h>
#include "BaseObj.h"
#include "ServerLog.h"

CBaseObj::CBaseObj(int8 nObjType):m_ObjType(nObjType)
{
	// 根据obj类型设置aoi模式
	if (m_ObjType == EOT_PLAYER)
		SetAoiMode("wm");
	else
		SetAoiMode("m");

	m_ObjSex = 0;
	m_ObjJob = 0;
	m_ObjLevel = 0;
	m_WaitRemoveTime = 0;
	m_DieTime = 0;
	memset(m_ObjName, 0, MAX_NAME_LEN);
}

CBaseObj::~CBaseObj()
{

}

void CBaseObj::Run()
{
	AoiRun();
	StatusRun();
	FightRun();
}
