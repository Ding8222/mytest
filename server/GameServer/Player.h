#pragma once
#include<string>
#include<unordered_map>
#include "BaseObj.h"

class CClient;
class scene;
struct Msg;
class CPlayer :public CBaseObj
{
public:
	CPlayer();
	~CPlayer();

	void Run();
	// 加载数据
	bool LoadData(Msg *pMsg);
	// 保存数据
	bool SaveData();
	// 下线
	void OffLine();

private:

};