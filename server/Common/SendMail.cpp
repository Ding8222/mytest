

/************************************************************************
* 发送邮件模块
*可以发送文本和附件（支持多个附件一起发送）
*Date:2011-12-01
************************************************************************/
#ifdef _WIN32
#include <string>
#include <tchar.h> 
#include <Ws2tcpip.h>
#include "winsock2.h"  
#include "SendMail.h"  
#pragma comment(lib,"WSOCK32")  
#pragma comment(lib, "ws2_32")

CSendMail::CSendMail(void)
{
	m_pcFileBuff = NULL;
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	memset(m_cReceiveBuff, 0, sizeof(m_cReceiveBuff));
}


CSendMail::~CSendMail(void)
{
	DeleteAllPath();
}


void CSendMail::Char2Base64(char* pBuff64, char* pSrcBuff, int iLen)
{
	//1   1   1   1   1   1   1   1  
	// 分配给pBuff64  ↑ 分配给pBuff64+1  
	//         point所在的位置  
	static char Base64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//base64所映射的字符表  
	int point;//每一个源字符拆分的位置，可取2,4,6；初始为2  
	point = 2;
	int i;
	int iIndex;//base64字符的索引  
	char n = 0;//上一个源字符的残留值  
	for (i = 0; i < iLen; i++)
	{
		if (point == 2)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3f;//取得pSrcBuff的高point位  
		}
		else if (point == 4)
		{
			iIndex = ((*pSrcBuff) >> point) & 0xf;//取得pSrcBuff的高point位  
		}
		else if (point == 6)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3;//取得pSrcBuff的高point位  
		}
		iIndex += n;//与pSrcBuff-1的低point结合组成Base64的索引  
		*pBuff64++ = Base64Encode[iIndex];//由索引表得到pBuff64  
		n = ((*pSrcBuff) << (6 - point));//计算源字符中的残留值  
		n = n & 0x3f;//确保n的最高两位为0  
		point += 2;//源字符的拆分位置上升2  
		if (point == 8)//如果拆分位置为8说明pSrcBuff有6位残留，可以组成一个完整的Base64字符，所以直接再组合一次  
		{
			iIndex = (*pSrcBuff) & 0x3f;//提取低6位，这就是索引了  
			*pBuff64++ = Base64Encode[iIndex];//  
			n = 0;//残留值为0  
			point = 2;//拆分位置设为2  
		}
		pSrcBuff++;

	}
	if (n != 0)
	{
		*pBuff64++ = Base64Encode[n];
	}
	if (iLen % 3 == 2)//如果源字符串长度不是3的倍数要用'='补全  
	{
		*pBuff64 = '=';
	}
	else if (iLen % 3 == 1)
	{
		*pBuff64++ = '=';
		*pBuff64 = '=';
	}
}

void CSendMail::AddFilePath(std::string &pcFilePath)//添加附件路径  
{
	if (pcFilePath.empty())
	{
		return;
	}
	auto iter = find(m_pcFilePathList.begin(), m_pcFilePathList.end(), pcFilePath);
	if(iter== m_pcFilePathList.end())
		m_pcFilePathList.push_back(pcFilePath);
}

void CSendMail::DeleteFilePath(std::string &pcFilePath)//删除附件路径  
{
	auto iter = find(m_pcFilePathList.begin(), m_pcFilePathList.end(), pcFilePath);
	if (iter != m_pcFilePathList.end())
		m_pcFilePathList.erase(iter);
}


void CSendMail::DeleteAllPath(void)
{
	m_pcFilePathList.clear();
}

int CSendMail::GetFileData(char* FilePath)
{
	m_pcFileBuff = NULL;
	if (FilePath == NULL)
	{
		return 0;
	}
	FILE* f;
	int len;

	TCHAR temp[256] = { 0 };

	MultiByteToWideChar(CP_UTF8, 0, FilePath, sizeof(FilePath)-1, temp, 256);
	
	errno_t err = _tfopen_s(&f, temp, _T("rb"));//打开Log文件
	if (err)
		return FALSE;

	fseek(f, 0, SEEK_END);
	len = ftell(f);
	m_pcFileBuff = new char[len + 1];
	memset(m_pcFileBuff, 0, len + 1);
	fread(m_pcFileBuff, len, 1, f);

	fclose(f);
	return len;
}

