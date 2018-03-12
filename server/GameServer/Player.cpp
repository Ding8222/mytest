#include "Player.h"
#include "scene.h"
#include "scenemgr.h"
#include "serverlog.h"
#include "msgbase.h"

#include "ServerMsg.pb.h"

CPlayer::CPlayer():CBaseObj(EOT_PLAYER)
{
	m_GateInfo = nullptr;
	m_ClientID = 0;
	m_CenterClientID = 0;
	m_Guid = 0;
	m_CreateTime = 0;
	m_LoginTime = 0;
	memset(m_Data, 0, sizeof(m_Data));
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

	SetCenterClientID(msg.ncenterclientid());
	SetAccount(msg.account());
	SetName(msg.name().c_str());
	SetGuid(msg.guid());
	SetSex(msg.sex());
	SetJob(msg.job());
	SetLevel(msg.level());
	SetCreateTime(msg.createtime());
	SetLoginTime(msg.logintime());
	SetMapID(msg.mapid());
	SetNowPos(msg.x(), msg.y(), msg.z());
	SetData(msg.data().c_str(), msg.data().size());

	CScene *_pScene = CSceneMgr::Instance().FindScene(msg.mapid());
	if (!_pScene)
		return false;
	_pScene->AddObj(this);

	ClientConnectLog("加载玩家数据成功！%s", msg.account().c_str());

	return true;
}

// 保存数据
bool CPlayer::SaveData()
{
	svrData::LoadPlayerData msg;
	PackData(&msg);

	return true;
}

// 打包数据
bool CPlayer::PackData(google::protobuf::Message *pPB)
{
	if (!pPB)
		return false;

	svrData::LoadPlayerData *pMsg = (svrData::LoadPlayerData *)pPB;

	pMsg->set_ncenterclientid(GetClientID());
	pMsg->set_account(GetAccount());
	pMsg->set_name(GetName());
	pMsg->set_guid(GetGuid());
	pMsg->set_sex(GetSex());
	pMsg->set_job(GetJob());
	pMsg->set_level(GetLevel());
	pMsg->set_createtime(GetCreateTime());
	pMsg->set_logintime(GetLoginTime());
	pMsg->set_mapid(GetMapID());
	pMsg->set_x(GetPosX());
	pMsg->set_y(GetPosY());
	pMsg->set_z(GetPosZ());
	pMsg->set_data(GetData());

	return true;
}

// 下线
void CPlayer::OffLine()
{
	LeaveScene();
}