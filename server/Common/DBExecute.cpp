// #include "json.hpp"
// static DataBase::CConnection g_dbhand;
// json LoadData(int64 uid)
// {
// 	const char * tbname;
// 	const char * columns;
// 	const char * pk;
// 	int offset = 0;
// 	const char * sql;
// 	json data;
// 	while (true)
// 	{
// 		if (uid)
// 		{
// 			if (columns)
// 			{
// 				sql = string.format("select %s from %s order by %s asc limit %d, 1000", config.columns, tbname, pk, offset)
// 			}
// 			else
// 			{
// 				sql = string.format("select * from %s order by %s asc limit %d, 1000", tbname, pk, offset)
// 			}
// 		}
// 		else
// 		{
// 			if (columns)
// 			{
// 				sql = string.format("select %s from %s where uid = '%s' order by %s asc limit %d, 1000", config.columns, tbname, uid, pk, offset)
// 			}
// 			else
// 			{
// 				sql = string.format("select * from %s where uid = '%s' order by %s asc limit %d, 1000", tbname, uid, pk, offset)
// 			}
// 		}
// 
// 		DataBase::CRecordset *res = g_dbhand.Execute(msg.sql().c_str());
// 		if (res && res->IsOpen())
// 		{
// 			while (res->IsEnd())
// 			{
// 				data = res->Get();
// 				res->NextRow();
// 			}
// 		}
// 		else
// 			break;
// 
// 		offset = offset + 1000
// 	}
// 	return data;
// }
// 
// json LoadSingleByUid(const char * tbName, int64 uid)
// {
// 	json config = DBTableConfig[tbName];
// 	json data = LoadData(config, uid);
// 	if (data.length() == 1)
// 	{
// 		return data[1];
// 	}
// 	return data;
// }
// 
// // 单句查询
// void ExecuteSingle(const char * tbName,int64 uid,json fields)
// {
// 	json data;
// 	json t = LoadSingleByUid(tbName, uid);
// 	if (fields.empty())
// 	{
// 		data = t;
// 	}
// 	else
// 	{
// 		for (auto &i : fields)
// 		{
// 			data[i.name] = t[i.name];
// 		}
// 	}
// 
// 	return data;
// }
// 
// json LoadMultiByUid(const char * tbName, int64 uid)
// {
// 	json config = DBTableConfig[tbName];
// 	json data = LoadData(config, uid);
// 	const char * pk = schema[const char * tbName]["pk"];
// 	for (auto &i : data)
// 	{
// 		data[i[pk]] = i;
// 	}
// 	return data;
// }
// 
// // 多句查询
// void ExecuteMulti(const char * tbName, int64 uid,int64 id, json fields)
// {
// 	json data;
// 	json t = LoadMultiByUid(tbName, uid);
// 
// 	if (id)
// 	{
// 		if (fields)
// 		{
// 			for (auto &i : fields)
// 			{
// 				data[i.name] = t[id][i.name];
// 			}
// 		}
// 		else
// 		{
// 			data = t[id];
// 		}
// 	}
// 	else
// 	{
// 		if (fields)
// 		{
// 			for (auto &i : t)
// 			{
// 				for (int i = 0; i < fields.size(); i++)
// 				{
// 					data[i.id][fields[i]] = t[i.id][fields[i]];
// 				}
// 			}
// 		}
// 		else
// 		{
// 			data = t;
// 		}
// 	}
// 	return data;
// }