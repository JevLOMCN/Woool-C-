#pragma once
#include "xinc.h"
#include <stdexcept>
#include <atomic>

using namespace std;
class xThread
{
public:
	xThread(void);
	virtual ~xThread(void);
	///启动线程
	BOOL Start(LPVOID lpParam = nullptr, BOOL bSuspended = FALSE);
	///继续线程
	BOOL Resume(void);
	/// 挂起线程
	BOOL Suspend(void);
	///终止线程
	void Terminate(void);
	///判断线程是否已终止
	BOOL IsTerminated(void);
	///线程是否已启动
	BOOL IsStarted(void);
	///等待线程退出
	void WaitFor(void);
	HANDLE getHandle()const { return m_hThread; }
	DWORD getThreadID()const { return m_dwThreadID; }
protected:
	///线程的主运行体
	virtual void Execute(LPVOID lpParam) = 0;
	///Terminate事件
	virtual void OnTerminated(BOOL bExcepted = FALSE) {};
	virtual void OnExecuteException(const exception& e) {}
	///标志是否已终止
	std::atomic<BOOL> m_bTerminated;
private:
	LPVOID m_lpParam;
	///线程号
	DWORD m_dwThreadID;
	///线程句柄
	HANDLE m_hThread;
	///标志是否已挂起
	BOOL m_bIsSuspended;
	///线程例程
	static unsigned WINAPI ThreadProc(LPVOID pParam);
};
