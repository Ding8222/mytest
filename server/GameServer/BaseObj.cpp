#include "baseobj.h"
#include "assert.h"
#include "scene.h"
#include "serverlog.h"

#define AOI_RADIS 200.0f
#define AOI_RADIS2 ((AOI_RADIS) * (AOI_RADIS))
#define LEAVE_AOI_RADIS2 ((AOI_RADIS2) * 4)
#define DIST2(p1,p2) ((p1[EOP_X] - p2[EOP_X]) * (p1[EOP_X] - p2[EOP_X]) + (p1[EOP_Y] - p2[EOP_Y]) * (p1[EOP_Y] - p2[EOP_Y]) + (p1[EOP_Z] - p2[EOP_Z]) * (p1[EOP_Z] - p2[EOP_Z]))

CBaseObj::CBaseObj(int8 nObjType):m_ObjType(nObjType)
{
	// 根据obj类型设置aoi模式
	if (m_ObjType == EOT_PLAYER)
		strncpy_s(m_AoiMode, "wm", MAX_AOIMODE_LEN);
	else
		strncpy_s(m_AoiMode, "m", MAX_AOIMODE_LEN);

	m_ObjSex = 0;
	m_ObjJob = 0;
	m_ObjLevel = 0;
	m_AoiMode[MAX_AOIMODE_LEN - 1] = '\0';
	m_WaitRemoveTime = 0;
	m_NowMapID = 0;
	memset(m_NowPos, 0, sizeof(float) * EOP_MAX);
	m_TempID = 0;
	m_Scene = nullptr;
	memset(m_ObjName, 0, MAX_NAME_LEN);
	m_AoiList.clear();
	m_AoiListOut.clear();
}

CBaseObj::~CBaseObj()
{
	m_AoiList.clear();
	m_AoiListOut.clear();
}

void CBaseObj::Run()
{
	AoiRun();
}

void CBaseObj::SetScene(CScene *_Scene)
{
	m_Scene = _Scene;
	SetMapID(_Scene == nullptr ? 0:_Scene->GetMapID());
}

// 离开当前场景
bool CBaseObj::LeaveScene()
{
	if (m_Scene)
	{
		if (!m_Scene->DelObj(this))
			return false;
		m_Scene = nullptr;
	}
	return false;
}

// 移动到某个坐标
bool CBaseObj::MoveTo(float x, float y, float z)
{
	if (m_Scene)
		return m_Scene->MoveTo(this, x, y, z);

	return false;
}

// 获取当前坐标
void CBaseObj::GetNowPos(float &x, float &y, float &z)
{
	x = m_NowPos[EOP_X];
	y = m_NowPos[EOP_Y];
	z = m_NowPos[EOP_Z];
}

// 设置当前坐标
void CBaseObj::SetNowPos(const float &x, const float &y, const float &z)
{
	m_NowPos[EOP_X] = x;
	m_NowPos[EOP_Y] = y;
	m_NowPos[EOP_Z] = z;
}

// 添加对象至AoiList
void CBaseObj::AddToAoiList(CBaseObj * p)
{
#ifdef _DEBUG
	assert(p);
	auto iter = m_AoiList.find(p->GetTempID());
	assert(GetScene()->GetMapID() == p->GetScene()->GetMapID());
	//assert(iter == m_AoiList.end());
#endif
	m_AoiList[p->GetTempID()] = p;

#ifdef _DEBUG
	float _Pos[EOP_MAX] = { 0 };
	p->GetNowPos(_Pos[EOP_X], _Pos[EOP_Y], _Pos[EOP_Z]);
	RunStateLog("[%d]进入[%d]视野,DIST2:%0.01f", p->GetTempID(), GetTempID(), DIST2(_Pos, m_NowPos));
#endif
}

// 从AoiList中移除对象
void CBaseObj::DelFromAoiList(uint32 id)
{
#ifdef _DEBUG
	auto iter = m_AoiList.find(id);
	assert(iter != m_AoiList.end());
#endif
	m_AoiList.erase(id);
	//RunStateLog("[%d]离开[%d]视野%d", id, GetTempID());
}

