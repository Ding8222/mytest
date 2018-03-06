#include <stdlib.h>
#include <new>
#include "datahand.h"
#include "threadinstance.h"
#include "serverlog.h"

datahand::datahand ()
{
	m_dbinstance = NULL;
}

datahand::~datahand ()
{
	Destroy();
}

bool datahand::Init (int delay, OnInitFunc initf, freetask ffunc, dofunc dfunc, processresult procfunc, runsomcallback runfunc)
{
	m_dbinstance = (threadinstance *)malloc(sizeof(threadinstance));
	if (!m_dbinstance)
	{
		RunStateError("create threadinstance memory failed!");
		return false;
	}
	new(m_dbinstance) threadinstance();
	if (!m_dbinstance->Init(delay, initf, ffunc, dfunc, procfunc))
	{
		RunStateError("init threadintsace failed!");
		return false;
	}

	m_dbinstance->SetRunSomeFunc(runfunc);

	StartTaskQueueThread(m_dbinstance);
	return true;
}

void datahand::RunOnce ()
{
	m_dbinstance->InLogicThreadRun();
}

bool datahand::PushTask (void *logicdata)
{
	return m_dbinstance->PushTask(logicdata);
}

void datahand::Destroy ()
{
	if (m_dbinstance)
	{
		m_dbinstance->~threadinstance();
		free(m_dbinstance);
		m_dbinstance = NULL;
	}
}

datahand *datahand_create ()
{
	datahand *obj = (datahand *)malloc(sizeof(datahand));
	if (!obj)
		return NULL;
	new(obj) datahand();
	return obj;
}

void datahand_release (datahand *obj)
{
	if (!obj)
		return;

	obj->~datahand();
	free(obj);
}

