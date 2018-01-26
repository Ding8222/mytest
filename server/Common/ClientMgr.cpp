#include"stdfx.h"
#include "ClientMgr.h"
#include "Client.h"
#include "idmgr.c"

static const int s_backlog = 128;
extern int64 g_currenttime;

static objectpool<CClient> &ClientPool()
{
	static objectpool<CClient> m(CClientMgr::enum_client_id_max, "CClient pools");
	return m;
}

static CClient *client_create()
{
	CClient *self = ClientPool().GetObject();
	if (!self)
	{
		log_error("创建 CClient 失败!");
		return NULL;
	}
	new(self) CClient();
	return self;
}

static void client_release(CClient *self)
{
	if (!self)
		return;
	self->~CClient();
	ClientPool().FreeObject(self);
}

CClientMgr::CClientMgr()
{
	m_MaxClientNum = 0;
	m_ListenPort = 0;
	m_OverTime = 0;
	m_Listen = NULL;
	m_ClientList.clear();
	m_WaitRemove.clear();
	m_ClientSet.clear();
	m_IDPool = NULL;
}

CClientMgr::~CClientMgr()
{
	Destroy();
}

bool CClientMgr::Init(int maxclientnum, int listenport, int clientovertime, int recvdatalimit, int senddatalimit)
{
	m_MaxClientNum = maxclientnum;
	m_ListenPort = listenport;
	m_OverTime = clientovertime;
	m_RecvDataLimit = recvdatalimit;
	m_SendDataLimit = senddatalimit;

	m_Listen = lxnet::Listener::Create();
	if (!m_Listen)
	{
		log_error("创建Listener失败!");
		return false;
	}

	if (!InitIdMgrAndClientSet())
	{
		log_error("初始化IDMgr失败!");
		return false;
	}

	if (!TestAndListen())
		return false;

	return true;
}

void CClientMgr::Run()
{
	idmgr_run(m_IDPool);

	AcceptNewClient();
	ProcessAllClient();
	CheckAndRemove();
}

void CClientMgr::EndRun()
{
	for (std::list<CClient*>::iterator itr = m_ClientList.begin(); itr != m_ClientList.end(); ++itr)
	{
		(*itr)->GetCon()->CheckRecv();
		(*itr)->GetCon()->CheckSend();
	}
}

void CClientMgr::Destroy()
{
	if (m_Listen)
	{
		StopListen();
		lxnet::Listener::Release(m_Listen);
		m_Listen = NULL;
	}
	for (std::list<CClient*>::iterator itr = m_ClientList.begin(); itr != m_ClientList.end(); ++itr)
	{
		ReleaseClientAndID(*itr);
	}
	m_ClientList.clear();

	for (std::list<CClient*>::iterator itr = m_WaitRemove.begin(); itr != m_WaitRemove.end(); ++itr)
	{
		ReleaseClientAndID(*itr);
	}
	m_WaitRemove.clear();
	m_ClientSet.clear();

	if (m_IDPool)
	{
		idmgr_release(m_IDPool);
		m_IDPool = NULL;
	}
}

void CClientMgr::GetCurrentInfo(char *buf, size_t buflen)
{
	snprintf(buf, buflen - 1, "最大 Client 限制:%d\n当前 Client 数量:%d\n总ID数量 total:%d\n当前使用ID数量:%d, 待移除 Client 数量:%d\n", m_MaxClientNum, (int)m_ClientList.size(), idmgr_total(m_IDPool), idmgr_usednum(m_IDPool), (int)m_WaitRemove.size());
}

void CClientMgr::SendMsgToClientByID(Msg *pMsg, int clientid)
{
	CClient *cl = FindClient(clientid);
	if (!cl)
		return;
	cl->SendMsg(pMsg);
}

CClient *CClientMgr::FindClient(int clientid)
{
	if (clientid <= 0 || clientid >= (int)m_ClientSet.size())
		return NULL;
	CClient *cl = m_ClientSet[clientid];

	//只返回处于正常列的。
	if (!cl || !cl->IsInNormal())
		return NULL;
	return cl;
}

void CClientMgr::SetClientAuthSucceed(int clientid)
{
	CClient *cl = FindClient(clientid);
	if (!cl)
		return;
	cl->SetAlreadyAuth();
}

