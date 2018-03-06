#include <stdlib.h>
#include <list>
#include "ossome.h"
#include "taskqueue.h"

class locklist
{
public:
	locklist();
	~locklist();
	bool init (freetask func);
	bool pushtask (void *task);
	void *poptask ();
private:
	freetask m_func;
	LOCK_struct m_lock;
	std::list<void*> m_list;
};

locklist::locklist()
{
	m_func = NULL;
	LOCK_INIT(&m_lock);
	m_list.clear();
}

locklist::~locklist()
{
	LOCK_LOCK(&m_lock);
	for (std::list<void*>::iterator itr = m_list.begin(); itr != m_list.end(); ++itr)
	{
		m_func(*itr);
	}
	m_list.clear();
	LOCK_UNLOCK(&m_lock);
	m_func = NULL;
	LOCK_DELETE(&m_lock);
}

bool locklist::init (freetask func)
{
	if (!func)
		return false;
	m_func = func;
	return true;
}

bool locklist::pushtask (void *task)
{
	if (!task)
		return false;
	LOCK_LOCK(&m_lock);
	m_list.push_back(task);
	LOCK_UNLOCK(&m_lock);
	return true;
}

void *locklist::poptask ()
{
	LOCK_LOCK(&m_lock);
	void *task = NULL;
	if (!m_list.empty())
	{
		task = m_list.front();
		m_list.pop_front();
	}
	LOCK_UNLOCK(&m_lock);
	return task;
}

taskqueue::taskqueue()
{
	m_list = NULL;
}

taskqueue::~taskqueue()
{
	destroy();
}

bool taskqueue::init (freetask func)
{
	if (m_list)
		return false;
	m_list = (locklist *)malloc(sizeof(locklist));
	if (!m_list)
		return false;
	new(m_list) locklist();
	return m_list->init(func);
}

void taskqueue::destroy ()
{
	if (m_list)
	{
		m_list->~locklist();
		free(m_list);
		m_list = NULL;
	}
}

bool taskqueue::pushtask (void *task)
{
	return m_list->pushtask(task);
}

void *taskqueue::poptask ()
{
	return m_list->poptask();
}

