#include "google/protobuf/message.h"
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
#include "serverlog.h"
#include "msgbase.h"
#include "GlobalDefine.h"
#include "fmt/ostream.h"

#include "MainType.h"
#include "ServerType.h"

#pragma comment(lib,"fmt.lib") 

extern int64 g_currenttime;

CRobotMgr::CRobotMgr()
{
	s_LoginServerIP.clear();
	m_LoginServerPort = 0;
	m_LoginServerID = 0;
	m_RobotList.clear();
	m_OverTime = 0;
	m_PingTime = 0;
}

CRobotMgr::~CRobotMgr()
{
	s_LoginServerIP.clear();
	m_LoginServerPort = 0;
	m_LoginServerID = 0;
	for (auto &i : m_RobotList)
	{
		delete i;
	}
	m_RobotList.clear();
	m_OverTime = 0;
	m_PingTime = 0;
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
		_pRobot->SetConnectInfo(s_LoginServerIP.c_str(), m_LoginServerPort, m_LoginServerID);
		_pRobot->SetAccount(fmt::format("RoBot_{0}", i));
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
			if ((*tempitr)->TryConnect(g_currenttime))
			{
				lxnet::Socketer *co = (*tempitr)->GetCon();
				if (co)
				{
					co->UseUncompress();
					co->UseEncrypt();
					co->UseDecrypt();
					//co->SendTGWInfo(s_LoginServerIP.c_str(), m_LoginServerPort);
				}
				// 请求登陆

				if ((*tempitr)->IsAuth())
				{
					// 逻辑服
					netData::Login sendMsg;
					sendMsg.set_stoken((*tempitr)->GetAccount());
					sendMsg.set_ssecret((*tempitr)->GetSecret());

					(*tempitr)->SendMsg(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_LOGIN);
				}
				else
				{
					// 登录服

					// 生成client key
					CryptoPP::AutoSeededRandomPool prng;
					CryptoPP::SecByteBlock key(0x00, CryptoPP::DES::DEFAULT_KEYLENGTH);
					prng.GenerateBlock(key, key.size());
					(*tempitr)->SetClientKey(key);

					netData::HandShake Msg;
					Msg.set_sclientkey(reinterpret_cast<const char*>(key.data()), key.size());
					(*tempitr)->SendMsg(Msg, LOGIN_TYPE_MAIN, LOGIN_SUB_HANDSHAKE);
					(*tempitr)->SetHandShake(false);
				}
			}
			continue;
		}
		if ((*tempitr)->NeedSendPing(g_currenttime, m_PingTime))
		{
			netData::Ping msg;
			(*tempitr)->SendMsg(msg, CLIENT_TYPE_MAIN, CLIENT_SUB_PING);
		}
		if ((*tempitr)->IsOverTime(g_currenttime, m_OverTime))
		{
			RunStateError("连接远程服务器:[%d] 超时，准备断开重连!", (*tempitr)->GetConnectID());
			(*tempitr)->OnConnectDisconnect();
			continue;
		}
		if ((*tempitr)->IsClose())
		{
			RunStateError("远程服务器断开连接，准备断开重连!", (*tempitr)->GetConnectID());
			(*tempitr)->OnConnectDisconnect();
			continue;
		}

		if ((*tempitr)->IsHandShake())
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

