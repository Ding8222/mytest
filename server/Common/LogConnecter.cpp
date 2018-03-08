#include "connector.h"
#include "log.h"
#include "msgbase.h"
#include "LogConnecter.h"

#include "MainType.h"
#include "ServerType.h"
#include "ServerMsg.pb.h"

extern int64 g_currenttime;

CLogConnecter::CLogConnecter()
{
	isReady = false;
	LogServerID = 0;
}

CLogConnecter::~CLogConnecter()
{
	Destroy();
}

bool CLogConnecter::Init(const char *logserverip, int logserverport, int logserverid,
	int serverid, int servertype, int pingtime, int overtime)
{
	if (!CConnectMgr::AddNewConnect(logserverip, logserverport, logserverid))
	{
		log_error("添加Log服务器失败!");
		return false;
	}

	return CConnectMgr::Init(serverid, servertype, pingtime, overtime);
}

void CLogConnecter::Destroy()
{
	CConnectMgr::Destroy();
	isReady = false;
	LogServerID = 0;
}

void CLogConnecter::ServerRegisterSucc(int id, const char *ip, int port)
{
	isReady = true;
	LogServerID = id;
}

void CLogConnecter::ConnectDisconnect(connector *)
{
	isReady = false;
	LogServerID = 0;
}

void CLogConnecter::ProcessMsg(connector *_con)
{
	Msg *pMsg = NULL;
	for (;;)
	{
		pMsg = _con->GetMsg();
		if (!pMsg)
			break;

		msgtail *tl = (msgtail *)(&((char *)pMsg)[pMsg->GetLength() - sizeof(msgtail)]);
		pMsg->SetLength(pMsg->GetLength() - (int)sizeof(msgtail));

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
				break;
			}
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}
}

void CLogConnecter::SendLog()
{
	if (isReady)
	{

	}
}
