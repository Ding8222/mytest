#pragma once

#include "fmt/ostream.h"
#include "serverlog.h"
#include <stdexcept>
#include <string>
#include <vector>
#include <list>
#include <sstream>

namespace CSV
{
	class Error : public std::runtime_error
	{

	public:
		Error(const std::string &msg) :
			std::runtime_error(std::string("").append(msg))
		{
		}
	};

	class Row
	{
	public:
		Row(const std::vector<std::string> &);
		~Row(void);

	public:
		unsigned int size(void) const;
		void push(const std::string &);
		bool set(const std::string &, const std::string &);

	private:
		const std::vector<std::string> _header;
		std::vector<std::string> _values;

	public:

		template<typename T>
		const T getValue(unsigned int pos) const
		{
			if (pos < _values.size())
			{
				T res;
				std::stringstream ss;
				ss << _values[pos];
				ss >> res;
				return res;
			}
			throw Error("can't return this value (doesn't exist)");
		}

		template<typename T>
		const T getValue(std::string key) const
		{
			std::vector<std::string>::const_iterator it;
			int pos = 0;

			for (it = _header.begin(); it != _header.end(); ++it)
			{
				if (key == *it)
				{
					T res;
					std::stringstream ss;
					ss << _values[pos];
					ss >> res;
					return res;
				}
				pos++;
			}

			throw Error("can't return this value (doesn't exist)");
		}

		template<typename T>
		const void getValue(T &_Ret, const std::string & key) const
		{
			std::vector<std::string>::const_iterator it;
			int pos = 0;
			for (it = _header.begin(); it != _header.end(); ++it)
			{
				if (key == *it)
				{
					std::stringstream ss;
					ss << _values[pos];
					ss >> _Ret;
					return;
				}
				pos++;
			}

			throw Error(fmt::format("没有找到字段{0}", key));
		}

		const std::string operator[](unsigned int) const;
		const std::string operator[](const std::string &valueName) const;
		friend std::ostream& operator<<(std::ostream& os, const Row &row);
		friend std::ofstream& operator<<(std::ofstream& os, const Row &row);
	};

	enum DataType {
		eFILE = 0,
		ePURE = 1
	};

	class Parser
	{

	public:
		Parser(const std::string &, const DataType &type = eFILE, char sep = ',');
		~Parser(void);

	public:
		Row & getRow(unsigned int row) const;
		unsigned int rowCount(void) const;
		unsigned int columnCount(void) const;
		std::vector<std::string> getHeader(void) const;
		const std::string getHeaderElement(unsigned int pos) const;
		const std::string &getFileName(void) const;

	public:
		bool deleteRow(unsigned int row);
		bool addRow(unsigned int pos, const std::vector<std::string> &);
		void sync(void) const;

	protected:
		void parseHeader(void);
		void parseContent(void);

	private:
		std::string _file;
		const DataType _type;
		const char _sep;
		std::vector<std::string> _originalFile;
		std::vector<std::string> _header;
		std::vector<Row *> _content;

	public:
		Row & operator[](unsigned int row) const;
	};

	class CsvLoader
	{
	public:
		template<typename T>
		static bool LoadCsv(const std::string &filename)
		{
			try
			{
				if (filename.empty())
				{
					RunStateError(fmt::format("CSV读取错误，文件名为空!调用类型:{0}",typeid(T).name()).c_str());
					return false;
				}
				
				CSV::Parser file = CSV::Parser(std::string("data/csv/").append(filename));
				for (size_t i = 0; i < file.rowCount(); i++)
				{
					if (!T::AddData(file.getRow(i)))
					{
						throw Error(fmt::format("第{0}行数据读取失败！", i));
					}
				}
			}
			catch (CSV::Error &e)
			{
				RunStateError(fmt::format("CSV文件[{0}]读取错误！{1}", filename, e.what()).c_str());
				return false;
			}
			return true;
		}
	};
}
