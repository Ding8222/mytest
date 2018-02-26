/*
* Client登陆管理
* Copyright (C) ddl
* 2018
*/
#pragma once

class CClientLogin
{
public:
	CClientLogin();
	~CClientLogin();

	static CClientLogin &Instance()
	{
		static CClientLogin m;
		return m;
	}

	void Destroy();

	void ProcessLoginMsg(connector *_con, Msg *pMsg, msgtail *tl);

	void ClientAuth(connector *_con, Msg *pMsg, msgtail *tl);
	void GetPlayerList(connector *_con, Msg *pMsg, msgtail *tl);
	void CreatePlayer(connector *_con, Msg *pMsg, msgtail *tl);
	void SelectPlayer(connector *_con, Msg *pMsg, msgtail *tl);
private:

};
