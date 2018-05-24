#include "StringPool.h"
#include <string.h>
#include <stdlib.h>
#include "msgbase.h"
#include "ossome.h"
#include "objectpool.h"
#include "blockbuf.h"
#include "serverlog.h"

enum
{
	enum_max_thread_num = 16,
};
struct thread_localuse
{
	THREAD_ID threadid;
	char *buf;
};

struct threadinfo
{
	bool isinit;
	int32 msgmaxsize;
	struct thread_localuse msgbuf[enum_max_thread_num];
	volatile long freeindex;
};

static struct threadinfo s_threadbuf = { false };

static bool threadinfo_init()
{
	if (s_threadbuf.isinit)
		return false;
	s_threadbuf.msgmaxsize = 256 * 1024;

	for (int32 i = 0; i < enum_max_thread_num; ++i)
	{
		s_threadbuf.msgbuf[i].threadid = 0;
		s_threadbuf.msgbuf[i].buf = nullptr;
	}

	s_threadbuf.freeindex = 0;
	s_threadbuf.isinit = true;
	return true;
}

static void threadinfo_release()
{
	if (!s_threadbuf.isinit)
		return;
	s_threadbuf.isinit = false;
	for (int32 i = 0; i < enum_max_thread_num; ++i)
	{
		if (s_threadbuf.msgbuf[i].buf)
		{
			free(s_threadbuf.msgbuf[i].buf);
			s_threadbuf.msgbuf[i].buf = nullptr;
		}
	}
}

static char *threadbuf_get_msg_buf()
{
	if (!s_threadbuf.isinit)
	{
		RunStateError("not init s_threadbuf, error!");
		exit(1);
	}
	THREAD_ID currentthreadid = CURRENT_THREAD;

	int32 index;
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
	index = (int32)atom_fetch_add(&s_threadbuf.freeindex, 1);
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
struct stringblock
{
	char buf[80];
};

class StringSomePool
{
public:

	StringSomePool()
	{
		m_pool = new objectpool<stringblock>(16000, "stringblock pools");
		LOCK_INIT(&m_blocklock);

		m_stringpool = new objectpool<CStringPool>(16000, "CStringPool pools");
		LOCK_INIT(&m_stringlock);
	}

	~StringSomePool()
	{
		delete m_pool;
		LOCK_DELETE(&m_blocklock);

		delete m_stringpool;
		LOCK_DELETE(&m_stringlock);
	}

	stringblock *AllocBlock()
	{
		stringblock *res = nullptr;
		LOCK_LOCK(&m_blocklock);
		res = m_pool->GetObject();
		LOCK_UNLOCK(&m_blocklock);
		return res;
	}

	void ReleaseBlock(stringblock *tk)
	{
		if (!tk)
			return;
		LOCK_LOCK(&m_blocklock);
		m_pool->FreeObject(tk);
		LOCK_UNLOCK(&m_blocklock);
	}

	CStringPool *AllocString()
	{
		CStringPool *res = nullptr;
		LOCK_LOCK(&m_stringlock);
		res = m_stringpool->GetObject();
		LOCK_UNLOCK(&m_stringlock);
		return res;
	}

	void ReleaseString(CStringPool *tk)
	{
		if (!tk)
			return;
		LOCK_LOCK(&m_stringlock);
		m_stringpool->FreeObject(tk);
		LOCK_UNLOCK(&m_stringlock);
	}

private:
	objectpool<stringblock> *m_pool;
	LOCK_struct m_blocklock;

	objectpool<CStringPool> *m_stringpool;
	LOCK_struct m_stringlock;
};

static StringSomePool &SomePool()
{
	static StringSomePool m;
	return m;
}

static blockbuf *block_create()
{
	blockbuf *self = (blockbuf *)SomePool().AllocBlock();
	if (!self)
	{
		RunStateError("create blockbuf failed!");
		return nullptr;
	}
	new(self) blockbuf();
	self->setsize(sizeof(stringblock) - sizeof(blockbuf));
	return self;
}

static void block_release(blockbuf *self)
{
	if (!self)
		return;
	self->~blockbuf();
	SomePool().ReleaseBlock((stringblock *)self);
}

CStringPool *string_create()
{
	CStringPool *self = SomePool().AllocString();
	if (!self)
	{
		RunStateError("create CStringPool failed!");
		return nullptr;
	}
	new(self) CStringPool();
	return self;
}

void string_release(void *self)
{
	CStringPool *tk = (CStringPool *)self;
	if (!tk)
		return;
	tk->~CStringPool();
	SomePool().ReleaseString(tk);
}

CStringPool::CStringPool()
{
	m_head = nullptr;
	m_currentforpush = nullptr;
	m_currentforget = nullptr;
}

CStringPool::~CStringPool()
{
	Destroy();
}

bool CStringPool::InitPools()
{
	stringblock *bf = SomePool().AllocBlock();
	SomePool().ReleaseBlock(bf);

	CStringPool *tk = SomePool().AllocString();
	SomePool().ReleaseString(tk);

	return threadinfo_init();
}

void CStringPool::DestroyPools()
{
	threadinfo_release();
}

//重置
void CStringPool::Reset()
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
bool CStringPool::PushMsg(std::string &str)
{
	CheckBlockNull();

	if (!m_currentforpush)
		return false;

	const char * src= str.c_str();
	int32 len = str.size();
	int32 writesize = 0;
	int32 pushsize = 0;

	m_currentforpush->pushdata(&len, sizeof(len));
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
					RunStateError("push string, but create new blockbuf failed!, string:%d", src);
					return false;
				}
			}

			//当前块存在下一个块，变更当前块
			m_currentforpush = m_currentforpush->next;
			assert(m_currentforpush->isempty());
		}
		pushsize = m_currentforpush->pushdata((char *)&(src[writesize]), len - writesize);
		writesize += pushsize;
	}
	return true;
}

//获取一个消息
const char *CStringPool::GetMsg()
{
	if (!m_currentforget)
		return nullptr;

	char *buf = threadbuf_get_msg_buf();
	int32 len = 0;
	int32 readsize = 0;

	//先读长度信息
	readsize = getdata((char *)&len, (int32)sizeof(len));
	if (readsize < (int32)sizeof(len))
		return nullptr;
	if (len >= s_threadbuf.msgmaxsize)
		return nullptr;
	readsize = getdata(buf, len);
	if (readsize < len)
		return nullptr;

	buf[len] = '\0';
	return buf;
}

void CStringPool::CheckBlockNull()
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

int32 CStringPool::getdata(char *buf, int32 len)
{
	if (!m_currentforget)
		return 0;
	int32 readsize = 0;
	int32 getsize = 0;
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

void CStringPool::Destroy()
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
