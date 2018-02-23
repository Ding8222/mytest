#pragma once

#include "platform_config.h"
#include "log.h"

extern int64 g_currenttime;
extern struct filelog *g_client_connectlog;
extern struct filelog *g_elapsed_log;
extern struct filelog *g_backcommand_log;

extern bool g_client_connectlog_flag;
extern bool g_elapsed_log_flag;

#define ClientConnectLog(...) \
do{if(g_client_connectlog && g_client_connectlog_flag)\
{filelog_writelog(g_client_connectlog, __VA_ARGS__);}\
}while(0)

#define ClientConnectError(...) \
do{if(g_client_connectlog && g_client_connectlog_flag)\
{filelog_error(g_client_connectlog, __VA_ARGS__);}\
}while(0)


#define RunStateLog(...) \
do{log_writelog(__VA_ARGS__);\
}while(0)

#define RunStateError(...) \
do{log_error(__VA_ARGS__);\
}while(0)

#define ElapsedLog(...) \
do{if(g_elapsed_log && g_elapsed_log_flag)\
{filelog_writelog(g_elapsed_log, __VA_ARGS__);}\
}while(0)

#define ElapsedError(...) \
do{if(g_elapsed_log && g_elapsed_log_flag)\
{filelog_error(g_elapsed_log, __VA_ARGS__);}\
}while(0)

#define BackCommandLog(...) \
do{if(g_backcommand_log)\
{filelog_writelog(g_backcommand_log, __VA_ARGS__);}\
}while(0)

bool init_log(const char *rootdirname);
void release_log();
