#include"NetConfig.h"
#include"tinyxml2.h"
#include"log.h"

using namespace tinyxml2;

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
	const char *filename = "./config/network.xml";
	XMLDocument doc;
	if (doc.LoadFile(filename) != XML_SUCCESS)
	{
		log_error("加载 %s 失败!", filename);
		return false;
	}

	XMLElement *pBaseInfo = doc.FirstChildElement("Network");
	if (!pBaseInfo)
	{
		log_error("没有找到节点：'Network'");
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("BigBufSize", &m_BigBufSize) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'BigBufSize'");
		return false;
	}

	if (m_BigBufSize <= 0)
	{
		log_error("BigBufSize小于等于0：%d", m_BigBufSize);
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("BigBufNum", &m_BigBufNum) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'BigBufNum'");
		return false;
	}

	if (m_BigBufNum <= 0)
	{
		log_error("BigBufSize小于等于0：%d", m_BigBufNum);
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("SmallBufSize", &m_SmallBufSize) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'SmallBufSize'");
		return false;
	}

	if (m_SmallBufSize <= 0)
	{
		log_error("SmallBufSize小于等于0：%d", m_SmallBufSize);
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("SmallBufNum", &m_SmallBufNum) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'SmallBufNum'");
		return false;
	}

	if (m_SmallBufNum <= 0)
	{
		log_error("SmallBufNum小于等于0：%d", m_SmallBufNum);
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("ListenerNum", &m_ListenerNum) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'ListenerNum'");
		return false;
	}

	if (m_ListenerNum <= 0)
	{
		log_error("ListenerNum小于等于0：%d", m_ListenerNum);
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("SocketerNum", &m_SocketerNum) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'SocketerNum'");
		return false;
	}

	if (m_SocketerNum <= 0)
	{
		log_error("SocketerNum小于等于0：%d", m_SocketerNum);
		return false;
	}

	if (pBaseInfo->QueryIntAttribute("ThreadNum", &m_ThreadNum) != XML_SUCCESS)
	{
		log_error("没有找到字段： 'ThreadNum'");
		return false;
	}

	if (m_ThreadNum < 0)
	{
		log_error("ThreadNum小于0：%d", m_ThreadNum);
		return false;
	}

	return true;
}
