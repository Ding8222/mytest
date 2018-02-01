#include "stdfx.h"
#include "LoginCenterConnect.h"
#include "LoginClientMgr.h"
#include "ClientAuth.h"
#include "Client.h"
#include "Config.h"

#include "osrng.h"
#include "des.h"
#include "dh.h"
#include <string>

#include "LoginType.h"
#include "DBSvrType.h"
#include "ClientMsg.pb.h"
#include "Login.pb.h"
#include "DBServer.pb.h"

CClientAuth::CClientAuth()
{
	m_Secret.clear();
}

CClientAuth::~CClientAuth()
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

	CryptoPP::SecByteBlock ClientKey((const unsigned char*)msg.sclientkey().data(), msg.sclientkey().size());

	CryptoPP::AutoSeededRandomPool prng;
	CryptoPP::SecByteBlock Challenge(0x00, CryptoPP::DES::DEFAULT_KEYLENGTH);
	CryptoPP::SecByteBlock ServerKey(0x00, CryptoPP::DES::DEFAULT_KEYLENGTH);

	prng.GenerateBlock(Challenge, Challenge.size());
	prng.GenerateBlock(ServerKey, ServerKey.size());

	netData::HandShakeRet sendMsg;
	sendMsg.set_schallenge(reinterpret_cast<const char*>(Challenge.data()), Challenge.size());
	sendMsg.set_sserverkey(reinterpret_cast<const char*>(ServerKey.data()), ServerKey.size());

	// 算出hmac
	AddSecret(cl->GetClientID(), "123");
	CLoginClientMgr::Instance().SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_HANDSHAKE_RET);
}

// 挑战
void CClientAuth::Challenge(CClient *cl, Msg *pMsg)
{
	if (!cl || !pMsg)
		return;

	netData::Challenge msg;
	_CHECK_PARSE_(pMsg, msg);

	// 验证hmac

	netData::ChallengeRet sendMsg;
	sendMsg.set_ncode(netData::ChallengeRet::EC_SUCC);
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

void CClientAuth::AddSecret(int64 clientid,std::string secret)
{
	m_Secret.insert(std::make_pair(clientid, secret));
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
