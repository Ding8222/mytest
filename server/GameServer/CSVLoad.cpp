#include "CSVLoad.h"
#include "serverlog.h"

namespace CSVData
{
	bool Init()
	{
		if (!CSV::CsvLoader::LoadCsv<Example>("Example.CSV"))
			return false;

		RunStateError("加载CSV成功！");
		return true;
	}

	void Destroy()
	{
		Example::Destroy();
	}

	std::unordered_map<int64, Example *> Example::m_Data;
	bool Example::AddData(CSV::Row & _Row)
	{
		Example *pdata = new Example;
		_Row.getValue(pdata->nIndex, "序号");
		_Row.getValue(pdata->nGold, "金额");
		_Row.getValue(pdata->text, "描述");

		m_Data.insert(std::make_pair(pdata->nIndex, pdata));
		return true;
	}
}