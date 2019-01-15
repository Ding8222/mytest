#include "PlayerBase.h"

CPlayerBase::CPlayerBase()
{
	m_GateInfo = nullptr;
	m_ClientID = 0;
	m_GameID = 0;
	m_Guid = 0;
	m_CreateTime = 0;
	m_LoginTime = 0;
	m_LastSaveTime = 0;
	m_LoadDataSucc = false;
}

CPlayerBase::~CPlayerBase()
{

}
