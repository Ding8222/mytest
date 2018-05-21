#pragma once
#include "DBCache.h"
#include "sqlinterface.h"
struct Msg;
class blockbuf;
class task
{
public:
	 task ();
	 ~task ();

	 enum
	 {
		 //需要处理的类型
		 tasktype_process = 0,

		 //转发类型
		 tasktype_transmit = 1,

		 //有连接断开
		 tasktype_serverdisconnect = 2,
	 };

	 static bool InitPools ();

	 static void DestroyPools ();

	 //设置一些必须的附带信息
	 void SetInfo(CDBCache *con, int32 serverid, int64 clientid = 0);
	 void SetInfo(DataBase::CConnection *con, int32 serverid, int64 clientid = 0);

	 //获取数据库操作句柄
	 CDBCache *GetCacheDBHand() { return m_cachecon; }
	 DataBase::CConnection *GetDBHand() { return m_con; }

	 //获取此任务来自哪个服务器
	 int32 GetServerID () { return m_serverid; }

	 //获取此任务来自哪个client
	 int64 GetClientID() { return m_clientid; }

	 //设置是否需要发送
	 void SetAsNeedSend (bool flag);

	 //是否需要发送
	 bool IsNeedSend () { return m_needsend; }

	 //设置是否发送到所有连接，默认只发送到关联的连接
	 void SetSendToAll (bool flag);

	 //是否发送到所有连接
	 bool IsSendToAll () { return m_sendtoall; }

	 //设置此任务的类型
	 void SetTaskType (int type) { m_tasktype = type; }

	 //是否是转发类型
	 bool IsTransmit () { return m_tasktype == tasktype_transmit; }

	 //是否需要执行此任务
	 bool IsProcess () { return m_tasktype == tasktype_process; }

	 //是否是连接断开
	 bool IsServerDisconnect () { return m_tasktype == tasktype_serverdisconnect; }

	 //重置
	 void Reset ();

	 //装入一个消息
	 bool PushMsg (Msg *pMsg);

	 //获取一个消息
	 Msg *GetMsg ();

private:
	void CheckBlockNull ();
	int getdata (char *buf, int len);
	void Destroy ();

private:
	CDBCache *m_cachecon;
	DataBase::CConnection *m_con;

	int8 m_tasktype;
	bool m_needsend;
	bool m_sendtoall;
	int32 m_serverid;

///////////////////////////////////////
	blockbuf *m_head;
	blockbuf *m_currentforpush;
	blockbuf *m_currentforget;
	int64 m_clientid;
};

task *task_create ();

void task_release (void *self);
