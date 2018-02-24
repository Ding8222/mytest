#include "Player.h"
#include "scene.h"
#include "scenemgr.h"
#include "serverlog.h"
#include "msgbase.h"

#include "ServerMsg.pb.h"
CPlayer::CPlayer()
{

}

CPlayer::~CPlayer()
{
}

void CPlayer::Run()
{
	CBaseObj::Run();
}

// 加载数据
bool CPlayer::LoadData(Msg *pMsg)
{
	svrData::LoadPlayerData msg;
	_CHECK_PARSE_(pMsg, msg) false;

	SetName(msg.name().c_str());
	SetNowPos(msg.x(), msg.y(), msg.z());
	CScene *_pScene = CScenemgr::Instance().FindScene(msg.mapid());
	if (!_pScene)
		return false;
	_pScene->AddObj(this);

	ClientConnectLog("加载玩家数据成功！%s", msg.account().c_str());

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
	if(GetScene())
		GetScene()->DelObj(this);
}