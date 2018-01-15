#pragma once
#include<unordered_map>

class CScene;
class CMapInfo;

class CScenemgr
{
public:
	CScenemgr();
	~CScenemgr();

	static CScenemgr &Instance()
	{
		static CScenemgr m;
		return m;
	}

	bool Init();
	void Run();
	bool LoadScene(CMapInfo* mapconfig);
	CScene *CreateScene(CMapInfo* mapconfig);
	CScene *GetScene(int mapid);
private:

	std::unordered_map<int ,CScene *> m_SceneList;
};