
/*
 * Copyright (C) lcinx
 * lcinx@163.com
*/

#include "platform_config.h"
#include "sqlinterface.h"
#include "errmsg.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define my_mkdir _mkdir
#define snprintf _snprintf
#else
#include <sys/stat.h>
#include <sys/types.h>
#define my_mkdir(a) mkdir((a), 0777)
#endif

namespace DataBase{

CRecordset::CRecordset()
{
	m_root = NULL;
	m_mysql_result = NULL;
	m_fields = NULL;
	m_num_fields = 0;

	m_current_row = NULL;
}

CRecordset::~CRecordset()
{
	Reset();
}

//重置
void CRecordset::Reset ()
{
	m_root = NULL;
	if (m_mysql_result)
		mysql_free_result(m_mysql_result);
	m_mysql_result = NULL;
	m_fields = NULL;
	m_num_fields = 0;

	m_current_row = NULL;
}

//装入结果
void CRecordset::LoadResult (MYSQL *con, CConnection *root)
{
	assert(con != NULL);
	assert(root != NULL);
	m_root = root;
	m_mysql_result = mysql_store_result(con);
	if (m_mysql_result)
	{
		//先把列名集以及列数目设置
		m_num_fields = mysql_num_fields(m_mysql_result);
		m_fields = mysql_fetch_fields(m_mysql_result);

		//然后先设置第一行
		m_current_row = mysql_fetch_row(m_mysql_result);
	}
	else
	{
		if (mysql_field_count(con) != 0)
		{
			char tempbuf[1024];
			snprintf(tempbuf, sizeof(tempbuf)-1, "mysql errno: %d, error string: %s", mysql_errno(con), mysql_error(con));
			tempbuf[sizeof(tempbuf)-1] = 0;
			m_root->ErrorOut(tempbuf);
		}
	}
}

int64 CRecordset::GetInt64(const char* fieldname)
{
	return std::stoll(Get(fieldname));
}

int CRecordset::GetInt(const char* fieldname)
{
	return std::stoi(Get(fieldname));
}

float CRecordset::GetFloat(const char* fieldname)
{
	return std::stof(Get(fieldname));
}

double CRecordset::GetDouble(const char* fieldname)
{
	return std::stod(Get(fieldname));
}

const char* CRecordset::GetChar(const char* fieldname)
{
	return Get(fieldname);
}

bool CRecordset::GetBool(const char* fieldname)
{
	return std::stoi(Get(fieldname));
}

//获取字段值
const char* CRecordset::Get (const char* fieldname)
{
	assert(fieldname);
	if (!fieldname)
		return NULL;
	assert(m_fields);
	if (!m_fields)
		return NULL;

	//先获取此字段是第几个字段
	int index;
	for(index = 0; index < m_num_fields; index++)
	{
		if (strcmp(m_fields[index].name, fieldname) == 0)
			return m_current_row[index];
	}
	return NULL;
}

//下一行
void CRecordset::NextRow()
{
	m_current_row = mysql_fetch_row(m_mysql_result);
}

//是否已至最后
bool CRecordset::IsEnd()
{
	return (m_current_row == NULL);
}

//是否打开
bool CRecordset::IsOpen()
{
	return (m_mysql_result != NULL);
}

CConnection::CConnection()
{
	m_dbname = "";
	m_username = "";
	m_password = "";
	m_ip = "";
	m_overtime = 0;
	m_port = 0;

	m_isopen = false;
	m_conn = NULL;

	m_charset = "";

	m_enablelog = true;

	m_errorfunc = NULL;
	m_logfunc = NULL;
	
	m_directoryname = "Log";
	m_lastfilename = "";
	m_fp = NULL;
}

CConnection::~CConnection()
{
	Close();
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

//解析ip:port
//默认端口为 MYSQL_PORT
static unsigned int parse_ipstr (char *tempbuf, size_t buflen, const char *ipstring)
{
	unsigned int port = MYSQL_PORT;
	strncpy(tempbuf, ipstring, buflen-1);
	tempbuf[buflen-1] = 0;

	char *res = strstr(tempbuf, ":");
	if (res)
	{
		*res = '\0';
		port = atoi(res+1);
	}
	return port;
}

//打开数据库 overtime 指定最长多少秒不执行sql语句就断开。
//ipstring 指定ip:port 为字符串，若仅仅是ip，则端口用默认的。
bool CConnection::Open (const char* dbname, const char *username, const char* password, const char* ipstring, int overtime)
{
	char tempbuf[512];
	unsigned int port = parse_ipstr(tempbuf, sizeof(tempbuf), ipstring);

	return Real_Open(dbname, username, password, tempbuf, overtime, port);
}

//当连接成功后，可以设置字符集，如:utf8
bool CConnection::SetCharacterSet (const char *coding)
{
	assert(m_conn != NULL);
	assert(m_isopen == true);
	if (!m_conn || !m_isopen)
		return false;
	if (strcmp(coding, "") == 0)
		return false;

	if (mysql_set_character_set(m_conn, coding) == 0)
	{
		m_charset = coding;
		return true;
	}
	return false;
}

//执行SQL语句返回一个记录集，若执行错误，返回NULL,否则返回一个有效地指针
CRecordset *CConnection::Execute (const char* sqlstr)
{
repeat_do:

	//重置
	m_result.Reset();

	//执行语句
	if (mysql_real_query(m_conn, sqlstr, strlen(sqlstr)) != 0)
	{
		if ((mysql_errno(m_conn) == CR_SERVER_GONE_ERROR) ||
			(mysql_errno(m_conn) == CR_SERVER_LOST))
		{
			//记录下进行了重连
			ErrorOut("Mysql 断开，正在尝试重连...");

			//重连
			if (Real_Open(m_dbname.c_str(), m_username.c_str(), m_password.c_str(), m_ip.c_str(), m_overtime, m_port))
			{
				SetCharacterSet(m_charset.c_str());
				goto repeat_do;
			}
		}

		ErrorOut(sqlstr);

		char buf[1024*2];
		snprintf(buf, sizeof(buf)-1, "errno:%d, error string:%s", mysql_errno(m_conn), mysql_error(m_conn));
		buf[sizeof(buf)-1] = 0;
		ErrorOut(buf);
		return NULL;
	}
	LogOut(sqlstr);
	LogOut("查询成功!");
	m_result.LoadResult(m_conn, this);
	return &m_result;
}

//检测连接是否打开
bool CConnection::IsOpen ()
{
	return m_isopen;
}

//关闭连接
void CConnection::Close ()
{
	m_result.Reset();
	if (m_conn)
	{
		mysql_close(m_conn);
		m_conn = NULL;
	}
	m_isopen = false;
}

//日志相关接口，若未设置相关函数，则会使用默认函数

//设置日志函数
void CConnection::SetLogOut (LogFunc func)
{
	m_logfunc = func;
}

//设置错误函数
void CConnection::SetErrorOut (LogFunc func)
{
	m_errorfunc = func;
}

//日志启用开关
void CConnection::SetEnableLog (bool enablelog)
{
	m_enablelog = enablelog;
}
//设置日志目录
void CConnection::SetLogDirectory (const char *directory)
{
	if (strcmp(directory, "") == 0)
		return;
	m_directoryname = directory;
}

void CConnection::DefaultLogFunc (const char* text)
{
	char szDate[256] = {0};
	char szTime[256] = {0};
	char szFile[256] = {0};
	char szPath[256] = {0};

	time_t tval;
	struct tm *currTM;
	time(&tval);
	currTM = localtime(&tval);

	snprintf(szDate, sizeof(szDate)-1, "%04d-%02d-%02d", currTM->tm_year+1900, currTM->tm_mon+1, currTM->tm_mday);
	snprintf(szTime, sizeof(szTime)-1, "%02d:%02d:%02d", currTM->tm_hour, currTM->tm_min, currTM->tm_sec);

	snprintf(szPath, sizeof(szPath)-1, "%s/%s", m_directoryname.c_str(), szDate);
	my_mkdir(m_directoryname.c_str());
	my_mkdir(szPath);

	snprintf(szFile, sizeof(szFile)-1, "%s/%02d_DB.log", szPath, currTM->tm_hour);

	if (strcmp(m_lastfilename.c_str(), szFile) != 0)
	{
		m_lastfilename = szFile;
		if (m_fp)
		{
			fclose(m_fp);
			m_fp = NULL;
		}
		m_fp = fopen(szFile, "a");
	}
	if (m_fp)
	{
		fprintf(m_fp, "%s %s    %s\n", szDate, szTime, text);
		fflush(m_fp);
	}
}

void CConnection::DefaultErrorFunc (const char *text)
{
	char szFile[256] = {0};

	time_t tval;
	struct tm *currTM;
	time(&tval);
	currTM = localtime(&tval);

	my_mkdir(m_directoryname.c_str());
	snprintf(szFile, sizeof(szFile)-1, "%s/_db_error_.log", m_directoryname.c_str());

	if (strcmp(m_lastfilename.c_str(), szFile) != 0)
	{
		m_lastfilename = szFile;
		if (m_fp)
		{
			fclose(m_fp);
			m_fp = NULL;
		}
		m_fp = fopen(szFile, "a");
	}

	if (m_fp)
	{
		fprintf(m_fp, "%04d-%02d-%02d %02d:%02d:%02d    %s\n", currTM->tm_year+1900, currTM->tm_mon+1, currTM->tm_mday, 
		currTM->tm_hour, currTM->tm_min, currTM->tm_sec, text);
		fflush(m_fp);
	}
}

void CConnection::LogOut (const char *text)
{
	if (m_enablelog)
	{
		if (m_logfunc)
			m_logfunc(text);
		else
			DefaultLogFunc(text);
	}
}

void CConnection::ErrorOut (const char *text)
{
	if (m_errorfunc)
		m_errorfunc(text);
	else
		DefaultErrorFunc(text);
}

//打开数据库 overtime 指定最长多少秒不执行sql语句就断开。
bool CConnection::Real_Open (const char* dbname, const char *username, const char* password, const char* ip, int overtime, unsigned int port)
{
	assert(dbname != NULL);
	assert(username != NULL);
	assert(password != NULL);
	assert(ip != NULL);
	assert(overtime > 0);
	assert(port != 0);
	if (!dbname || !username || !password || !ip || (overtime <= 0) || !port)
		return false;

	m_dbname = dbname;
	m_username = username;
	m_password = password;
	m_ip = ip;
	m_overtime = overtime;
	m_port = port;
	
	Close();

	m_conn = mysql_init(NULL);
	if (!m_conn)
	{
		ErrorOut("Mysql初始化失败!");
		ErrorOut(mysql_error(m_conn));
		return false;
	}
	if (m_conn != mysql_real_connect(m_conn, ip, username, password, dbname, port, NULL, 0))
	{
		ErrorOut("Mysql连接失败!");
		ErrorOut(mysql_error(m_conn));
		return false;
	}
	LogOut("Mysql连接成功!");

	char buf[1024];
	snprintf(buf, sizeof(buf)-1, "set wait_timeout = %d", overtime);
	buf[sizeof(buf)-1] = 0;
	Execute(buf);
	m_isopen = true;
	return true;
}

}



