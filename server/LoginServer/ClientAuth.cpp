#include "LoginCenterConnect.h"
#include "LoginClientMgr.h"
#include "Client.h"
#include "Config.h"
#include "GlobalDefine.h"
#include "msgbase.h"
#include "ClientAuth.h"
#include "objectpool.h"
#include "serverlog.h"

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

static objectpool<ClientAuthInfo> &ClientAuthInfoPool()
{
	static objectpool<ClientAuthInfo> m(CLIENT_ID_MAX, "ClientAuthInfo pools");
	return m;
}

static ClientAuthInfo *clientauthinfo_create()
{
	ClientAuthInfo *self = ClientAuthInfoPool().GetObject();
	if (!self)
	{
		ClientConnectError("创建 ClientAuthInfo 失败!");
		return NULL;
	}
	new(self) ClientAuthInfo();
	return self;
}

static void clientauthinfo_release(ClientAuthInfo *self)
{
	if (!self)
		return;
	self->~ClientAuthInfo();
	ClientAuthInfoPool().FreeObject(self);
}

CClientAuth::CClientAuth()
{
	m_Secret.clear();
	m_ClientAuthInfoSet.clear();
}

CClientAuth::~CClientAuth()
{
	Destroy();
}

bool CClientAuth::Init()
{
	m_ClientAuthInfoSet.resize(CLIENT_ID_MAX + 1, NULL);
	return true;
}

void CClientAuth::Destroy()
{
	m_Secret.clear();
	for (auto &i : m_ClientAuthInfoSet)
	{
		clientauthinfo_release(i);
	}
	m_ClientAuthInfoSet.clear();
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
	
	if (AddSecret(cl->GetClientID(), sendMsg.schallenge()))
		sendMsg.set_ncode(netData::HandShakeRet::EC_SUCC);
	else
		sendMsg.set_ncode(netData::HandShakeRet::EC_FAIL);

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

	const std::string secret = GetSecret(cl->GetClientID());
	if (!secret.empty())
	{
		msg.set_ssecret(secret);
		CLoginCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), msg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH, cl->GetClientID());
	}
	else
	{
		//要求先握手
		netData::AuthRet sendMsg;
		sendMsg.set_ncode(netData::AuthRet::EC_HANDSHAKE);
		CLoginClientMgr::Instance().SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET);
	}
}

// 认证返回
bool CClientAuth::AuthRet(int32 clientid, const std::string &token)
{
	return AddToken(clientid, token);
}

// 请求角色列表
void CClientAuth::GetPlayerList(CClient *cl, Msg *pMsg)
{
	const std::string &token = GetToken(cl->GetClientID());
	if (!token.empty())
	{
		netData::PlayerList sendMsg;
		_CHECK_PARSE_(pMsg, sendMsg);
		sendMsg.set_account(token);

		CLoginCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_PLAYER_LIST, cl->GetClientID());
	}
}

// 请求创建角色
void CClientAuth::CreatePlayer(CClient *cl, Msg *pMsg)
{
	const std::string &token = GetToken(cl->GetClientID());
	if (!token.empty())
	{
		netData::CreatePlayer sendMsg;
		_CHECK_PARSE_(pMsg, sendMsg);
		sendMsg.set_account(token);

		CLoginCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER, cl->GetClientID());
	}
}

// 请求选择角色
void CClientAuth::SelectPlayer(CClient *cl, Msg *pMsg)
{
	const std::string &token = GetToken(cl->GetClientID());
	if (!token.empty())
	{
		CLoginCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), *pMsg, cl->GetClientID());
	}
}

// Client断开连接
void CClientAuth::OnClientDisconnect(CClient *cl)
{
	DelAutoInfo(cl->GetClientID());
}

bool CClientAuth::AddSecret(int32 clientid, const std::string &secret)
{
	ClientAuthInfo *newinfo = clientauthinfo_create();
	if (!newinfo)
	{
		log_error("创建ClientAuthInfo失败!");
		return false;
	}
	newinfo->ClientID = clientid;
	newinfo->Secret = std::move(secret); 

	assert(m_ClientAuthInfoSet[clientid] == nullptr);
	if (m_ClientAuthInfoSet[clientid])
		return false;

	m_ClientAuthInfoSet[clientid] = newinfo;
	return true;
}

bool CClientAuth::AddToken(int32 clientid, const std::string &token)
{
	if (clientid <= 0 || clientid >= m_ClientAuthInfoSet.size())
	{
		ClientConnectError("AddToken的clientid错误!");
		return false;
	}

	assert(m_ClientAuthInfoSet[clientid]);
	if (!m_ClientAuthInfoSet[clientid])
		return false;

	m_ClientAuthInfoSet[clientid]->Token = std::move(token);
	return true;
}

void CClientAuth::DelAutoInfo(int32 clientid)
{
	if (clientid <= 0 || clientid >= m_ClientAuthInfoSet.size())
	{
		ClientConnectError("要释放的ClientAuthInfo的ID错误!");
		return;
	}

	clientauthinfo_release(m_ClientAuthInfoSet[clientid]);
	m_ClientAuthInfoSet[clientid] = nullptr;
}

bool CClientAuth::CheckSecret(int32 clientid, const std::string &secret)
{
	auto iter = m_Secret.find(clientid);
	if (iter != m_Secret.end())
	{
		return iter->second == secret;
	}
	return false;
}

// 获取Secret
std::string CClientAuth::GetSecret(int32 clientid)
{
	if (clientid <= 0 || clientid >= m_ClientAuthInfoSet.size())
	{
		ClientConnectError("GetSecret的clientid错误!");
		return "";
	}

	if (m_ClientAuthInfoSet[clientid])
		return m_ClientAuthInfoSet[clientid]->Secret;

	return "";
}

std::string CClientAuth::GetToken(int32 clientid)
{
	if (clientid <= 0 || clientid >= m_ClientAuthInfoSet.size())
	{
		ClientConnectError("GetToken的clientid错误!");
		return "";
	}

	if (m_ClientAuthInfoSet[clientid])
		return m_ClientAuthInfoSet[clientid]->Token;

	return "";
}