//延时关闭某个客户端
void CClientMgr::DelayCloseClient(int clientid)
{
	CClient *cl = FindClient(clientid);
	if (!cl)
		return;
	cl->SetRemove(g_currenttime);
}

void CClientMgr::ReleaseAllClient()
{
	for (std::list<CClient*>::iterator itr = m_ClientList.begin(); itr != m_ClientList.end(); ++itr)
	{
		ReleaseClientAndID(*itr);
	}
	m_ClientList.clear();

	for (std::list<CClient*>::iterator itr = m_WaitRemove.begin(); itr != m_WaitRemove.end(); ++itr)
	{
		ReleaseClientAndID(*itr);
	}
	m_WaitRemove.clear();
	log_error("服务器断开连接, 关闭所有Client!");
}

void CClientMgr::AsLogicServerDisconnect(int logicserverid)
{
	for (std::list<CClient*>::iterator itr = m_ClientList.begin(); itr != m_ClientList.end(); ++itr)
	{
		if ((*itr)->GetLogicServer() == logicserverid)
		{
			(*itr)->SetLogicServerID(0);
			(*itr)->SetInLogicServerID(0);
		}
	}
}

void CClientMgr::StopListen()
{
	if (!m_Listen->IsClose())
	{
		m_Listen->Close();
		log_error("停止监听端口 %d", m_ListenPort);
	}
}

bool CClientMgr::TestAndListen()
{
	if (!m_Listen->IsClose())
		//already listening.
		return true;

	if (m_Listen->Listen(m_ListenPort, s_backlog))
	{
		log_error("监听端口 %d 成功!", m_ListenPort);
		return true;
	}

	log_error("监听端口 %d 失败!", m_ListenPort);
	return false;
}

int64 CClientMgr::OnNewClient()
{
	if ((int)m_ClientList.size() >= m_MaxClientNum)
		return 0;

	int id = 0;
	lxnet::Socketer *sock = NULL;
	CClient *newclient = NULL;
	sock = m_Listen->Accept();
	if (!sock)
		return 0;

	id = idmgr_allocid(m_IDPool);
	if (id <= 0)
	{
		log_error("为新Client分配ID失败!, id:%d", id);
		goto do_error;
	}

	newclient = client_create();
	if (!newclient)
	{
		log_error("创建Client失败!");
		goto do_error;
	}

	sock->SetRecvLimit(m_RecvDataLimit);
	sock->SetSendLimit(m_SendDataLimit);
	sock->UseCompress();
	sock->UseEncrypt();
	sock->UseDecrypt();
	//sock->UseTGW();

	newclient->SetClientID(id);
	newclient->SetCon(sock);
	newclient->SetConnectTime(g_currenttime);
	newclient->SetPingTime(g_currenttime);

	m_ClientList.push_back(newclient);
	newclient->SetInNormal();

	char ip[64];
	sock->GetIP(ip, sizeof(ip) - 1);
	ip[sizeof(ip) - 1] = 0;

	//ClientConnectLog("新Client连接, ip:%s, ID:%d, 当前Client总数量:%d", ip, id, (int)m_ClientList.size());

	assert(NULL == m_ClientSet[id]);
	m_ClientSet[id] = newclient;

	return id;

do_error:
	if (id > 0)
	{
		if (!idmgr_freeid(m_IDPool, id))
			log_error("释放ID失败!, ID:%d", id);
	}
	if (sock)
		lxnet::Socketer::Release(sock);
	if (newclient)
		client_release(newclient);
	return 0;
}

void CClientMgr::AcceptNewClient()
{
	for (int i = 0; i < s_backlog; ++i)
	{
		if (!m_Listen->CanAccept())
			break;

		OnNewClient();
	}
}

void CClientMgr::OnClientDisconnect(CClient *cl)
{
	assert(cl->IsInNormal());
	cl->SetInWaitRemove();
	m_WaitRemove.push_back(cl);
	cl->SetRemove(g_currenttime);
}

void CClientMgr::SendMsg(CClient *cl, google::protobuf::Message &pMsg, int maintype, int subtype)
{
	assert(cl != nullptr);

	MessagePack pk;
	pk.Pack(&pMsg, maintype, subtype);
	cl->SendMsg(&pk);
}


