#include <map>
#include "serverinfo.h"
#include "serverlog.h"
#include "GlobalDefine.h"
#include "NameSet.h"

#include "ServerType.h"
#include "LoginType.h"
#include "Login.pb.h"

#include "sqlinterface.h"
#include "NameCheckServerMgr.h"
#include "Config.h"

extern int64 g_currenttime;

CNameCheckServerMgr::CNameCheckServerMgr()
{
	m_CenterList.clear();
}

CNameCheckServerMgr::~CNameCheckServerMgr()
{
	Destroy();
}

bool CNameCheckServerMgr::Init(const char *ip, int serverid, int port, int overtime)
{
	DataBase::CConnection dbhand;
	dbhand.SetLogDirectory("log_log/CheckNameServer_Log/dbhand_log");

	const char *tmp;
	DataBase::CRecordset *res;
	int num = 0;
	std::list<DBInfo *> temp = Config.GetTableList();
	for (std::list<DBInfo *>::iterator itr = temp.begin(); itr != temp.end(); ++itr)
	{
		if (!dbhand.Open((*itr)->dbname.c_str(), (*itr)->dbusername.c_str(), (*itr)->dbpassword.c_str(), (*itr)->dbip.c_str()))
		{
			RunStateError("连接数据库失败! DBName:%s, IP:%s, 条目:%d", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num);
			return false;
		}

		if (!dbhand.SetCharacterSet("utf8"))
		{
			RunStateError("设置表playerdate为utf8模式失败! DBName:%s, IP:%s, 条目:%d", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num);
			return false;
		}

		res = dbhand.Execute((*itr)->sqlstr.c_str());
		if (!res || !res->IsOpen())
		{
			RunStateError("SQL语句查询失败! DBName:%s, IP:%s, 条目:%d", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num);
			return false;
		}

		while (!res->IsEnd())
		{
			tmp = res->Get((*itr)->fieldname.c_str());
			if (!tmp)
			{
				RunStateError("获取字段失败! DBName:%s, IP:%s, 条目:%d", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num);
				return false;
			}
			
			if (!NameSet.AddName(tmp))
			{
				RunStateError("添加名称失败失败! DBName:%s, IP:%s, 条目:%d，名称:%s", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num, tmp);
				return false;
			}

			res->NextRow();
		}

		++num;
	}

	return CServerMgr::Init(ip, serverid, port, overtime);
}

void CNameCheckServerMgr::Destroy()
{
	CServerMgr::Destroy();
	m_CenterList.clear();
}

void CNameCheckServerMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "当前注册的服务器信息：\n中心服务器数量：%d\n",
		(int)m_CenterList.size());
}

void CNameCheckServerMgr::ResetMsgNum()
{
	for (std::map<int, serverinfo*>::iterator itr = m_CenterList.begin(); itr != m_CenterList.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}
}

const char *CNameCheckServerMgr::GetMsgNumInfo()
{
	static char tempbuf[1024 * 32];
	char *buf = tempbuf;
	size_t len = sizeof(tempbuf);
	int res = 0;
	serverinfo *info = nullptr;

	for (std::map<int, serverinfo*>::iterator itr = m_CenterList.begin(); itr != m_CenterList.end(); ++itr)
	{
		info = itr->second;
		snprintf(buf, len - 1, "中心服务器: %d, 收到消息数量:%d, 发送消息数量:%d\n", \
			info->GetServerID(), info->GetRecvMsgNum(), info->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	tempbuf[sizeof(tempbuf) - len] = 0;
	tempbuf[sizeof(tempbuf) - 1] = 0;
	return tempbuf;
}

void CNameCheckServerMgr::OnConnectDisconnect(serverinfo *info, bool overtime)
{
	switch (info->GetServerType())
	{
	case ServerEnum::EST_CENTER:
	{
		m_CenterList.erase(info->GetServerID());
		if (overtime)
			RunStateError("中心服器超时移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("中心服器关闭移除:[%d], ip:[%s]", info->GetServerID(), info->GetIP());

		break;
	}
	default:
	{
		if (overtime)
			RunStateError("未注册的服务器超时移除, ip:[%s]", info->GetIP());
		else
			RunStateError("未注册的服务器关闭移除, ip:[%s]", info->GetIP());
	}
	}
}

void CNameCheckServerMgr::ProcessMsg(serverinfo *info)
{
	Msg *pMsg;
	for (;;)
	{
		pMsg = info->GetMsg();
		if (!pMsg)
			return;

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
				info->SendMsg(pMsg);
				info->SetPingTime(g_currenttime);
				break;
			}
			}
			break;
		}
		case LOGIN_TYPE_MAIN:
		{
			switch (pMsg->GetSubType())
			{
			case LOGIN_SUB_CREATE_PLAYER:
			{
				netData::CreatePlayer msg;
				_CHECK_PARSE_(pMsg, msg);

				if (!NameSet.AddName(msg.sname()))
					msg.set_nnamecheckret(netData::CreatePlayer::EC_REPEATED);
				else
					msg.set_nnamecheckret(netData::CreatePlayer::EC_SUCC);

				SendMsg(info, msg, LOGIN_TYPE_MAIN, LOGIN_SUB_CREATE_PLAYER, tl, sizeof(msgtail));
				break;
			}
			}
			break;
		}
		}
	}
}

bool CNameCheckServerMgr::AddNewServer(serverinfo *info, int nServerID, int nType)
{
	if (FindServer(nServerID, nType))
	{
		RunStateError("添加服务器失败！已经存在的服务器，远程服务器ID：[%d] IP:[%s]", nServerID, info->GetIP());
		return false;
	}

	std::map<int, serverinfo*> *_pList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_CENTER:
	{
		_pList = &m_CenterList;
		break;
	}
	default:
	{
		RunStateError("添加服务器失败！不存在的服务器类型，远程服务器ID：[%d] 类型：[%d] IP:[%s]", nServerID, nType, info->GetIP());
		return false;
	}
	}

	info->SetAlreadyRegister();
	info->SetServerID(nServerID);
	info->SetServerType(nType);
	_pList->insert(std::make_pair(nServerID, info));
	return true;
}

serverinfo *CNameCheckServerMgr::FindServer(int nServerID, int nType)
{
	std::map<int, serverinfo*> *_pList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_CENTER:
	{
		_pList = &m_CenterList;
		break;
	}
	default:
		return nullptr;
	}
	std::map<int, serverinfo*>::iterator itr = _pList->find(nServerID);
	if (itr != _pList->end())
		return itr->second;
	return nullptr;
}
