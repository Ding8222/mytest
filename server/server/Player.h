#pragma once
#include<string>
#include<unordered_map>
#include "BaseObj.h"

class CClient;
class scene;

class CPlayer :public CBaseObj
{
public:
	CPlayer(CClient* _client);
	~CPlayer();

	void Run();
	// 加载数据
	bool LoadData();
	// 保存数据
	bool SaveData();
	// 下线
	void OffLine();
private:

	CClient *m_Client;
};