// 添加对象至AoiList
void CBaseObj::AddToAoiListOut(CBaseObj * p)
{
#ifdef _DEBUG
	assert(p);
	auto iter = m_AoiListOut.find(p->GetTempID());
	assert(iter == m_AoiListOut.end());
#endif
	m_AoiListOut[p->GetTempID()] = p;

#ifdef _DEBUG
	float _Pos[EOP_MAX] = { 0 };
	p->GetNowPos(_Pos[EOP_X], _Pos[EOP_Y], _Pos[EOP_Z]);
	//RunStateLog("[%d]离开[%d]视野%d", p->GetTempID(), GetTempID(), (int)DIST2(_Pos, m_NowPos));
#endif
}

// 从AoiList中移除对象
void CBaseObj::DelFromAoiListOut(uint32 id)
{
#ifdef _DEBUG
	auto iter = m_AoiListOut.find(id);
	assert(iter != m_AoiListOut.end());
#endif
	m_AoiListOut.erase(id);
}

// 离开AOI,清理自己和别人的AoiList
void CBaseObj::LeaveAoi()
{
	auto iterB = m_AoiList.begin();
	for (; iterB != m_AoiList.end();)
	{
		CBaseObj* _Obj = iterB->second;
		if (_Obj)
		{
			_Obj->DelFromAoiList(GetTempID());
		}
		++iterB;
	}
	m_AoiList.clear();

	auto iterOutB = m_AoiListOut.begin();
	for (; iterOutB != m_AoiListOut.end();)
	{
		CBaseObj* _Obj = iterOutB->second;
		if (_Obj)
		{
			_Obj->DelFromAoiListOut(GetTempID());
		}
		++iterOutB;
	}
	m_AoiListOut.clear();

	if (m_Scene)
	{
		char *mode = "d";
		m_Scene->Update(GetTempID(), mode, m_NowPos);
	}
}

// 计算Obj距离,处理AoiList
void CBaseObj::AoiRun()
{
	auto iterB = m_AoiList.begin();
	for (; iterB!= m_AoiList.end();)
	{
		CBaseObj* _Obj = iterB->second;
		if (_Obj)
		{
			float _Pos[EOP_MAX] = { 0 };
			_Obj->GetNowPos(_Pos[EOP_X], _Pos[EOP_Y], _Pos[EOP_Z]);
			float nDistance = DIST2(_Pos, m_NowPos);
			if (nDistance > AOI_RADIS2 && nDistance <= LEAVE_AOI_RADIS2)
			{
				// 离开视野，移动到m_AoiListOut
				iterB = m_AoiList.erase(iterB);
				_Obj->DelFromAoiList(GetTempID());
				_Obj->AddToAoiListOut(this);
				AddToAoiListOut(_Obj);
				continue;
			}
			else if (nDistance > LEAVE_AOI_RADIS2)
			{
				// 需要移除
				iterB = m_AoiList.erase(iterB);
				_Obj->DelFromAoiList(GetTempID());
				continue;
			}
		}
		++iterB;
	}

	auto iterOutB = m_AoiListOut.begin();
	for (; iterOutB != m_AoiListOut.end();)
	{
		CBaseObj* _Obj = iterOutB->second;
		if (_Obj)
		{
			float _Pos[EOP_MAX] = { 0 };
			_Obj->GetNowPos(_Pos[EOP_X], _Pos[EOP_Y], _Pos[EOP_Z]);
			float nDistance = DIST2(_Pos, m_NowPos);
			if (nDistance <= AOI_RADIS2)
			{
				// 进入视野，移动到m_AoiList
				iterOutB = m_AoiListOut.erase(iterOutB);
				_Obj->DelFromAoiListOut(GetTempID());
				_Obj->AddToAoiList(this);
				AddToAoiList(_Obj);
				continue;
			}
			else if (nDistance > LEAVE_AOI_RADIS2)
			{
				// 需要移除
				iterOutB = m_AoiListOut.erase(iterOutB);
				_Obj->DelFromAoiListOut(GetTempID());
				continue;
			}
		}
		++iterOutB;
	}
}
