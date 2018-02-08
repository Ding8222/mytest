#pragma once
#include<unordered_map>

struct idmgr;
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
	void Destroy();
	void Run();

	void CheckAndRemove();
	bool AddScene(CMapInfo* mapconfig);
	int AddInstance(CMapInfo* mapconfig);
	void DelInstance(int instanceid);
	CScene *GetScene(int mapid);
private:
	void ReleaseInstanceAndID(CScene *scene);
private:

	// mapid,scene
	std::unordered_map<int ,CScene *> m_SceneMap;

	//副本
	std::unordered_map<int, CScene *> m_InstanceMap;
	std::list<CScene *> m_WaitRemove;
	std::vector<CScene *> m_InstanceSet;
	idmgr *m_IDPool;
};