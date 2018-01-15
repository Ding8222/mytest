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
	// ��������
	bool LoadData();
	// ��������
	bool SaveData();
	// ����
	void OffLine();
private:

	CClient *m_Client;
};