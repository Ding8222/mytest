#pragma once

class task;
struct Msg;

void ProcessMsg(task *tk, Msg *pMsg);

void ProcessServerMsg(task *tk, Msg *pMsg);

void ProcessLoginMsg(task *tk, Msg *pMsg);
