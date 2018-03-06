#include "dotask.h"
#include "task.h"
#include "serverlog.h"
#include "GlobalDefine.h"
#include "fmt/ostream.h"
#include "Timer.h"
#include "msgbase.h"

//当处理任务时
void OnDoTask(void *tk)
{
	task *d = (task *)tk;
	Msg *pMsg = d->GetMsg();

	//切记重置
	d->Reset();
	if (!pMsg)
	{
		RunStateError("从task中获取消息失败!");
		return;
	}

	// TODO
	// 记录log
}
