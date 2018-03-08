#include "CSVLoad.h"
#include "serverlog.h"
#include "fmt/ostream.h"

namespace CSVData
{
	// 读取CSV
	bool Init()
	{
		if (!CSV::CsvLoader::LoadCsv<CExample>("Example"))
			return false;
		
		RunStateLog("加载所有CSV成功！");
		return true;
	}

	// 需要释放申请的资源
	void Destroy()
	{
		CExample::Destroy();
	}

	std::unordered_map<int64, stExample *> CExample::m_Data;
	bool CExample::AddData(CSV::Row & _Row)
	{
		stExample *pdata = new stExample;
		_Row.getValue(pdata->nIndex, "序号");
		_Row.getValue(pdata->nGold, "金额");
		_Row.getValue(pdata->text, "描述");

		m_Data.insert(std::make_pair(pdata->nIndex, pdata));
		return true;
	}
}