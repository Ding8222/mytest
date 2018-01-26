#include "stdfx.h"
#include "Robot.h"
#include "RobotMgr.h"
#include "Login.pb.h"
#include "ClientMsg.pb.h"
#include "LoginType.h"
#include "ClientType.h"
#include "des.h"
#include "osrng.h"
#include "base64.h"
#include <string>

extern int64 g_currenttime;

CRobotMgr::CRobotMgr()
{
	s_LoginServerIP.clear();
	m_LoginServerPort = 0;
	m_LoginServerID = 0;
	m_RobotList.clear();
}

CRobotMgr::~CRobotMgr()
{
	m_RobotList.clear();
}

bool CRobotMgr::Init(const char *ip, int port, int id, int maxrobot, int pingtime, int overtime)
{
	s_LoginServerIP = ip;
	m_LoginServerPort = port;
	m_LoginServerID = id;
	m_PingTime = pingtime;
	m_OverTime = overtime;
	for (int i = 0; i < maxrobot; i++)
	{
		CRobot *_pRobot = new CRobot;
		if (_pRobot->Init(s_LoginServerIP.c_str(), m_LoginServerPort, 
			m_LoginServerID))
			m_RobotList.push_back(_pRobot);
	}

	return true;
}

void CRobotMgr::Run()
{
	std::list<CRobot *>::iterator itr, tempitr;
	for (itr = m_RobotList.begin(); itr != m_RobotList.end();)
	{
		tempitr = itr;
		++itr;
		if (!(*tempitr)->IsAlreadyConnect())
		{
			if ((*tempitr)->TryConnect(g_currenttime, s_LoginServerIP.c_str(), m_LoginServerPort))
			{
				(*tempitr)->GetCon()->UseUncompress();
				(*tempitr)->GetCon()->UseEncrypt();
				(*tempitr)->GetCon()->UseDecrypt();
				//(*tempitr)->GetCon()->SendTGWInfo(s_LoginServerIP.c_str(), m_LoginServerPort);
				// 请求登陆

				std::string s1("hello world");
				CryptoPP::SecByteBlock b1((const unsigned char*)s1.data(), s1.size());

				// 生成client key
				CryptoPP::AutoSeededRandomPool prng;
				CryptoPP::SecByteBlock key(0x00, CryptoPP::DES::DEFAULT_KEYLENGTH);
				prng.GenerateBlock(key, key.size());
				(*tempitr)->SetClientKey(key);

				std::string test(reinterpret_cast<const char*>(key.data()), key.size());

				unsigned char input[CryptoPP::DES::BLOCKSIZE] = "12345";
				unsigned char output[CryptoPP::DES::BLOCKSIZE];
				unsigned char txt[CryptoPP::DES::BLOCKSIZE];

				CryptoPP::DESEncryption encryption_DES;
				encryption_DES.SetKey(key, CryptoPP::DES::KEYLENGTH);
				encryption_DES.ProcessBlock(input, output);

				CryptoPP::DESDecryption decryption_DES;
				decryption_DES.SetKey(key, CryptoPP::DES::KEYLENGTH);
				decryption_DES.ProcessBlock(output, txt);

				netData::HandShake Msg;
				std::string temp(reinterpret_cast<const char*>(key.data()), key.size());

				Msg.set_sclientkey(temp);
				(*tempitr)->SendMsg(Msg, LOGIN_TYPE_MAIN, LOGIN_SUB_HANDSHAKE);
			}
			return;
		}
		if ((*tempitr)->NeedSendPing(g_currenttime, m_PingTime))
		{
			netData::Ping msg;
			(*tempitr)->SendMsg(msg, CLIENT_TYPE_MAIN, CLIENT_SUB_PING);
		}
		if ((*tempitr)->IsOverTime(g_currenttime, m_OverTime))
		{
			log_error("连接远程服务器:[%d] 超时，准备断开重连!", m_LoginServerID);
			(*tempitr)->OnConnectDisconnect();
			return;
		}
		if ((*tempitr)->IsClose())
		{
			log_error("远程服务器断开连接，准备断开重连!", m_LoginServerID);
			(*tempitr)->OnConnectDisconnect();
			return;
		}

		if ((*tempitr)->IsReady())
			ProcessMsg((*tempitr));
		else
			ProcessRegister((*tempitr));
	}
}

void CRobotMgr::EndRun()
{
	for (auto &i : m_RobotList)
	{
		i->CheckSend();
		i->CheckRecv();
	}
}

void CRobotMgr::Destroy()
{
	for (auto &i : m_RobotList)
	{
		delete i;
	}

	m_RobotList.clear();
}

void CRobotMgr::ProcessRegister(CRobot *con)
{
	MessagePack *pMsg = (MessagePack *)con->GetMsg();

	//未注册，则一帧只处理一个消息。
	if (pMsg)
	{
		switch (pMsg->GetMainType())
		{
		case CLIENT_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case CLIENT_SUB_PING:
			{
				con->SetRecvPingTime(g_currenttime);
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
			switch (pMsg->GetSubType())
			{
			case LOGIN_SUB_HANDSHAKE_RET:
			{
				netData::HandShakeRet msg;
				_CHECK_PARSE_(pMsg, msg);

				log_error("ServerKey:%s", msg.sserverkey().c_str());
				log_error("ChallengeKey:%s", msg.schallenge().c_str());

				netData::Auth sendMsg;
				sendMsg.set_setoken("123");

				con->SendMsg(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH);
				con->SetReady(true);
				break;
			}
			default:
				break;
			}
		}
		default:
			break;
		}
	}
}

void CRobotMgr::ProcessMsg(CRobot *_con)
{
	Msg *pMsg = NULL;
	for (;;)
	{
		pMsg = _con->GetMsg();
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
				_con->SetRecvPingTime(g_currenttime);
				break;
			}
			default:
			{
			}
			}
			break;
		}
		case SERVER_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				_con->SetRecvPingTime(g_currenttime);
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
			switch (pMsg->GetSubType())
			{
			case LOGIN_SUB_AUTH_RET:
			{
				netData::AuthRet msg;
				_CHECK_PARSE_(pMsg, msg);

				log_error("AuthRet:%d", msg.ncode());
				break;
			}
			default:
			{
			}
			}
			break;
		}
		default:
		{
		}
		}
	}
}