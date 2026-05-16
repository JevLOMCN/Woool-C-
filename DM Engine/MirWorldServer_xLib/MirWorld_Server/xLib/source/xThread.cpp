#include "..\header\xthread.h"

xThread::xThread(void) : m_dwThreadID(0), m_hThread(INVALID_HANDLE_VALUE),
m_bTerminated(FALSE), m_bIsSuspended(FALSE), m_lpParam(nullptr)
{
}

xThread::~xThread(void)
{
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		if (m_bIsSuspended)
			Resume();
		Terminate();
		WaitFor();
	}
}

void xThread::WaitFor(void)
{
	// 处理 SendMessage 导致的死锁：在等待线程结束时泵消息
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		HANDLE handles[1] = { m_hThread };
		for (;;)
		{
			DWORD result = MsgWaitForMultipleObjects(1, handles, FALSE, INFINITE, QS_ALLINPUT);
			if (result == WAIT_OBJECT_0)
			{
				break; // 线程句柄被触发（线程已退出）
			}
			else if (result == WAIT_OBJECT_0 + 1)
			{
				MSG msg; // 有消息, 处理所有消息后继续等待
				while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						PostQuitMessage(static_cast<int>(msg.wParam)); // 将 WM_QUIT 重新投递, 保持外部应用行为不变
					}
					else
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				// 继续循环等待线程信号或更多消息
			}
			else
			{
				break; // WAIT_FAILED 或 超时/错误, 退出以避免无限循环
			}
		}
	}
}

BOOL xThread::Start(LPVOID lpParam, BOOL bSuspended)
{
	unsigned int dwThreadID;
	int nFlag = 0;
	m_lpParam = lpParam;
	if (bSuspended)
	{
		nFlag = CREATE_SUSPENDED;
		m_bIsSuspended = TRUE;
	}
	else
		m_bIsSuspended = FALSE;

	HANDLE hThread = CreateThread(nullptr,
		0,
		reinterpret_cast<LPTHREAD_START_ROUTINE>(ThreadProc),
		static_cast<LPVOID>(this),
		nFlag,
		reinterpret_cast<LPDWORD>(&dwThreadID));
	if (hThread == INVALID_HANDLE_VALUE)
		return FALSE;
	// 这样IOCP工作线程能优先处理网络IO事件
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	m_dwThreadID = dwThreadID;
	m_hThread = hThread;
	return TRUE;
}

BOOL xThread::Resume(void)
{
	if (ResumeThread(m_hThread) != static_cast<DWORD>(-1))
	{
		m_bIsSuspended = FALSE;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL xThread::Suspend(void)
{
	if (SuspendThread(m_hThread) != static_cast<DWORD>(-1))
	{
		m_bIsSuspended = TRUE;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL xThread::IsTerminated(void)
{
	return m_bTerminated;
}

BOOL xThread::IsStarted(void)
{
	return m_hThread != INVALID_HANDLE_VALUE;
}

void xThread::Terminate(void)
{
	m_bTerminated.store(TRUE);
}

unsigned WINAPI xThread::ThreadProc(LPVOID pParam)
{
	xThread* pThread = static_cast<xThread*>(pParam);
	pThread->m_bTerminated.store(FALSE);
	srand(static_cast<UINT>(reinterpret_cast<uintptr_t>(pThread->getHandle()) + timeGetTime()));
	BOOL	bException = FALSE;
	try
	{
		pThread->Execute(pThread->m_lpParam);
	}
	catch (...)
	{
		throw;
	}
	pThread->OnTerminated(bException);
	CloseHandle(pThread->m_hThread);
	pThread->m_hThread = INVALID_HANDLE_VALUE;
	pThread->m_dwThreadID = 0;
	pThread->m_bTerminated = TRUE;
	return 0;
}
