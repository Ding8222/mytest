#include "Player.h"
#include "scene.h"
#include "scenemgr.h"
#include <iostream>

CPlayer::CPlayer(CClient* _client)
{
	m_Client = _client;
}

CPlayer::~CPlayer()
{
	m_Client = nullptr;
}

void CPlayer::Run()
{
	CBaseObj::Run();
}

// 加载数据
bool CPlayer::LoadData()
{
	SetScene(CScenemgr::Instance().GetScene(1));
	GetScene()->AddObj(this);
	SetName("");
	return true;
}

// 保存数据
bool CPlayer::SaveData()
{
	return true;
}

// 下线
void CPlayer::OffLine()
{

}