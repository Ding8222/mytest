#include "stdfx.h"
#include "Client.h"
#include "Config.h"
#include "LoginClientMgr.h"
#include "LoginCenterConnect.h"
#include "osrng.h"
#include "des.h"
#include "dh.h"
#include <string>

#include "ClientType.h"
#include "LoginType.h"
#include "DBSvrType.h"
#include "ClientMsg.pb.h"
#include "Login.pb.h"
#include "DBServer.pb.h"

extern int64 g_currenttime;

CLoginClientMgr::CLoginClientMgr()
{

}

CLoginClientMgr::~CLoginClientMgr()
{

}

int64 CLoginClientMgr::OnNewClient()
{
	if (!CLoginCenterConnect::Instance().IsReady(CConfig::Instance().GetCenterServerID()))
		return 0;

	int64 nClientID = CClientMgr::OnNewClient();
	if (nClientID == 0)
		return 0;

	MessagePack msg;
	msg.SetMainType(SERVER_TYPE_MAIN);
	msg.SetSubType(SVR_SUB_NEW_CLIENT);
	CLoginCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), msg, nClientID);
	return nClientID;
}

void CLoginClientMgr::OnClientDisconnect(CClient *cl)
{
	CClientMgr::OnClientDisconnect(cl);
}

void CLoginClientMgr::ProcessClientMsg(CClient *cl)
{
	Msg *pMsg = NULL;
	for (int i = 0; i < 2; ++i)
	{
		pMsg = cl->GetMsg();
		if (!pMsg)
			break;
		switch (pMsg->GetMainType())
		{
		case CLIENT_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case CLIENT_SUB_PING:
			{
				cl->SendMsg(pMsg);
				cl->SetPingTime(g_currenttime);
				break;
			}
			default:
			{
			}
			}
			break;
		}
		case LOGIN_TYPE_MAIN:
		{
			if (cl->IsAlreadyAuth())
			{
				// 认证通过的
			}
			else
			{
				// 未认证的，进行认证
				switch (pMsg->GetSubType())
				{
				case LOGIN_SUB_HANDSHAKE:
				{
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
					AddClientSecret(cl->GetClientID(),"123");
					SendMsg(cl,sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_HANDSHAKE_RET);
					break;
				}
				case LOGIN_SUB_CHALLENGE:
				{
					netData::Challenge msg;
					_CHECK_PARSE_(pMsg, msg);

					// 验证hmac

					netData::ChallengeRet sendMsg;
					sendMsg.set_ncode(netData::ChallengeRet::EC_SUCC);
					SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CHALLENGE_RET);
					break;
				}
				case LOGIN_SUB_AUTH:
				{
					netData::Auth msg;
					_CHECK_PARSE_(pMsg, msg);
					
					msg.set_ssecret(GetClientSecret(cl->GetClientID()));
					if(!msg.ssecret().empty())
						CLoginCenterConnect::Instance().SendMsgToServer(CConfig::Instance().GetCenterServerID(), msg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH, cl->GetClientID());
					else
					{
						//要求先握手
						netData::AuthRet sendMsg;
						sendMsg.set_ncode(netData::AuthRet::EC_HANDSHAKE);
						SendMsg(cl, sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH_RET);
					}
					break;
				}
				default:
				{

				}
				}
				break;
			}
		}
		default:
		{

		}
		}
	}
}