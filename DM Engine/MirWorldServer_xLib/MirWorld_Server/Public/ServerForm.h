#pragma once
#include "ioconsole.h"
#include "baseServer.h"
#include <windows.h>
#include <string>

class CServerForm : public CIOConsole
{
public:
	CServerForm(void);
	virtual ~CServerForm(void);
	BOOL Create(const char* pszTitle, const char* pszCmdLine = nullptr);
	BOOL OnClose();
	virtual BOOL StartServer();
	virtual BOOL StopServer();
	virtual VOID OnTimer();
	void OutPutStatic(DWORD dwColor, const char* pszString);
	CBaseServer* GetServer() { return m_pServer; }
	void SetServer(CBaseServer* pServer) { m_pServer = pServer; }
	// 设置是否开启更新状态
	void SetStatus(BOOL bStatus) { m_bStatus = bStatus; }
	int EnterMessageLoop();
	void OnCommand(UINT id);
	// 设置 Arena 预留内存大小（单位：KB）
	// 需要在 Create() 之后、StartServer() 之前调用
	void SetArenaReserve(size_t kbSize);
protected:
	CBaseServer* m_pServer;
	char* m_pCmdLine;
	char* m_pszServerName;
	HANDLE m_hConsole;
	WORD m_wOriginalAttributes;
	BOOL m_bRunning;
	BOOL m_bStatus;
	CServerTimer m_tmrCleanup;
	void SetConsoleColor(WORD color) const;
	WORD GetConsoleColor(DWORD dwColor);
	static BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);
	static CServerForm* s_pInstance;
};