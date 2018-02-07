#pragma once

class CLoginMsg
{
public:
	CLoginMsg();
	~CLoginMsg();

	static CLoginMsg &Instance()
	{
		static CLoginMsg m;
		return m;
	}

	void ProcessMsg();
private:

};
