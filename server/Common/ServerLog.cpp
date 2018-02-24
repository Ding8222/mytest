#include "serverlog.h"
#include <string>

struct filelog *g_client_connectlog = NULL;
struct filelog *g_elapsed_log = NULL;
struct filelog *g_backcommand_log = NULL;

bool g_client_connectlog_flag = false;
bool g_elapsed_log_flag = false;

bool init_log(const char *rootdirname)
{
	std::string tempname = rootdirname;
	tempname = "log_log/" + tempname;
	std::string temp;
	if (tempname == "")
		return false;

	mymkdir_r(tempname.c_str());

	if (!g_client_connectlog)
	{
		g_client_connectlog = filelog_create();
		temp = tempname + "/client_connectlog";
		filelog_set_directory(g_client_connectlog, temp.c_str());
	}

	temp = tempname + "/runstate_log";
	log_set_directory(temp.c_str());

	if (!g_elapsed_log)
	{
		g_elapsed_log = filelog_create();
		temp = tempname + "/elapsed_log";
		filelog_set_directory(g_elapsed_log, temp.c_str());
	}

	if (!g_backcommand_log)
	{
		g_backcommand_log = filelog_create();
		temp = tempname + "/backcommand_log";
		filelog_set_directory(g_backcommand_log, temp.c_str());
	}

	return true;
}

void release_log()
{
	if (g_client_connectlog)
	{
		filelog_release(g_client_connectlog);
		g_client_connectlog = NULL;
	}

	if (g_elapsed_log)
	{
		filelog_release(g_elapsed_log);
		g_elapsed_log = NULL;
	}
}

