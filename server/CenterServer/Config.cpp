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

	SetServerType(ServerEnum::EST_CENTER);

	return true;
}