#include "..\header\xiocpworkthread.h"
#include "..\header\xIocpUnit.h"

xIocpWorkThread::xIocpWorkThread(void)
{
	m_hIocp = nullptr;
}

xIocpWorkThread::xIocpWorkThread(HANDLE hIocp)
{
	m_hIocp = hIocp;
}

xIocpWorkThread::~xIocpWorkThread(void)
{
}

void xIocpWorkThread::Execute(LPVOID lpParam)
{
	DWORD	dwNumberOfBytes = 0;
	ULONG_PTR	ulCompletionKey = 0;
	IOCP_UNIT* pIocpunit;
	while (!xThread::IsTerminated())
	{
		// 使用 INFINITE 超时, 这样当没有工作时线程就会阻塞.
		// 超时为 0 会导致繁忙轮询循环和高CPU占用率.
		if (GetQueuedCompletionStatus(m_hIocp,
			&dwNumberOfBytes,
			&ulCompletionKey,
			(LPOVERLAPPED*)&pIocpunit,
			INFINITE))
		{
			if (pIocpunit == nullptr)
			{
				// 高实时性优化：移除 Sleep(1), 立即处理下一个请求
				continue;
			}
			pIocpunit->pUnit->OnComplete(dwNumberOfBytes, (DWORD)ulCompletionKey);
		}
		else
		{
			if (pIocpunit != nullptr)
			{
				pIocpunit->pUnit->OnComplete(0, 0);
			}
			// 高实时性优化：移除 Sleep(1), 立即处理下一个请求
		}
	}
}
