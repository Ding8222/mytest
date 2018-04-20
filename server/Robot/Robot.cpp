#include "Connector.h"
#include "serverlog.h"
#include "msgbase.h"
#include "GlobalDefine.h"
#include "google/protobuf/message.h"
#include "Robot.h"

#include "ServerType.h"
#include "ServerMsg.pb.h"

extern int64 g_currenttime;

CRobot::CRobot()
{
	sSecret.clear();
	sAccount.clear();
	m_isAuth = false;
	m_isHandShake = false;
	m_TempID = 0;
	bChangeMap = false;
}

CRobot::~CRobot()
{
	Destroy();
}

void CRobot::SendMsg(google::protobuf::Message &pMsg, int8 maintype, int8 subtype, void *adddata, size_t addsize)
{
	MessagePack pk;
	pk.Pack(&pMsg, maintype, subtype);
	connector::SendMsg(&pk, adddata, addsize);
}

void CRobot::OnConnectDisconnect()
{
	if (IsAlreadyRegister())
	{

	}

	ResetConnect();
	SetAlreadyRegister(false);
}

void CRobot::ChangeConnect(const char *ip, int port, int id, bool bauth)
{
	m_isAuth = bauth;
	SetConnectInfo(ip,port,id);
	OnConnectDisconnect();
}

void CRobot::Destroy()
{

}

void CRobot::ProcessRegister(connector *con)
{
	MessagePack *pMsg = (MessagePack *)con->GetMsg();

	//未注册，则一帧只处理一个消息。
	if (pMsg)
	{
		switch (pMsg->GetMainType())
		{
		case SERVER_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case SVR_SUB_PING:
			{
				con->SetRecvPingTime(g_currenttime);
				break;
			}
			case SVR_SUB_SERVER_REGISTER_RET:
			{
				svrData::ServerRegisterRet msg;
				_CHECK_PARSE_(pMsg, msg);

				switch (msg.nretcode())
				{
				case svrData::ServerRegisterRet::EC_SUCC:
				{
					// 认证成功
					con->SetAlreadyRegister(true);
					RunStateLog("注册到远程服务器成功！");
					break;
				}
				case svrData::ServerRegisterRet::EC_SERVER_ID_EXIST:
				{
					// 已存在相同ServerID被注册
					RunStateError("注册到远程服务器失败！已存在相同ServerID被注册，远程服务器ID：[%d] IP:[%s]", GetConnectID(), GetConnectIP());
					exit(-1);
					break;
				}
				case svrData::ServerRegisterRet::EC_TO_CONNECT_ID_NOT_EQUAL:
				{
					// 请求注册的ServerID和远程ServerID不同
					RunStateError("注册到远程服务器失败！请求注册的ServerID和远程ServerID不同，远程服务器ID：[%d] IP:[%s]", GetConnectID(), GetConnectIP());
					exit(-1);
					break;
				}
				}
				break;
			}
			default:
				break;
			}
			break;
		}
		}
	}
}

void CRobot::ProcessMsg(connector *_con)
{
	Msg *pMsg = NULL;
	for (;;)
	{
		pMsg = _con->GetMsg();
		if (!pMsg)
			break;
		switch (pMsg->GetMainType())
		{
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
		default:
		{
		}
		}
	}
}