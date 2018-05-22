#include <fstream>
#include "NetConfig.h"
#include "log.h"
#include "json.hpp"

CNetConfig::CNetConfig()
{
	m_BigBufSize = 0;
	m_BigBufNum = 0;
	m_SmallBufSize = 0;
	m_SmallBufNum = 0;
	m_ListenerNum = 0;
	m_SocketerNum = 0;
	m_ThreadNum = 0;
}

CNetConfig::~CNetConfig()
{

}

bool CNetConfig::Init()
{
	std::string filename = "./config/Network.json";
	std::ifstream i(filename);
	if (!i.is_open())
	{
		log_error("加载 %s 失败!", filename.c_str());
		return false;
	}

	nlohmann::json config = nlohmann::json::parse(i);

	if (config["BigBufSize"].is_null())
	{
		log_error("没有找到字段： 'BigBufSize'");
		return false;
	}
	m_BigBufSize = config["BigBufSize"];
	if (m_BigBufSize <= 0)
	{
		log_error("BigBufSize小于等于0：%d", m_BigBufSize);
		return false;
	}

	if (config["BigBufNum"].is_null())
	{
		log_error("没有找到字段： 'BigBufNum'");
		return false;
	}
	m_BigBufNum = config["BigBufNum"];
	if (m_BigBufNum <= 0)
	{
		log_error("BigBufSize小于等于0：%d", m_BigBufNum);
		return false;
	}

	if (config["SmallBufSize"].is_null())
	{
		log_error("没有找到字段： 'SmallBufSize'");
		return false;
	}
	m_SmallBufSize = config["SmallBufSize"];
	if (m_SmallBufSize <= 0)
	{
		log_error("SmallBufSize小于等于0：%d", m_SmallBufSize);
		return false;
	}

	if (config["SmallBufNum"].is_null())
	{
		log_error("没有找到字段： 'SmallBufNum'");
		return false;
	}
	m_SmallBufNum = config["SmallBufNum"];
	if (m_SmallBufNum <= 0)
	{
		log_error("SmallBufNum小于等于0：%d", m_SmallBufNum);
		return false;
	}

	if (config["ListenerNum"].is_null())
	{
		log_error("没有找到字段： 'ListenerNum'");
		return false;
	}
	m_ListenerNum = config["ListenerNum"];
	if (m_ListenerNum <= 0)
	{
		log_error("ListenerNum小于等于0：%d", m_ListenerNum);
		return false;
	}

	if (config["SocketerNum"].is_null())
	{
		log_error("没有找到字段： 'SocketerNum'");
		return false;
	}
	m_SocketerNum = config["SocketerNum"];
	if (m_SocketerNum <= 0)
	{
		log_error("SocketerNum小于等于0：%d", m_SocketerNum);
		return false;
	}

	if (config["ThreadNum"].is_null())
	{
		log_error("没有找到字段： 'ThreadNum'");
		return false;
	}
	m_ThreadNum = config["ThreadNum"];
	if (m_ThreadNum < 0)
	{
		log_error("ThreadNum小于0：%d", m_ThreadNum);
		return false;
	}

	i.close();
	config.clear();
	return true;
}
