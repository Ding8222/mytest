#include <stdlib.h>
#include <list>
#include "lxnet\base\cthread.h"
#include "taskqueue.h"

class locklist
{
public:
	locklist();
	~locklist();
	bool init (freetask func);
	bool pushtask (void *task);
	void *poptask ();
	size_t size() { return m_list.size(); }
	size_t maxsize() { return m_maxsize; }
private:
	freetask m_func;
	cspin m_lock;
	std::list<void*> m_list;
	size_t m_maxsize;
};

locklist::locklist()
{
	m_func = NULL;
	cspin_init(&m_lock);
	m_list.clear();
	m_maxsize = 0;
}

locklist::~locklist()
{
	cspin_lock(&m_lock);
	for (std::list<void*>::iterator itr = m_list.begin(); itr != m_list.end(); ++itr)
	{
		m_func(*itr);
	}
	m_list.clear();
	cspin_unlock(&m_lock);
	m_func = NULL;
	cspin_destroy(&m_lock);
	m_maxsize = 0;
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
	cspin_lock(&m_lock);
	m_list.push_back(task);
	if (m_maxsize < m_list.size())
		m_maxsize = m_list.size();
	cspin_unlock(&m_lock);
	return true;
}

void *locklist::poptask ()
{
	cspin_lock(&m_lock);
	void *task = NULL;
	if (!m_list.empty())
	{
		task = m_list.front();
		m_list.pop_front();
	}
	cspin_unlock(&m_lock);
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

size_t taskqueue::size()
{
	return m_list->size();
}

size_t taskqueue::maxsize()
{
	return m_list->maxsize();
}