
#pragma once
#include "taskqueue.h"
#include "lxnet\base\crosslib.h"

class threadinstance
{
public:
	threadinstance();
	~threadinstance();

	// 帧间隔，单位：毫秒
	// initf 线程开启时执行的函数
	// ffunc 指定释放任务的函数
	// dfunc 指定执行任务的函数。 若任务函数返回true，则返回后释放此任务，并不把任务压缩结果队列； 否则压入结果队列，等待进一步处理
	// procfunc 指定处理任务结果的函数
	bool Init (int delay, OnInitFunc initf, freetask ffunc, dofunc dfunc, processresult procfunc);
	
	//runfunc 每帧run的时候调用的回调
	void SetRunSomeFunc (runsomcallback runfunc);

	void Run ();
	bool PushTask (void *logicdata);
	
	//在主逻辑线程里调用此函数
	void InLogicThreadRun ();

	size_t tasksize() { return m_tasklist.size(); }
	size_t taskmaxsize() { return m_tasklist.maxsize(); }
	size_t resultsize() { return m_resultlist.size(); }
	size_t resultmaxsize() { return m_resultlist.maxsize(); }
private:
	void RunStep ();
	void Destroy ();
private:
	int m_delay;					//帧间隔
	volatile bool m_run;			//线程运行标记
	volatile bool m_exitok;			//成功退出标记

	taskqueue m_tasklist;			//任务队列
	taskqueue m_resultlist;			//任务结果集

	OnInitFunc m_initfunc;			//初始化时的函数
	freetask m_ffunc;				//释放任务的函数
	dofunc m_dofunc;				//执行任务的函数
	processresult m_procfunc;		//处理任务结果的函数
	runsomcallback m_somerun;
	int64 m_lastrun;	
};

//开启一个任务线程
void StartTaskQueueThread (threadinstance *instance);
