
#pragma once

typedef void (*freetask) (void *task);
typedef bool (*dofunc) (void *logicdata);
typedef void (*processresult) (void *logicdata, freetask ffunc);
typedef void (*runsomcallback) ();
typedef void (*FuncOnConnect) ();
typedef void (*FuncOnDisconnect) ();

struct Msg;
typedef void (*FuncProcessMsg) (Msg *pMsg);
typedef void (*OnInitFunc) ();
