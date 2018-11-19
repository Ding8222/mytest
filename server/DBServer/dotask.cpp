#include <stdlib.h>
#include "serverlog.h"
#include "GlobalDefine.h"

#include "ServerType.h"
#include "LoginType.h"
#include "Login.pb.h"
#include "ServerMsg.pb.h"

#include "dotask.h"
#include "task.h"
#include "msgbase.h"
#include "ProcessMsg.h"
#include "fmt/ostream.h"
#include "Timer.h"
//当处理任务时
void OnDoTask (void *tk)
{
	task *d = (task *)tk;
	if (d->IsProcess())
	{
		Msg *pMsg = d->GetMsg();

		//切记重置
		d->Reset();
		if (!pMsg)
		{
			RunStateError("从task中获取消息失败!");
			return;
		}

		ProcessMsg(d, pMsg);
	}
}
