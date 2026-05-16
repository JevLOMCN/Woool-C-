#include "stdafx.h"
#include "GameServer.h"
#include "server.h"
#include "fmttextfile.h"
#include "monsterex.h"
#include "scriptobject.h"
#include "VMProtectHelper.h"
#include "HumanPlayerMgr.h"

CServerForm g_Form;
CServer* g_pServer = nullptr;

// 提供额外的崩溃信息
static const char* GetServerAdditionalInfo()
{
	static char info[512];
	sprintf_s(info, sizeof(info),
		"服务器名称: %s"
		"在线玩家数: %u"
		"运行时间: %u 秒"
		"当前地图数: %u",
		SERVER_NAME,
		0, // 这里可以添加实际的玩家数量
		0, // 这里可以添加实际的运行时间
		0  // 这里可以添加实际的地图数量
	);
	return info;
}

static int CheckLicense()
{
#ifndef USE_VMPROTECT
	return 1;
#endif
	if (!VMP_IS_PROTECTED()) return -1;
	if (VMP_IS_DEBUGGER_PRESENT(TRUE)) return -2;
	if (!VMP_IS_VALID_IMAGECRC()) return -2;
	FILE* fp = nullptr;
	if (fopen_s(&fp, "license.key", "r") == 0 && fp)
	{
		CHAR szSerial[1024] = { 0 };
		fgets(szSerial, sizeof(szSerial), fp);
		fclose(fp);
		size_t len = strlen(szSerial); // 去掉换行符
		while (len > 0 && (szSerial[len - 1] == '\n' || szSerial[len - 1] == '\r'))
			szSerial[--len] = 0;
		if (VMP_SET_SERIAL_NUMBER(szSerial) != 0) return -1; //序列号无效
	}
	else
		return 1; //未找到序列号文件
	return 0;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, ".936");
	// 初始化崩溃处理器
	CRASH_HANDLER_INIT();
	CRASH_HANDLER_SETPATH("..\\日志\\");
	CrashHandler::SetAdditionalInfoCallback(GetServerAdditionalInfo);

	TRY_INIT
	CLogFile::GetInstance()->Init("..\\日志\\龙路引擎");
	// 组合命令行参数
	char cmdLine[256] = { 0 };
	for (int i = 1; i < argc; i++)
	{
		if (i > 1) strcat(cmdLine, " ");
		strcat(cmdLine, argv[i]);
	}
	g_Form.Create(SERVER_NAME, cmdLine);

	VMP_PROTECT_BEGIN("许可证");
	CHAR szHWSerial[256] = { 0 };
	CHAR szMsg[512] = { 0 };
	const char* pszDecryptString = nullptr;
	DWORD dwColor = 0xFF;
	VMP_GET_CURRENT_HWID(szHWSerial, 256);
	int nResult = CheckLicense();
	switch (nResult)
	{
	case 1:
	{
		pszDecryptString = VMP_DECRYPT_STRINGA("测试版引擎！\n用户名：测试\n人数：5人\n硬件ID：%s\n");
		snprintf(szMsg, sizeof(szMsg), pszDecryptString, szHWSerial);
		dwColor = 0xFF00;
		CHumanPlayerMgr::GetInstance()->SetTestMode();
	}
	break;
	case 0:
	{
		VMProtectSerialNumberData sd = { 0 };
		VMP_GET_SERIAL_NUMBER_DATA(&sd, sizeof(sd));
		SYSTEMTIME st; // 过期时间校验
		GetLocalTime(&st);
		int nExpire = sd.dtExpire.wYear * 10000 + sd.dtExpire.bMonth * 100 + sd.dtExpire.bDay;
		int nNow = st.wYear * 10000 + st.wMonth * 100 + st.wDay;
		if (nExpire < nNow)
		{
			pszDecryptString = VMP_DECRYPT_STRINGA("错误：许可证已过期！\n硬件ID：%s\n");
			snprintf(szMsg, sizeof(szMsg), pszDecryptString, szHWSerial);
			nResult = -1;
		}
		else
		{
			pszDecryptString = VMP_DECRYPT_STRINGA("引擎已注册！\n用户名：%ls\n人数：无限制\n有效期：%u年-%u月-%u日\n硬件ID：%s\n");
			snprintf(szMsg, sizeof(szMsg), pszDecryptString, sd.wUserName, sd.dtExpire.wYear, sd.dtExpire.bMonth, sd.dtExpire.bDay, szHWSerial);
			dwColor = 0xFF00;
		}
	}
	break;
	case -1:
	{
		pszDecryptString = VMP_DECRYPT_STRINGA("错误：许可证验证失败！\n硬件ID：%s\n");
		snprintf(szMsg, sizeof(szMsg), pszDecryptString, szHWSerial);
	}
	break;
	case -2:
	{
		pszDecryptString = VMP_DECRYPT_STRINGA("错误：程序被恶意修改！\n硬件ID：%s\n");
		snprintf(szMsg, sizeof(szMsg), pszDecryptString, szHWSerial);
	}
	break;
	}
	g_Form.OutPutStatic(dwColor, szMsg);
	VMP_FREE_STRING(pszDecryptString);
	if (nResult < 0) return -1;
	VMP_PROTECT_END();

	g_Form.SetArenaReserve(256 * 1024);
	g_pServer = CServer::GetInstance();
	g_pServer->SetServerName(SERVER_NAME);
	g_pServer->SetIoConsole(&g_Form);
	g_Form.SetInputListener(g_pServer);
	g_Form.SetServer(g_pServer);
	g_Form.SetStatus(TRUE);
	return g_Form.EnterMessageLoop();
}