﻿#include <string.h>
#include <stdlib.h>
#include "task.h"
#include "msgbase.h"
#include "lxnet\base\catomic.h"
#include "lxnet\base\cthread.h"
#include "objectpool.h"
#include "blockbuf.h"
#include "serverlog.h"

enum
{
	enum_max_thread_num = 16,	//
};
struct thread_localuse
{
	unsigned int threadid;
	char *buf;
};

struct threadinfo
{
	bool isinit;
	int msgmaxsize;
	struct thread_localuse msgbuf[enum_max_thread_num];
	catomic freeindex;
};

static struct threadinfo s_threadbuf = {false};

static bool threadinfo_init ()
{
	if (s_threadbuf.isinit)
		return false;
	s_threadbuf.msgmaxsize = 256*1024;

	for (int i = 0; i < enum_max_thread_num; ++i)
	{
		s_threadbuf.msgbuf[i].threadid = 0;
		s_threadbuf.msgbuf[i].buf = nullptr;
	}

	catomic_inc(&s_threadbuf.freeindex);
	s_threadbuf.isinit = true;
	return true;
}

static void threadinfo_release ()
{
	if (!s_threadbuf.isinit)
		return;
	s_threadbuf.isinit = false;
	for (int i = 0; i < enum_max_thread_num; ++i)
	{
		if (s_threadbuf.msgbuf[i].buf)
		{
			free(s_threadbuf.msgbuf[i].buf);
			s_threadbuf.msgbuf[i].buf = nullptr;
		}
	}
}

static char *threadbuf_get_msg_buf ()
{
	if (!s_threadbuf.isinit)
	{
		RunStateError("not init s_threadbuf, error!");
		exit(1);
	}
	unsigned int currentthreadid = cthread_self_id();

	int index;
	for (index = 0; index < enum_max_thread_num; ++index)
	{
		if (s_threadbuf.msgbuf[index].threadid == currentthreadid)
		{
			if (s_threadbuf.msgbuf[index].buf == nullptr)
			{
				RunStateError("why find this thread local msgbuf, but the buf is nil!, error!");
				exit(1);
			}
			return s_threadbuf.msgbuf[index].buf;
		}
	}
	index = (int)catomic_fetch_add(&s_threadbuf.freeindex, 1);
	if (index < 0 || index >= enum_max_thread_num)
	{
		RunStateError("thread num is overtop, failed!, index:%d, enum_max_thread_num:%d", index, enum_max_thread_num);
		exit(1);
	}
	s_threadbuf.msgbuf[index].buf = (char *)malloc(s_threadbuf.msgmaxsize);
	if (!s_threadbuf.msgbuf[index].buf)
	{
		RunStateError("malloc new thread local msgbuf failed!, error!");
		exit(1);
	}
	s_threadbuf.msgbuf[index].threadid = currentthreadid;
	return s_threadbuf.msgbuf[index].buf;
}


struct blocktempbuf
{
	char buf[1024*8];
};

class TaskSomePool
{
public:

	TaskSomePool ()
	{
		m_pool = new objectpool<blocktempbuf>(16000, "blocktempbuf pools");
		cspin_init(&m_blocklock);

		m_taskpool = new objectpool<task>(16000, "task pools");
		cspin_init(&m_tasklock);
	}

	~TaskSomePool ()
	{
		delete m_pool;
		cspin_destroy(&m_blocklock);

		delete m_taskpool;
		cspin_destroy(&m_tasklock);
	}

	blocktempbuf *AllocBlock ()
	{
		blocktempbuf *res = nullptr;
		cspin_lock(&m_blocklock);
		res = m_pool->GetObject();
		cspin_unlock(&m_blocklock);
		return res;
	}

	void ReleaseBlock (blocktempbuf *tk)
	{
		if (!tk)
			return;
		cspin_lock(&m_blocklock);
		m_pool->FreeObject(tk);
		cspin_unlock(&m_blocklock);
	}

	task *AllocTask ()
	{
		task *res = nullptr;
		cspin_lock(&m_tasklock);
		res = m_taskpool->GetObject();
		cspin_unlock(&m_tasklock);
		return res;
	}

	void ReleaseTask (task *tk)
	{
		if (!tk)
			return;
		cspin_lock(&m_tasklock);
		m_taskpool->FreeObject(tk);
		cspin_unlock(&m_tasklock);
	}

private:
	objectpool<blocktempbuf> *m_pool;
	cspin m_blocklock;

	objectpool<task> *m_taskpool;
	cspin m_tasklock;
};

static TaskSomePool &SomePool ()
{
	static TaskSomePool m;
	return m;
}

static blockbuf *block_create ()
{
	blockbuf *self = (blockbuf *)SomePool().AllocBlock();
	if (!self)
	{
		RunStateError("create blockbuf failed!");
		return nullptr;
	}
	new(self) blockbuf();
	self->setsize(sizeof(blocktempbuf) - sizeof(blockbuf));
	return self;
}

