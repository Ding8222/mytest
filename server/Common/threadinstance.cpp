#ifdef _WIN32
#include <process.h>
#include <windows.h>
#define delaytime(v)	Sleep(v)
#else
#include <pthread.h>
#define delaytime(v)	usleep((v) * 1000)
#endif

#include <assert.h>
#include "threadinstance.h"
#include "lxnet\base\crosslib.h"

threadinstance::threadinstance()
{
	m_delay = 10;
	m_run = false;
	m_exitok = true;
	m_ffunc = 0;
	m_initfunc = 0;
	m_dofunc = 0;
	m_procfunc = 0;
	m_somerun = 0;
	m_lastrun = 0;
}

threadinstance::~threadinstance()
{
	Destroy();
}

//初始化
// delay 帧间隔，单位：毫秒
// initf 线程开启时执行的函数
// ffunc 指定释放任务的函数
// dfunc 指定执行任务的函数。 若任务函数返回true，则返回后释放此任务，并不把任务压缩结果队列。 否则压入结果队列，等待进一步处理
// procfunc 指定处理任务结果的函数
bool threadinstance::Init (int delay, OnInitFunc initf, freetask ffunc, dofunc dfunc, processresult procfunc)
{
	if (!ffunc || !dfunc || !procfunc)
		return false;

	if (delay < 0)
		m_delay = 10;
	else
		m_delay = delay;

	if (!m_tasklist.init(ffunc) || !m_resultlist.init(ffunc))
		return false;
	m_ffunc = ffunc;
	m_initfunc = initf;
	m_dofunc = dfunc;
	m_procfunc = procfunc;
	m_run = true;
	return true;
}

//runfunc 每帧run的时候调用的回调
void threadinstance::SetRunSomeFunc (runsomcallback runfunc)
{
	m_somerun = runfunc;
}

void threadinstance::RunStep ()
{
	if (m_somerun)
		m_somerun();

	void *task;
	for (;;)
	{
		task = m_tasklist.poptask();
		if (!task)
			break;
		if (m_dofunc(task))
			m_ffunc(task);
		else
			m_resultlist.pushtask(task);
	}
}

void threadinstance::Destroy ()
{
	//清理掉任务
	m_tasklist.destroy();
	m_resultlist.destroy();

	//设置退出
	m_run = false;

	//等待退出
	while (!m_exitok)
	{
		delaytime(10);
	}
}

void threadinstance::Run ()
{
	static const int maxdelay = m_delay;
	int64 delay;
	int64 currenttime;
	if (m_initfunc)
		m_initfunc();

	m_exitok = false;
	while (m_run)
	{
		currenttime = get_microsecond();
		RunStep();
		delay = get_microsecond() - currenttime;
		if (delay < maxdelay)
			delaytime(static_cast<DWORD>(maxdelay - delay));
	}
	m_exitok = true;
}

bool threadinstance::PushTask (void *logicdata)
{
	return m_tasklist.pushtask(logicdata);
}

//在主逻辑线程里调用此函数
void threadinstance::InLogicThreadRun ()
{
	void *task;
	for (;;)
	{
		task = m_resultlist.poptask();
		if (!task)
			break;
		m_procfunc(task, m_ffunc);
	}
}

#ifdef WIN32
static void thiread_ran(void *data)
#else
static void *thiread_ran(void *data)
#endif
{
	assert(data);
	threadinstance *_con = (threadinstance*)data;
	_con->Run();
#ifndef WIN32
	return NULL;
#endif
}

//开启一个任务线程
void StartTaskQueueThread (threadinstance *instance)
{
	//开启逻辑线程
#ifdef WIN32
	_beginthread(thiread_ran, 0, instance);
#else
	pthread_t thandle;
	pthread_create(&thandle, 0, thiread_ran, (void*)instance);
#endif

}

