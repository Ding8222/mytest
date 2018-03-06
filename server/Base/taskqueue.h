
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
private:
	locklist *m_list;
};
