#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#endif
#include "mysql.h"
#include <string>
#include <stdio.h>
#include "platform_config.h"

namespace DataBase
{
	class CConnection;
	class CRecordset
	{
	public:
		CRecordset();
		~CRecordset();

	//外部模块请勿调用。
	public:
		//重置
		void Reset ();

		//装入结果
		void LoadResult (MYSQL *con, CConnection *root);

	public:
		int64		GetInt64(const char* fieldname);
		int32		GetInt(const char* fieldname);
		float		GetFloat(const char* fieldname);
		double		GetDouble(const char* fieldname);
		const char*	GetChar(const char* fieldname);
		bool		GetBool(const char* fieldname);
	//对外的接口，其他模块可用
	public:

		//获取字段值
		const char* Get (const char* fieldname);

		//下一行
		void NextRow();

		//是否已至最后
		bool IsEnd();

		//是否打开
		bool IsOpen();

		int64 GetRowCount() { return m_num_rows; }
	private:
		CConnection *m_root;
		MYSQL_RES *m_mysql_result;	//结果集
		MYSQL_FIELD *m_fields;		//列名字数组
		int32 m_num_fields;			//列数目
		int64 m_num_rows;				//行数目

		MYSQL_ROW m_current_row;	//当前行
	};

	class CConnection
	{
	public:
		CConnection();
		~CConnection();
		
		//日志函数
		typedef void (*LogFunc) (const char*); 

		//打开数据库 overtime 指定最长多少秒不执行sql语句就断开。
		//ipstring 指定ip:port 为字符串，若仅仅是ip，则端口用默认的。
		bool Open (const char* dbname, const char *username, const char* password, const char* ipstring, int32 overtime = 7*24*3600);

		//当连接成功后，可以设置字符集，如:utf8
		bool SetCharacterSet (const char *coding);

		//执行SQL语句返回一个记录集，若执行错误，返回NULL,否则返回一个有效地指针
		CRecordset *Execute (const char* sqlstr);

		//检测连接是否打开
		bool IsOpen ();
		
		//关闭连接
		void Close ();


//日志相关接口，若未设置相关函数，则会使用默认函数
//默认为开启日志

		//设置日志函数
		void SetLogOut (LogFunc func);

		//设置错误函数
		void SetErrorOut (LogFunc func);

		//日志启用开关
		void SetEnableLog (bool enablelog);

		//设置日志目录
		void SetLogDirectory (const char *directory);
	private:
		void DefaultLogFunc (const char* text);
		void DefaultErrorFunc (const char *text);

		void LogOut (const char *text);
	public:
		void ErrorOut (const char *text);
	private:
		//打开数据库 overtime 指定最长多少秒不执行sql语句就断开。
		bool Real_Open (const char* dbname, const char *username, const char* password, const char* ip, int32 overtime, uint32 port);
	private:
		//记录一些连接参数，重连时使用
		std::string m_dbname;
		std::string m_username;
		std::string m_password;
		std::string m_ip;
		int32 m_overtime;
		uint32 m_port;
	private:
		//与数据库交互相关
		bool m_isopen;				//连接状态

		MYSQL *m_conn;				//连接句柄
		CRecordset m_result;		//执行查询的结果。

		std::string m_charset;		//链接时设置的字符集，存储下来，为了重连时设置

		//日志相关
		bool m_enablelog;			//启用日志开关

		//若指定日志函数，则不使用默认的
		LogFunc m_errorfunc;		//错误函数
		LogFunc m_logfunc;			//日志函数

		//默认日志相关
		std::string m_directoryname;//目录名
		std::string m_lastfilename;	//最后打开的文件
		FILE *m_fp;
	};
}
