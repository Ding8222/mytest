#include "connector.h"
#include "log.h"
#include "msgbase.h"
#include "LogConnecter.h"

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

bool CLogConnecter::Init(const char *logserverip, int logserverport, int logserverid, const char *logservername, 
	int serverid, int servertype, const char *servername, int pingtime, int overtime)
{
	if (!CConnectMgr::AddNewConnect(logserverip, logserverport, logserverid, logservername))
	{
		log_error("添加Log服务器失败!");
		return false;
	}

	return CConnectMgr::Init(serverid, servertype, servername, pingtime, overtime);
}

void CLogConnecter::Destroy()
{
	CConnectMgr::Destroy();
	isReady = false;
	LogServerID = 0;
}

void CLogConnecter::ServerRegisterSucc(connector * con)
{
	isReady = true;
	LogServerID = con->GetConnectID();
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
			}
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
