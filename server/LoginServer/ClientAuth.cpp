#include "LoginCenterConnect.h"
#include "LoginClientMgr.h"
#include "Client.h"
#include "Config.h"
#include "GlobalDefine.h"
#include "msgbase.h"
#include "ClientAuth.h"

#include "osrng.h"
#include "des.h"
#include <string>

#include "MainType.h"
#include "LoginType.h"
#include "DBSvrType.h"
#include "ClientMsg.pb.h"
#include "Login.pb.h"
#include "DBServer.pb.h"

CryptoPP::AutoSeededRandomPool CClientAuth::prng;

CClientAuth::CClientAuth()
{
	m_Secret.clear();
}

CClientAuth::~CClientAuth()
{
	Destroy();
}

void CClientAuth::Destroy()
{
	m_Secret.clear();
}

// 握手
void CClientAuth::HandShake(CClient *cl, Msg *pMsg)
{
	if (!cl || !pMsg)
		return;

	netData::HandShake msg;
	_CHECK_PARSE_(pMsg, msg);
	
	CryptoPP::SecByteBlock Challenge(0x00, CryptoPP::DES::DEFAULT_KEYLENGTH);

	prng.GenerateBlock(Challenge, Challenge.size());

	netData::HandShakeRet sendMsg;
	sendMsg.set_schallenge(reinterpret_cast<const char*>(Challenge.data()), Challenge.size());
	
	// 算出hmac
	AddSecret(cl->GetClientID(), sendMsg.schallenge());
	CLoginClientMgr::Instance().SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_HANDSHAKE_RET);
}

// 挑战
void CClientAuth::Challenge(CClient *cl, Msg *pMsg)
{
	if (!cl || !pMsg)
		return;

	netData::Challenge msg;
	_CHECK_PARSE_(pMsg, msg);

	netData::ChallengeRet sendMsg;
	if (msg.shmac() == GetSecret(cl->GetClientID()))
		sendMsg.set_ncode(netData::ChallengeRet::EC_SUCC);
	else
		sendMsg.set_ncode(netData::ChallengeRet::EC_FAIL);

	CLoginClientMgr::Instance().SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CHALLENGE_RET);
}

// 认证
void CClientAuth::Auth(CClient *cl, Msg *pMsg)
{
	if (!cl || !pMsg)
		return;

	netData::Auth msg;
	_CHECK_PARSE_(pMsg, msg);

	msg.set_ssecret(GetSecret(cl->GetClientID()));
	if (!msg.ssecret().empty())
		CLoginCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), msg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH, cl->GetClientID());
	else
	{
		//要求先握手
		netData::AuthRet sendMsg;
		sendMsg.set_ncode(netData::AuthRet::EC_HANDSHAKE);
		CLoginClientMgr::Instance().SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET);
	}
}

// Client断开连接
void CClientAuth::OnClientDisconnect(CClient *cl)
{
	DelSecret(cl->GetClientID());
}

void CClientAuth::AddSecret(int64 clientid,std::string secret)
{
	m_Secret.insert(std::make_pair(clientid, secret));
}

void CClientAuth::DelSecret(int64 clientid)
{
	m_Secret.erase(clientid);
}

bool CClientAuth::CheckSecret(int64 clientid, std::string &secret)
{
	auto iter = m_Secret.find(clientid);
	if (iter != m_Secret.end())
	{
		return iter->second == secret;
	}
	return false;
}

// 获取Secret
std::string CClientAuth::GetSecret(int64 clientid)
{
	auto iter = m_Secret.find(clientid);
	if (iter != m_Secret.end())
	{
		return iter->second;
	}
	return "";
}