// 				RunStateLog("ServerKey:%s", msg.sserverkey().c_str());
// 				RunStateLog("ChallengeKey:%s", msg.schallenge().c_str());
				con->SetSecret(msg.schallenge());

				netData::Auth sendMsg;
				sendMsg.set_setoken(con->GetAccount());

				con->SendMsg(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH);
				con->SetHandShake(true);
				break;
			}
			}
		}
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
			case CLIENT_SUB_MOVE_RET:
			{
				netData::PlayerMoveRet msg;
				_CHECK_PARSE_(pMsg, msg);

				if (_con->GetChangeMap())
				{
					if (msg.ntempid() == _con->GetTempID())
					{
						netData::PlayerMove sendMsg;
						int x = msg.x();
						int y = msg.y();
						int nRand = rand() % 10000;
						if (nRand > 5000)
							x += 10;
						else
							x -= 10;

						if (x > 1000)
							x = 1000;
						else if (x < 0)
							x = 0;

						nRand = rand() % 10000;
						if (nRand > 5000)
							y += 10;
						else
							y -= 10;

						if (y > 1000)
							y = 1000;
						else if (y < 0)
							y = 0;

						sendMsg.set_x(x);
						sendMsg.set_y(y);
						sendMsg.set_z(0);
						_con->SendMsg(sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_MOVE);
					}
				}
				else
				{
					netData::ChangeMap SendMsg;
					SendMsg.set_nmapid(10);
					_con->SendMsg(SendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_CHANGEMAP);
					_con->SetChangeMap();
				}

				break;
			}
			case CLIENT_SUB_CHANGEMAP_RET:
			{
				netData::ChangeMapRet msg;
				_CHECK_PARSE_(pMsg, msg);

				if (msg.ncode() == netData::ChangeMapRet::EC_SUCC)
				{
					if (msg.bchangeip())
					{
						_con->ChangeConnect(msg.sip().c_str(), msg.nport(), msg.nserverid(), true);
					}
				}
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
			}
			break;
		}
		case LOGIN_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case LOGIN_SUB_LOGIN_RET:
			{
				netData::LoginRet msg;
				_CHECK_PARSE_(pMsg, msg);

				RunStateLog("LoginRet:%d", msg.ncode());

				if (msg.ncode() == netData::LoginRet::EC_SUCC)
				{
					RunStateLog("逻辑服加载数据成功!TempID:%d", msg.ntempid());
					_con->SetTempID(msg.ntempid());
					netData::PlayerMove sendMsg;
					sendMsg.set_x(1);
					sendMsg.set_y(1);
					sendMsg.set_z(0);
					_con->SendMsg(sendMsg, CLIENT_TYPE_MAIN, CLIENT_SUB_MOVE);
				}
				else
				{
					_con->ChangeConnect(s_LoginServerIP.c_str(), m_LoginServerPort, m_LoginServerID, false);
				}
				break;
			}
			case LOGIN_SUB_AUTH_RET:
			{
				netData::AuthRet msg;
				_CHECK_PARSE_(pMsg, msg);

				if (msg.ncode() == netData::AuthRet::EC_SUCC)
				{
					netData::PlayerList sendMsg;
					_con->SendMsg(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_PLAYER_LIST);
				}
				else
				{
					netData::Auth sendMsg;
					sendMsg.set_setoken(_con->GetAccount());
					_con->SendMsg(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_AUTH);
				}
				break;
			}
			case LOGIN_SUB_PLAYER_LIST_RET:
			{
				netData::PlayerListRet msg;
				_CHECK_PARSE_(pMsg, msg);
				RunStateLog("PlayerListRet:%d", msg.list_size());

				if (msg.list_size() == 0)
				{
					//没有角色，创建角色
					netData::CreatePlayer sendMsg;
					sendMsg.set_sname(_con->GetAccount());
					sendMsg.set_njob(1);
					sendMsg.set_nsex(1);

					_con->SendMsg(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER);
				}
				else
				{
					//有角色选择角色
					netData::SelectPlayer sendMsg;
					sendMsg.set_guid(msg.list(0).guid());

					_con->SendMsg(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_SELECT_PLAYER);
				}
				break;
			}
			case LOGIN_SUB_CREATE_PLAYER_RET:
			{
				netData::CreatePlayerRet msg;
				_CHECK_PARSE_(pMsg, msg);
				RunStateLog("CreatePlayerRet:%d", msg.ncode());
				if (msg.ncode() == netData::CreatePlayerRet::EC_SUCC)
				{
					netData::SelectPlayer sendMsg;
					sendMsg.set_guid(msg.info().guid());

					_con->SendMsg(sendMsg, LOGIN_TYPE_MAIN, LOGIN_SUB_SELECT_PLAYER);
				}
				break;
			}
			case LOGIN_SUB_SELECT_PLAYER_RET:
			{
				netData::SelectPlayerRet msg;
				_CHECK_PARSE_(pMsg, msg);

				if(msg.ncode() == netData::SelectPlayerRet::EC_SUCC)
					_con->ChangeConnect(msg.sip().c_str(), msg.nport(), msg.nserverid(), true);
				RunStateLog("SelectPlayerRet:%d", msg.ncode());
				break;
			}
			}
			break;
		}
		}
	}
}