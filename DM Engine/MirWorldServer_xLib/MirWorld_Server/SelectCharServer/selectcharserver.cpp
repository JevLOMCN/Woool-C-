#include "stdafx.h"
#include "selectcharserver.h"
#include "server.h"

CServerForm g_Form;
CServer* g_pServer = nullptr;

// 提供额外的崩溃信息
static const char* GetSelectCharServerAdditionalInfo()
{
	static char info[512];
	sprintf_s(info, sizeof(info),
		"服务器名称: %s"
		"在线连接数: %u"
		"运行时间: %u 秒",
		SERVER_NAME,
		0, // 这里可以添加实际的连接数
		0  // 这里可以添加实际的运行时间
	);
	return info;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, ".936");
	// 初始化崩溃处理器
	CRASH_HANDLER_INIT();
	CRASH_HANDLER_SETPATH("..\\日志\\");
	CrashHandler::SetAdditionalInfoCallback(GetSelectCharServerAdditionalInfo);

	CLogFile::GetInstance()->Init("..\\日志\\角色网关");
	// 组合命令行参数
	char cmdLine[256] = { 0 };
	for (int i = 1; i < argc; i++)
	{
		if (i > 1) strcat(cmdLine, " ");
		strcat(cmdLine, argv[i]);
	}

	g_Form.Create(SERVER_NAME, cmdLine);
	g_pServer = CServer::GetInstance();
	g_pServer->SetServerName(SERVER_NAME);
	g_pServer->SetIoConsole(&g_Form);
	g_Form.SetInputListener(g_pServer);
	g_Form.SetServer(g_pServer);
	g_Form.SetStatus(FALSE);
	return g_Form.EnterMessageLoop();
}