#include "GlobalDefine.h"
#include "config.h"
#include "tinyxml2.h"
#include "log.h"

using namespace tinyxml2;

CConfig::CConfig()
{

}
CConfig::~CConfig()
{

}

bool CConfig::Init(const char *servername)
{
	if (!CBaseConfig::Init(servername))
		return false;

	SetServerType(ServerEnum::EST_LOG);

	const char *filename = "./config/serverconfig.xml";
	XMLDocument doc;
	if (doc.LoadFile(filename) != XML_SUCCESS)
	{
		log_error("加载 %s 失败!", filename);
		return false;
	}

	XMLElement *pinfo = doc.FirstChildElement(servername);
	if (!pinfo)
	{
		log_error("没有找到节点：'%s'", servername);
		return false;
	}

	s_DBName = pinfo->Attribute("DBName");
	if (s_DBName.empty())
	{
		log_error("没有找到字段： 'DBName'");
		return false;
	}

	s_DBUser = pinfo->Attribute("DBUser");
	if (s_DBUser.empty())
	{
		log_error("没有找到字段： 'DBUser'");
		return false;
	}

	s_DBPass = pinfo->Attribute("DBPass");
	if (s_DBPass.empty())
	{
		log_error("没有找到字段： 'DBPass'");
		return false;
	}

	s_DBIP = pinfo->Attribute("DBIP");
	if (s_DBIP.empty())
	{
		log_error("没有找到字段： 'DBIP'");
		return false;
	}

	return true;
}