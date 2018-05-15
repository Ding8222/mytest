
#pragma once
#include "funcdef.h"
class locklist;
class taskqueue
{
public:
	taskqueue();
	~taskqueue();
	bool init (freetask func);
	void destroy ();
	bool pushtask (void *task);
	void *poptask ();
	size_t size();
	size_t maxsize();
private:
	locklist *m_list;
};