void CSendMail::GetFileName(char* fileName, char* filePath)
{
	if (filePath == NULL || fileName == NULL)
	{
		return;
	}
	int i;
	for (i = 0; i < (int)strlen(filePath); i++)
	{
		if (filePath[strlen(filePath) - 1 - i] == '\\')
		{
			memcpy(fileName, &filePath[strlen(filePath) - i], i);
			return;
		}
	}
}

bool CSendMail::CReateSocket(SOCKET &sock)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return false;
	}
	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		return false;
	}
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sock == INVALID_SOCKET)
	{
		return false;
	}

	sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(25);//发邮件一般都是25端口  
	if (m_sMailInfo.m_pcIPName == NULL)
	{
		//直接使用IP地址 
		inet_pton(AF_INET, m_sMailInfo.m_pcIPAddr, reinterpret_cast<LPVOID>(servaddr.sin_addr.S_un.S_addr));
	}
	else
	{
		struct addrinfo *answer, hint, *curr;
		char ipstr[16];
		ZeroMemory(&hint, sizeof(hint));
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;

		int ret = getaddrinfo(m_sMailInfo.m_pcIPName, NULL, &hint, &answer);
		if (ret != 0)
		{
			DWORD dwErrCode = GetLastError();
			return false;
		}

		for (curr = answer; curr != NULL; curr = curr->ai_next) {
			inet_ntop(AF_INET,
				&(((struct sockaddr_in *)(curr->ai_addr))->sin_addr),
				ipstr, 16);
		}
		struct in_addr s;

		inet_pton(AF_INET, ipstr, (void*) &(s));
		servaddr.sin_addr.s_addr = s.s_addr;
		//servaddr.sin_addr.s_addr = ((struct sockaddr_in *)(curr->ai_addr))->sin_addr.s_addr;
	}


	int ret = connect(sock, (sockaddr*)&servaddr, sizeof(servaddr));//建立连接  
	if (ret == SOCKET_ERROR)
	{
		DWORD dwErr = GetLastError();
		return false;
	}

	return true;
}


bool CSendMail::Logon(SOCKET &sock)
{
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "HELO []\r\n");
	send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);//开始会话  
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '2' || m_cReceiveBuff[1] != '5' || m_cReceiveBuff[2] != '0')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "AUTH LOGIN\r\n");
	send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);//请求登录  
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '3' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '4')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	Char2Base64(m_cSendBuff, m_sMailInfo.m_pcUserName, static_cast<int>(strlen(m_sMailInfo.m_pcUserName)));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);//发送用户名  
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '3' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '4')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	Char2Base64(m_cSendBuff, m_sMailInfo.m_pcUserPassWord, static_cast<int>(strlen(m_sMailInfo.m_pcUserPassWord)));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);//发送用户密码  
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '2' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '5')
	{
		return false;
	}
	return true;//登录成功  
}


bool CSendMail::SendHead(SOCKET &sock)
{
	int rt;
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "MAIL FROM:<%s>\r\n", m_sMailInfo.m_pcSender);
	rt = send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);

	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "RCPT TO:<%s>\r\n", m_sMailInfo.m_pcReceiver);
	rt = send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	memcpy(m_cSendBuff, "DATA\r\n", strlen("DATA\r\n"));
	rt = send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "From:\"%s\"<%s>\r\n", m_sMailInfo.m_pcSenderName, m_sMailInfo.m_pcSender);
	sprintf_s(&m_cSendBuff[strlen(m_cSendBuff)], 150, "To:\"INVT.COM.CN\"<%s>\r\n", m_sMailInfo.m_pcReceiver);
	sprintf_s(&m_cSendBuff[strlen(m_cSendBuff)], 150, "Subject:%s\r\nMime-Version: 1.0\r\nContent-Type: multipart/mixed;   boundary=\"INVT\"\r\n\r\n", m_sMailInfo.m_pcTitle);
	rt = send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}

	return true;
}

