#include <assert.h>
#include "ObjScene.h"
#include "scene.h"
#include "BaseObj.h"
#include "ServerLog.h"
#include "Utilities.h"

#include "ClientType.h"
#include "ClientMsg.pb.h"

CObjScene::CObjScene()
{
	m_AoiMode[MAX_AOIMODE_LEN - 1] = '\0';
	m_NowMapID = 0;
	memset(m_NowPos, 0, sizeof(float) * EOP_MAX);
	m_TempID = 0;
	m_Scene = nullptr;
	m_AoiList.clear();
	m_AoiListOut.clear();
}

CObjScene::~CObjScene()
{
	m_AoiList.clear();
	m_AoiListOut.clear();
}


void CObjScene::SetScene(CScene *_Scene)
{
	m_Scene = _Scene;
	SetMapID(_Scene == nullptr ? 0 : _Scene->GetMapID());
}

// 离开当前场景
bool CObjScene::LeaveScene()
{
	if (m_Scene)
	{
		if (!m_Scene->DelObj(GetObj()))
			return false;
		m_Scene = nullptr;
		return true;
	}
	return false;
}

// 移动到某个坐标
bool CObjScene::MoveTo(float x, float y, float z)
{
	if (m_Scene)
		if (m_Scene->MoveTo(GetObj(), x, y, z))
		{
			netData::PlayerMoveRet sendMsg;
			sendMsg.set_ntempid(GetTempID());
			sendMsg.set_x(x);
			sendMsg.set_y(y);
			sendMsg.set_z(z);

			FuncUti::SendPBNoLoop(GetObj(), sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_MOVE_RET, true);
			return true;
		}

	return false;
}

// 设置当前坐标
void CObjScene::SetNowPos(const Vector3D &pos)
{
	m_vNowPos = pos;
	m_NowPos[EOP_X] = m_vNowPos.x;
	m_NowPos[EOP_Y] = m_vNowPos.y;
	m_NowPos[EOP_Z] = m_vNowPos.z;
}

void CObjScene::SetNowPos(const float &x, const float &y, const float &z)
{
	m_NowPos[EOP_X] = x;
	m_NowPos[EOP_Y] = y;
	m_NowPos[EOP_Z] = z;
	m_vNowPos.x = x;
	m_vNowPos.y = y;
	m_vNowPos.z = z;
}

// 获取当前坐标
void CObjScene::GetNowPos(float &x, float &y, float &z)
{
	x = m_NowPos[EOP_X];
	y = m_NowPos[EOP_Y];
	z = m_NowPos[EOP_Z];
}

// 设置出生点坐标
void CObjScene::SetHomePos(const float &x, const float &y, const float &z)
{
	m_vHomePos.x = x;
	m_vHomePos.y = y;
	m_vHomePos.z = z;
}

// 添加对象至AoiList
void CObjScene::AddToAoiList(CBaseObj *p)
{
#ifdef _DEBUG
	assert(p);
	auto iter = m_AoiList.find(p->GetTempID());
	assert(GetScene()->GetMapID() == p->GetScene()->GetMapID());
	//assert(iter == m_AoiList.end());
#endif
	m_AoiList[p->GetTempID()] = p;
	
	if (GetObj()->IsPlayer())
		GetObj()->UpdataObjInfo(p);

#ifdef _DEBUG
	float _Pos[EOP_MAX] = { 0 };
	p->GetNowPos(_Pos[EOP_X], _Pos[EOP_Y], _Pos[EOP_Z]);
	RunStateLog("[%d]进入[%d]视野,DIST2:%0.01f", p->GetTempID(), GetTempID(), DIST2(_Pos, m_NowPos));
#endif
}

// 从AoiList中移除对象
void CObjScene::DelFromAoiList(uint32 id)
{
#ifdef _DEBUG
	auto iter = m_AoiList.find(id);
	assert(iter != m_AoiList.end());
#endif
	m_AoiList.erase(id);
	GetObj()->DelObjFromView(id);
	//RunStateLog("[%d]离开[%d]视野%d", id, GetTempID());
}

// 添加对象至AoiList
void CObjScene::AddToAoiListOut(CBaseObj *p)
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
void CObjScene::DelFromAoiListOut(uint32 id)
{
#ifdef _DEBUG
	auto iter = m_AoiListOut.find(id);
	assert(iter != m_AoiListOut.end());
#endif
	m_AoiListOut.erase(id);
}

CBaseObj * CObjScene::FindFromAoiList(uint32 id)
{
	auto iter = m_AoiList.find(id);
	if (iter != m_AoiList.end())
	{
		return iter->second;
	}

	return nullptr;
}

// 离开AOI,清理自己和别人的AoiList
void CObjScene::LeaveAoi()
{
	auto iterB = m_AoiList.begin();
	for (; iterB != m_AoiList.end();)
	{
		CBaseObj *_Obj = iterB->second;
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
		CBaseObj *_Obj = iterOutB->second;
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
void CObjScene::AoiRun()
{
	auto iterB = m_AoiList.begin();
	for (; iterB != m_AoiList.end();)
	{
		CBaseObj *_Obj = iterB->second;
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
				_Obj->AddToAoiListOut(GetObj());
				AddToAoiListOut(_Obj);
				GetObj()->DelObjFromView(_Obj->GetTempID());
				continue;
			}
			else if (nDistance > LEAVE_AOI_RADIS2)
			{
				// 需要移除
				iterB = m_AoiList.erase(iterB);
				_Obj->DelFromAoiList(GetTempID());
				GetObj()->DelObjFromView(_Obj->GetTempID());
				continue;
			}
		}
		++iterB;
	}

	auto iterOutB = m_AoiListOut.begin();
	for (; iterOutB != m_AoiListOut.end();)
	{
		CBaseObj *_Obj = iterOutB->second;
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
				_Obj->AddToAoiList(GetObj());
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
