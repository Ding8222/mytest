#include <map>
#include "serverinfo.h"
#include "Config.h"
#include "serverlog.h"
#include "GlobalDefine.h"
#include "sqlinterface.h"
#include "NameCheckServerMgr.h"

extern int64 g_currenttime;

CNameCheckServerMgr::CNameCheckServerMgr()
{
	m_LoginList.clear();
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
	std::list<TableInfo *> temp = CConfig::Instance().GetTableList();
	for (std::list<TableInfo *>::iterator itr = temp.begin(); itr != temp.end(); ++itr)
	{
		if (!dbhand.Open((*itr)->dbname.c_str(), (*itr)->dbusername.c_str(), (*itr)->dbpassword.c_str(), (*itr)->dbip.c_str()))
		{
			RunStateError("�������ݿ�ʧ��! DBName:%s, IP:%s, ��Ŀ:%d", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num);
			return false;
		}

		if (!dbhand.SetCharacterSet("utf8"))
		{
			RunStateError("���ñ�playerdateΪutf8ģʽʧ��! DBName:%s, IP:%s, ��Ŀ:%d", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num);
			return false;
		}

		res = dbhand.Execute((*itr)->sqlstr.c_str());
		if (!res || !res->IsOpen())
		{
			RunStateError("SQL����ѯʧ��! DBName:%s, IP:%s, ��Ŀ:%d", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num);
			return false;
		}

		while (!res->IsEnd())
		{
			tmp = res->Get((*itr)->fieldname.c_str());
			if (!tmp)
			{
				RunStateError("��ȡ�ֶ�ʧ��! DBName:%s, IP:%s, ��Ŀ:%d", (*itr)->dbname.c_str(), (*itr)->dbip.c_str(), num);
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
	m_LoginList.clear();
	m_CenterList.clear();
}

void CNameCheckServerMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "��ǰע��ķ�������Ϣ��\n��½������������%d\n���ķ�����������%d\n",
		(int)m_LoginList.size(), (int)m_CenterList.size());
}

void CNameCheckServerMgr::ResetMsgNum()
{
	for (std::map<int, serverinfo*>::iterator itr = m_LoginList.begin(); itr != m_LoginList.end(); ++itr)
	{
		itr->second->ResetMsgNum();
	}
	
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
	for (std::map<int, serverinfo*>::iterator itr = m_LoginList.begin(); itr != m_LoginList.end(); ++itr)
	{
		info = itr->second;
		snprintf(buf, len - 1, "��½������: %d, �յ���Ϣ����:%d, ������Ϣ����:%d\n", \
			info->GetServerID(), info->GetRecvMsgNum(), info->GetSendMsgNum());

		res = strlen(buf);
		buf += res;
		len -= res;
	}

	for (std::map<int, serverinfo*>::iterator itr = m_CenterList.begin(); itr != m_CenterList.end(); ++itr)
	{
		info = itr->second;
		snprintf(buf, len - 1, "���ķ�����: %d, �յ���Ϣ����:%d, ������Ϣ����:%d\n", \
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
	case ServerEnum::EST_LOGIN:
	{
		m_LoginList.erase(info->GetServerID());
		if (overtime)
			RunStateError("��½������ʱ�Ƴ�:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("��½�����ر��Ƴ�:[%d], ip:[%s]", info->GetServerID(), info->GetIP());

		break;
	}
	case ServerEnum::EST_CENTER:
	{
		m_CenterList.erase(info->GetServerID());
		if (overtime)
			RunStateError("���ķ�����ʱ�Ƴ�:[%d], ip:[%s]", info->GetServerID(), info->GetIP());
		else
			RunStateError("���ķ����ر��Ƴ�:[%d], ip:[%s]", info->GetServerID(), info->GetIP());

		break;
	}
	default:
	{
		if (overtime)
			RunStateError("δע��ķ�������ʱ�Ƴ�, ip:[%s]", info->GetIP());
		else
			RunStateError("δע��ķ������ر��Ƴ�, ip:[%s]", info->GetIP());
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

	}
}

bool CNameCheckServerMgr::AddNewServer(serverinfo *info, int nServerID, int nType)
{
	if (FindServer(nServerID, nType))
	{
		RunStateError("��ӷ�����ʧ�ܣ��Ѿ����ڵķ�������Զ�̷�����ID��[%d] IP:[%s]", nServerID, info->GetIP());
		return false;
	}

	std::map<int, serverinfo*> *_pList = nullptr;
	switch (nType)
	{
	case ServerEnum::EST_LOGIN:
	{
		_pList = &m_LoginList;
		break;
	}
	case ServerEnum::EST_CENTER:
	{
		_pList = &m_CenterList;
		break;
	}
	default:
	{
		RunStateError("��ӷ�����ʧ�ܣ������ڵķ��������ͣ�Զ�̷�����ID��[%d] ���ͣ�[%d] IP:[%s]", nServerID, nType, info->GetIP());
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
	case ServerEnum::EST_LOGIN:
	{
		_pList = &m_LoginList;
		break;
	}
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

void CNameCheckServerMgr::ServerRegisterSucc(int id, int type, const char *ip, int port)
{

}
