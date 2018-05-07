#pragma once
#include <string>
#include <list>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "pool.h"


class objectpoolbase
{
public:
	objectpoolbase () {}
	virtual ~objectpoolbase () {}
	virtual void writeinfotofile (FILE *fp) = 0;
};

class objectmeminfo
{
public:
	objectmeminfo ()
	{
		m_filename = "pools_meminfo_log.txt";
		m_infolist.clear();
		m_otherstr.clear();
	}
	~objectmeminfo ()
	{
		m_infolist.clear();
	}
	static objectmeminfo & Instance()
	{
		static objectmeminfo m;
		return m;
	}
	void AddInfo (objectpoolbase *info)
	{
		if (IsIn(info))
		{
			assert(false && "already in info list!");
			return;
		}
		m_infolist.push_back(info);
	}
	void RemoveInfo (objectpoolbase *info)
	{
		if (!IsIn(info))
		{
			assert(false && "why not in info list!");
			return;
		}
		m_infolist.remove(info);
	}
	void writeinfotofile ()
	{
		FILE *fp = fopen(m_filename.c_str(), "a");
		if (!fp)
			return;

		time_t tval;
		time(&tval);
		struct tm *currTM = localtime(&tval);
		fprintf(fp, "memory pool info data:	[%d/%02d/%02d %02d:%02d:%02d]\n", currTM->tm_year+1900, currTM->tm_mon+1, currTM->tm_mday, currTM->tm_hour, currTM->tm_min, currTM->tm_sec);
		fprintf(fp, "<+++++++++++++++++++++++++++++++++++++++++++++++++++++>\n");
		for (std::list<objectpoolbase *>::iterator itr = m_infolist.begin(); itr != m_infolist.end(); ++itr)
		{
			fprintf(fp, "\n");
			(*itr)->writeinfotofile(fp);
			fprintf(fp, "\n");
		}
		for (std::list<std::string>::iterator itr = m_otherstr.begin(); itr != m_otherstr.end(); ++itr)
		{
			fprintf(fp, "\n%s\n", (*itr).c_str());
		}
		m_otherstr.clear();
		fprintf(fp, "<+++++++++++++++++++++++++++++++++++++++++++++++++++++>\n\n\n\n");
		fclose(fp);
	}
	void AddOtherString (const char *str)
	{
		if (str)
		{
			m_otherstr.push_back(str);
		}
	}
	void SetMeminfoFileName (const char *filename)
	{
		assert(filename != NULL);
		if (!filename)
			return;
		if (strcmp(filename, "") == 0)
			return;
		m_filename = filename;
	}
private:
	bool IsIn (objectpoolbase *info)
	{
		for (std::list<objectpoolbase *>::iterator itr = m_infolist.begin(); itr != m_infolist.end(); ++itr)
		{
			if ((*itr) == info)
				return true;
		}
		return false;
	}
private:
	std::string m_filename;
	std::list<objectpoolbase *> m_infolist;
	std::list<std::string> m_otherstr;
};
#define sPoolInfo objectmeminfo::Instance()

template<class T>
class objectpool:public objectpoolbase
{
private:
	size_t m_num;
	const char *m_name;
	struct poolmgr *m_pool;
private:
	void CheckAndCreate ()
	{
		if (!m_pool)
		{
			m_pool = poolmgr_create(sizeof(T), 8, m_num, 1, m_name);
		}
	}
public:
	objectpool (size_t num, const char *name)
	{
		m_num = num;
		m_name = name;
		m_pool = NULL;
		sPoolInfo.AddInfo(this);
	}

	~objectpool ()
	{
		sPoolInfo.RemoveInfo(this);
		if (m_pool)
		{
			poolmgr_release(m_pool);
			m_pool = NULL;
		}
	}

	T *GetObject ()
	{
		CheckAndCreate();
		return (T*)poolmgr_alloc_object(m_pool);
	}
	
	void FreeObject (T *obj)
	{
		CheckAndCreate();
		poolmgr_free_object(m_pool, obj);
	}
	
	void writeinfotofile (FILE *fp)
	{
		assert(fp != NULL);
		if (!fp)
			return;
		CheckAndCreate();
		char buf[1024*16];
		poolmgr_get_info(m_pool, buf, sizeof(buf)-1);
		buf[sizeof(buf)-1] = 0;
		fprintf(fp, "%s", buf);
	}
};