bool CSendMail::SendTextBody(SOCKET &sock)
{
	int rt;
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "--INVT\r\nContent-Type: text/plain;\r\n  charset=\"gb2312\"\r\n\r\n%s\r\n\r\n", m_sMailInfo.m_pcBody);
	rt = send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CSendMail::SendFileBody(SOCKET &sock)
{
	char filePath[128];
	std::string strPath;
	int rt;
	int len;
	int pt = 0;
	char fileName[128];
	auto iterBeg = m_pcFilePathList.begin();
	auto iterEnd = m_pcFilePathList.end();
	for (; iterBeg != iterEnd; ++iterBeg)
	{
		pt = 0;
		memset(fileName, 0, 128);
		strPath = *iterBeg;
		strncpy_s(filePath, 128, iterBeg->c_str(), 127);
		len = GetFileData(filePath);
		GetFileName(fileName, filePath);

		sprintf_s(m_cSendBuff, "--INVT\r\nContent-Type: application/octet-stream;\r\n  name=\"%s\"\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment;\r\n  filename=\"%s\"\r\n\r\n", fileName, fileName);
		send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);
		while (pt < len)
		{
			memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
			Char2Base64(m_cSendBuff, &m_pcFileBuff[pt], min(len - pt, 3000));
			m_cSendBuff[strlen(m_cSendBuff)] = '\r';
			m_cSendBuff[strlen(m_cSendBuff)] = '\n';
			rt = send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);
			pt += min(len - pt, 3000);
			if (rt != strlen(m_cSendBuff))
			{
				return false;
			}
		}
		if (len != 0)
		{
			delete[] m_pcFileBuff;
		}
	}

	return true;
}

bool CSendMail::SendEnd(SOCKET &sock)
{
	sprintf_s(m_cSendBuff, "--INVT--\r\n.\r\n");
	send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);

	sprintf_s(m_cSendBuff, "QUIT\r\n");
	send(sock, m_cSendBuff, static_cast<int>(strlen(m_cSendBuff)), 0);
	closesocket(sock);
	WSACleanup();
	return true;
}


bool CSendMail::SendMail(sMailInfo &smailInfo)
{
	memcpy(&m_sMailInfo, &smailInfo, sizeof(smailInfo));
	if (m_sMailInfo.m_pcBody == NULL
		|| m_sMailInfo.m_pcIPAddr == NULL
		|| m_sMailInfo.m_pcIPName == NULL
		|| m_sMailInfo.m_pcReceiver == NULL
		|| m_sMailInfo.m_pcSender == NULL
		|| m_sMailInfo.m_pcSenderName == NULL
		|| m_sMailInfo.m_pcTitle == NULL
		|| m_sMailInfo.m_pcUserName == NULL
		|| m_sMailInfo.m_pcUserPassWord == NULL)
	{
		return false;
	}
	SOCKET sock;
	if (!CReateSocket(sock))//建立连接  
	{
		return false;
	}

	if (!Logon(sock))//登录邮箱  
	{
		return false;
	}

	if (!SendHead(sock))//发送邮件头  
	{
		return false;
	}

	if (!SendTextBody(sock))//发送邮件文本部分  
	{
		return false;
	}

	if (!SendFileBody(sock))//发送附件  
	{
		return false;
	}

	if (!SendEnd(sock))//结束邮件，并关闭sock  
	{
		return false;
	}

	return true;
}

#define BODY_MAX_LEN 4096

void SendEMailToInternet(const char *str, char *addr)
{
	char body[BODY_MAX_LEN] = { 0 };

	strncpy_s(body, BODY_MAX_LEN, str, BODY_MAX_LEN - 1);

	strncat_s(body, BODY_MAX_LEN, "宕机啦", BODY_MAX_LEN - 1);

	sMailInfo sm;
	sm.m_pcUserName = "minidump@tbiglong.com";//"";
	sm.m_pcUserPassWord = "Dump123456";// "";
	sm.m_pcSenderName = "宕机提示";
	sm.m_pcSender = "minidump@tbiglong.com";//发送者的邮箱地址  
	sm.m_pcReceiver = addr;//接收者的邮箱地址  
	sm.m_pcTitle = "宕机提示";//邮箱标题  
	sm.m_pcBody = body;//邮件文本正文  
	sm.m_pcIPAddr = "";//服务器的IP可以留空 
	sm.m_pcIPName = "smtp.mxhichina.com";//服务器的名称（IP与名称二选一，优先取名称）  

	CSendMail csm;
	csm.SendMail(sm);
}
#endif