void CClientMgr::SendMsg(CClient *cl, Msg *pMsg)
{
	assert(cl != nullptr);
	assert(pMsg != nullptr);

	cl->SendMsg(pMsg);
}

void CClientMgr::SendMsg(int64 clientid, google::protobuf::Message &pMsg, int maintype, int subtype)
{
	std::list<CClient*>::iterator _Iter
		= std::find_if(m_ClientList.begin(), m_ClientList.end(), [clientid](CClient* cl)->bool { return cl->GetClientID() == clientid; });
	if (_Iter != m_ClientList.end())
	{
		MessagePack pk;
		pk.Pack(&pMsg, maintype, subtype);
		(*_Iter)->SendMsg(&pk);
	}
}


void CClientMgr::SendMsg(int64 clientid, Msg *pMsg)
{
	std::list<CClient*>::iterator _Iter
		= std::find_if(m_ClientList.begin(), m_ClientList.end(), [clientid](CClient* cl)->bool { return cl->GetClientID() == clientid; });
	if (_Iter != m_ClientList.end())
	{
		(*_Iter)->SendMsg(pMsg);
	}
}

void CClientMgr::ProcessAllClient()
{
	std::list<CClient*>::iterator itr, tempitr;
	for (itr = m_ClientList.begin(); itr != m_ClientList.end();)
	{
		tempitr = itr;
		++itr;

		//从连接上来，经过若干时间，还未认证，则断开连接
		if ((*tempitr)->IsEnoughXMNotAlreadyAuth(g_currenttime))
		{
			(*tempitr)->GetCon()->Close();
		}

		if ((*tempitr)->IsNeedRemove())
		{
			//ClientConnectLog("服务器延时移除Client, ID:%d, 剩余Client数量:%d", (*tempitr)->GetClientID(), (int)m_ClientList.size() - 1);
			assert((*tempitr)->IsInNormal());
			(*tempitr)->SetInWaitRemove();
			(*tempitr)->GetCon()->CheckSend();
			m_WaitRemove.push_back(*tempitr);

			m_ClientList.erase(tempitr);
			continue;
		}

		if ((*tempitr)->GetCon()->IsClose())
		{
			//ClientConnectLog("与Client的连接断开, id:%d, 剩余Client数量:%d", (*tempitr)->GetClientID(), (int)m_ClientList.size() - 1);
			OnClientDisconnect(*tempitr);
			m_ClientList.erase(tempitr);
			continue;
		}

		if ((*tempitr)->IsOverTime(g_currenttime, m_OverTime))
		{
			//ClientConnectLog("与Client的连接超时关闭, id:%d, 剩余Client数量:%d", (*tempitr)->GetClientID(), (int)m_ClientList.size() - 1);
			(*tempitr)->GetCon()->Close();
			OnClientDisconnect(*tempitr);
			m_ClientList.erase(tempitr);
			continue;
		}

		ProcessClientMsg(*tempitr);
	}
}

void CClientMgr::CheckAndRemove()
{
	CClient *cl;
	while (!m_WaitRemove.empty())
	{
		cl = m_WaitRemove.front();
		if (!cl->CanRemove(g_currenttime))
			break;
		ReleaseClientAndID(cl);
		m_WaitRemove.pop_front();
	}
}

void CClientMgr::ReleaseClientAndID(CClient *cl)
{
	if (!cl)
		return;
	int id = cl->GetClientID();
	if (id <= 0 || id >= (int)m_ClientSet.size())
	{
		log_error("要释放的Client的ID错误!");
		return;
	}
	m_ClientSet[id] = NULL;

	if (!idmgr_freeid(m_IDPool, id))
	{
		log_error("释放ID错误, ID:%d", id);
	}

	cl->SetClientID(0);
	client_release(cl);
}

bool CClientMgr::InitIdMgrAndClientSet()
{
	m_IDPool = idmgr_create(enum_client_id_max + 1, enum_client_id_delay_time);
	if (!m_IDPool)
	{
		log_error("创建IDMgr失败!");
		return false;
	}
	m_ClientSet.resize(enum_client_id_max + 1, NULL);
	return true;
}

