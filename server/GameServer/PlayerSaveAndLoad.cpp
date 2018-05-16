#include "Player.h"
#include "Timer.h"
#include "Utilities.h"
#include "GameGatewayMgr.h"
#include "serverlog.h"
#include "scenemgr.h"
#include "scene.h"
#include "zlib.h"
#include "Base64.h"
#include "RandomPool.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

#define PACK_DATA_OFFSET \
pdata += datalen;\
leftlen -= datalen;\
datalen = leftlen

#define UNPACK_DATA_OFFSET \
pdata += datalen;\
leftlen -= datalen;\
datalen = leftlen

static char g_PackData[16 * 1024];
static char g_CompressData[16 * 1024];
static std::string g_Base64Data;


// 加载数据
bool CPlayer::LoadData(Msg *pMsg)
{
	svrData::LoadPlayerData msg;
	_CHECK_PARSE_(pMsg, msg) false;

	if (!UnPackData(msg.data().c_str(), msg.data().size()))
		return false;

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
	m_LastSaveTime = CTimer::GetTime() + CRandomPool::GetOneLess(60);

	CScene *_pScene = SceneMgr.FindScene(msg.nmapid());
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
		SendMsg.set_ngameid(GetGameID());
		SendMsg.set_nclientid(GetClientID());
		SendMsg.set_ngateid(GetGateID());
		SendMsg.set_account(GetAccount());
		FuncUti::SendMsgToCenter(this, SendMsg, SERVER_TYPE_MAIN, SVR_SUB_ADD_PLAYER_TO_CENTER);
	}
	m_LoadDataSucc = true;
	return true;
}

bool CPlayer::SaveData()
{
	// 没有加载数据的时候，不保存数据
	if (!m_LoadDataSucc)
		return true;

	svrData::LoadPlayerData SendMsg;
	if (SaveData(&SendMsg))
	{
		FuncUti::SendMsgToCenter(this, SendMsg, SERVER_TYPE_MAIN, SVR_SUB_PLAYERDATA);
		return true;
	}
	return false;
}

// 保存数据
bool CPlayer::SaveData(google::protobuf::Message *pMsg)
{
	// 没有加载数据的时候，不保存数据
	if (!m_LoadDataSucc)
		return true;

	if (!pMsg)
		return false;

	if (PackData())
	{
		svrData::LoadPlayerData *data = (svrData::LoadPlayerData *)pMsg;
		m_LastSaveTime = CTimer::GetTime();
		data->set_account(GetAccount());
		data->set_name(GetName());
		data->set_nguid(GetGuid());
		data->set_nsex(GetSex());
		data->set_njob(GetJob());
		data->set_nlevel(GetLevel());
		data->set_ncreatetime(GetCreateTime());
		data->set_nlogintime(GetLoginTime());
		data->set_nmapid(GetMapID());
		data->set_nx(GetNowPosX());
		data->set_ny(GetNowPosY());
		data->set_nz(GetNowPosZ());
		data->set_data(g_Base64Data);
		return true;
	}
	return false;
}

// 打包数据
bool CPlayer::PackData()
{
	// 将玩家数据写到g_PackData
	memset(g_PackData, 0, sizeof(g_PackData));
	char *pdata = g_PackData;
	int32 datalen = sizeof(g_PackData);
	int32 leftlen = datalen;

	///////////////////////////////////////////////////////////////////
	// 保存背包数据
	if (!m_Package.Save(pdata, datalen))
	{
		RunStateError("背包数据保存失败！%s", GetName());
		return false;
	}
	PACK_DATA_OFFSET;
	
	///////////////////////////////////////////////////////////////////
	// 压缩
	memset(g_CompressData, 0, sizeof(g_CompressData));
	int32 nCompressSize = sizeof(g_CompressData);
	Compress(g_PackData, sizeof(g_PackData) - leftlen, g_CompressData, nCompressSize);

	// Base64
	g_Base64Data.clear();
	g_Base64Data.resize(Base64::EncodedLength(nCompressSize));	
	return Base64::Encode(g_CompressData, nCompressSize, &g_Base64Data[0], g_Base64Data.size());
}

// 解析数据
bool CPlayer::UnPackData(const char *data, int32 len)
{
	if (len <= 0)
		return true;

	g_Base64Data.clear();
	g_Base64Data.resize(Base64::DecodedLength(data, len));
	if (!Base64::Decode(data, len, &g_Base64Data[0], g_Base64Data.size())) {
		return false;
	}

	// 先解压
	memset(g_CompressData, 0, sizeof(g_CompressData));
	int32 nCompressSize = sizeof(g_CompressData);
	UnCompress(g_Base64Data.c_str(), g_Base64Data.size(), g_CompressData, nCompressSize);

	const char *pdata = g_CompressData;
	int32 datalen = nCompressSize;
	int32 leftlen = nCompressSize;

	///////////////////////////////////////////////////////////////////
	// 解析背包数据
	if (!m_Package.Load(pdata, datalen))
	{
		RunStateError("背包数据读取失败！%s", GetName());
		return false;
	}
	UNPACK_DATA_OFFSET;

	return true;
}