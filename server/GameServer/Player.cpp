#include "Player.h"
#include "scene.h"
#include "scenemgr.h"
#include "serverlog.h"
#include "msgbase.h"
#include "Utilities.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

CPlayer::CPlayer():CBaseObj(EOT_PLAYER)
{
	m_GateInfo = nullptr;
	m_ClientID = 0;
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

	SetAccount(msg.account());
	SetName(msg.name().c_str());
	SetGuid(msg.nguid());
	SetSex(msg.nsex());
	SetJob(msg.njob());
	SetLevel(msg.nlevel());
	SetCreateTime(msg.ncreatetime());
	SetLoginTime(msg.nlogintime());
	SetMapID(msg.nmapid());
	SetNowPos(msg.nx(), msg.ny(), msg.nz());
	SetData(msg.data().c_str(), msg.data().size());

	CScene *_pScene = CSceneMgr::Instance().FindScene(msg.nmapid());
	if (!_pScene)
	{
		RunStateLog("没有找到玩家：%s要登陆的地图：%d", GetName(), msg.nmapid());
		return false;
	}
	if (!_pScene->AddObj(this))
	{
		RunStateLog("添加玩家：%s到地图：%d，失败！", GetName(), msg.nmapid());
		return false;
	}

	RunStateLog("加载玩家：%s数据成功！账号：%s", GetName(), msg.account().c_str());

	if (!msg.bchangeline())
	{
		svrData::AddPlayerToCenter SendMsg;
		SendMsg.set_nguid(GetGuid());
		SendMsg.set_nclientid(GetClientID());
		SendMsg.set_ngateid(GetGateID());
		SendMsg.set_account(GetAccount());
		FuncUti::SendMsgToCenter(this, SendMsg, SERVER_TYPE_MAIN, SVR_SUB_ADD_PLAYER_TO_CENTER);
	}

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

	pMsg->set_account(GetAccount());
	pMsg->set_name(GetName());
	pMsg->set_nguid(GetGuid());
	pMsg->set_nsex(GetSex());
	pMsg->set_njob(GetJob());
	pMsg->set_nlevel(GetLevel());
	pMsg->set_ncreatetime(GetCreateTime());
	pMsg->set_nlogintime(GetLoginTime());
	pMsg->set_nmapid(GetMapID());
	pMsg->set_nx(GetPosX());
	pMsg->set_ny(GetPosY());
	pMsg->set_nz(GetPosZ());
	pMsg->set_data(GetData());

	return true;
}

// 下线
void CPlayer::OffLine()
{
	LeaveScene();
}