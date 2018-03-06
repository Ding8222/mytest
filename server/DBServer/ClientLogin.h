﻿/*
* Client登陆管理
* Copyright (C) ddl
* 2018
*/
#pragma once

class task;
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

	void ProcessLoginMsg(task *tk, Msg *pMsg);

	void ClientAuth(task *tk, Msg *pMsg);
	void GetPlayerList(task *tk, Msg *pMsg);
	void CreatePlayer(task *tk, Msg *pMsg);
	void SelectPlayer(task *tk, Msg *pMsg);
private:

};
