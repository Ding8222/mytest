#pragma once

#include "platform_config.h"
#include "log.h"

extern int64 g_currenttime;
extern struct filelog *g_client_connectlog;
extern struct filelog *g_elapsed_log;
extern struct filelog *g_backcommand_log;

extern bool g_client_connectlog_flag;
extern bool g_elapsed_log_flag;

// client连接log
#define ClientConnectLog(fmt, ...) \
do{if(g_client_connectlog && g_client_connectlog_flag)\
{char temp_buf[32] = { 0 };\
_format_prefix_string_(0, -1, temp_buf, sizeof(temp_buf),\
0, 0, 0);\
_log_printf_(enum_debug_print_time,\
fmt, temp_buf, ##__VA_ARGS__, __END__ARG__FLAG__);\
filelog_write_log(g_client_connectlog, fmt, ##__VA_ARGS__);}\
}while(0)

// client连接出错log
#define ClientConnectError(fmt, ...) \
do{if(g_client_connectlog && g_client_connectlog_flag)\
{char temp_buf[32] = { 0 };\
_format_prefix_string_(0, -1, temp_buf, sizeof(temp_buf),\
0, 0, 0);\
_log_printf_(enum_debug_print_time,\
fmt, temp_buf, ##__VA_ARGS__, __END__ARG__FLAG__);\
filelog_write_error(g_client_connectlog, fmt, ##__VA_ARGS__);}\
}while(0)

// 运行时log
#define RunStateLog(fmt, ...) \
do{log_writelog(fmt, ##__VA_ARGS__);\
}while(0)

// 运行时错误log
#define RunStateError(fmt, ...) \
do{log_error(fmt, ##__VA_ARGS__);\
}while(0)

// 超时log
#define ElapsedLog(fmt, ...) \
do{if(g_elapsed_log && g_elapsed_log_flag)\
{filelog_write_log(g_elapsed_log, fmt, ##__VA_ARGS__);}\
}while(0)

// 超时错误log
#define ElapsedError(fmt, ...) \
do{if(g_elapsed_log && g_elapsed_log_flag)\
{filelog_write_error(g_elapsed_log, fmt, ##__VA_ARGS__);}\
}while(0)

// 后台命令log
#define BackCommandLog(fmt, ...) \
do{if(g_backcommand_log)\
{filelog_write_log(g_backcommand_log, fmt, ##__VA_ARGS__);}\
}while(0)

bool init_log(const char *rootdirname);
void release_log();
