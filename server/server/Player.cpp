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

// ��������
bool CPlayer::LoadData()
{
	SetScene(CScenemgr::Instance().GetScene(1));
	GetScene()->AddObj(this);
	SetName("");
	return true;
}

// ��������
bool CPlayer::SaveData()
{
	return true;
}

// ����
void CPlayer::OffLine()
{

}