static void block_release (blockbuf *self)
{
	if (!self)
		return;
	self->~blockbuf();
	SomePool().ReleaseBlock((blocktempbuf *)self);
}

task *task_create ()
{
	task *self = SomePool().AllocTask();
	if (!self)
	{
		RunStateError("create task failed!");
		return nullptr;
	}
	new(self) task();
	return self;
}

void task_release (void *self)
{
	task *tk = (task *)self;
	if (!tk)
		return;
	tk->~task();
	SomePool().ReleaseTask(tk);
}

task::task ()
{
	m_cachecon = nullptr;
	m_con = nullptr;

	m_tasktype = tasktype_process;
	m_needsend = true;
	m_sendtoall = false;
	m_serverid = 0;
	m_clientid = 0;

	m_head = nullptr;
	m_currentforpush = nullptr;
	m_currentforget = nullptr;
}

task::~task ()
{
	Destroy();
}

bool task::InitPools ()
{
	blocktempbuf *bf = SomePool().AllocBlock();
	SomePool().ReleaseBlock(bf);

	task *tk = SomePool().AllocTask();
	SomePool().ReleaseTask(tk);

	return threadinfo_init();
}

void task::DestroyPools ()
{
	threadinfo_release();
}

//设置一些必须的附带信息
void task::SetInfo (CDBCache *con, int32 serverid, int64 clientid)
{
	m_cachecon = con;
	m_serverid = serverid;
	m_clientid = clientid;
}

void task::SetInfo(DataBase::CConnection *con, int32 serverid, int64 clientid)
{
	m_con = con;
	m_serverid = serverid;
	m_clientid = clientid;
}

//设置是否需要发送
void task::SetAsNeedSend (bool flag)
{
	m_needsend = flag;
}

//设置是否发送到所有连接，默认只发送到关联的连接
void task::SetSendToAll (bool flag)
{
	m_sendtoall = flag;
}

//重置
void task::Reset ()
{
	blockbuf *node, *next;
	for (node = m_head; node; node = next)
	{
		next = node->next;
		node->reset();
	}
	m_currentforpush = m_head;
	m_currentforget = m_head;
}

//装入一个消息
bool task::PushMsg (Msg *pMsg)
{
	CheckBlockNull();

	if (!m_currentforpush)
		return false;

	char *src = (char *)pMsg;
	int len = pMsg->GetLength();
	int writesize = 0;
	int pushsize = 0;
	while (writesize < len)
	{
		//当前块可写字节为0时
		if (m_currentforpush->getwritesize() == 0)
		{
			if (!m_currentforpush->next)
			{
				//若不存在下一个块，创建新快，附加上去
				m_currentforpush->next = block_create();
				if (!m_currentforpush->next)
				{
					RunStateError("pushmsg, but create new blockbuf failed!, msg type:%d", pMsg->GetType());
					return false;
				}
			}
			
			//当前块存在下一个块，变更当前块
			m_currentforpush = m_currentforpush->next;
			assert(m_currentforpush->isempty());
		}
		pushsize = m_currentforpush->pushdata(&src[writesize], len - writesize);
		writesize += pushsize;
	}
	return true;
}

//获取一个消息
Msg *task::GetMsg ()
{
	if (!m_currentforget)
		return nullptr;

	char *buf = threadbuf_get_msg_buf();
	int len = 0;
	int readsize = 0;
	
	//先读长度信息
	readsize = getdata((char *)&len, (int)sizeof(len));
	if (readsize < (int)sizeof(len))
		return nullptr;
	if (len >= s_threadbuf.msgmaxsize)
		return nullptr;
	*(int *)&buf[0] = len;
	readsize = getdata(&buf[sizeof(len)], len - (int)sizeof(len));
	if (readsize < (len - (int)sizeof(len)))
		return nullptr;
	return (Msg *)buf;
}

void task::CheckBlockNull ()
{
	if (m_head)
		return;
	m_head = block_create();
	if (!m_head)
	{
		RunStateError("create blockbuf failed!");
		return;
	}
	m_currentforpush = m_head;
	m_currentforget = m_head;
}

int task::getdata (char *buf, int len)
{
	if (!m_currentforget)
		return 0;
	int readsize = 0;
	int getsize = 0;
	while (readsize < len)
	{
		if (m_currentforget->getreadsize() == 0)
			m_currentforget = m_currentforget->next;
		if (!m_currentforget)
			break;
		getsize = m_currentforget->getdata(&buf[readsize], len - readsize);
		readsize += getsize;
	}
	return readsize;
}

void task::Destroy ()
{
	blockbuf *node, *next;
	for (node = m_head; node; node = next)
	{
		next = node->next;
		block_release(node);
	}
	m_head = nullptr;
	m_currentforpush = nullptr;
	m_currentforget = nullptr;
}


