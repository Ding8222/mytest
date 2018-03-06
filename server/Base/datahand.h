
#pragma once
#include "funcdef.h"

class threadinstance;
class datahand
{
public:
	datahand ();
	~datahand ();

	bool Init (int delay, OnInitFunc initf, freetask ffunc, dofunc dfunc, processresult procfunc, runsomcallback runfunc);
	void RunOnce ();
	bool PushTask (void *logicdata);
	void Destroy ();
private:
	threadinstance *m_dbinstance;
};

datahand *datahand_create ();
void datahand_release (datahand *obj);
