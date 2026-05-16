#include "..\header\xinc.h"
#include "..\header\xiocpmanager.h"
#include "..\header\xiocpworkthread.h"

xIocpManager::xIocpManager(void)
{
	m_hIocp = nullptr;
	m_iIocpWorkThreadCount = 0;
	m_pIocpWorkThread = nullptr;
}

xIocpManager::~xIocpManager(void)
{
	if (m_hIocp != nullptr)
		CloseHandle(m_hIocp);
}

BOOL xIocpManager::Start()
{
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (m_hIocp == nullptr)
	{
		setError(-1, "创建完成端口失败, Errorcode = %d\n", GetLastError());
		return FALSE;
	}

	SYSTEM_INFO	si;
	GetSystemInfo(&si);
	m_iIocpWorkThreadCount = static_cast<int>(si.dwNumberOfProcessors * 4);

	m_pIocpWorkThread = new xIocpWorkThread[m_iIocpWorkThreadCount];

	for (int i = 0; i < m_iIocpWorkThreadCount; i++)
	{
		m_pIocpWorkThread[i].setIocpHandle(m_hIocp);
		m_pIocpWorkThread[i].Start();
	}
	return TRUE;
}

VOID xIocpManager::Stop()
{
	if (m_hIocp == nullptr)
		return;

	if (m_pIocpWorkThread == nullptr)
	{
		CloseHandle(m_hIocp);
		m_hIocp = nullptr;
		m_iIocpWorkThreadCount = 0;
		return;
	}

	for (int i = 0; i < m_iIocpWorkThreadCount; ++i)
	{
		m_pIocpWorkThread[i].Terminate();
	}

	for (int i = 0; i < m_iIocpWorkThreadCount; ++i)
	{
		PostQueuedCompletionStatus(m_hIocp, 0, 0, nullptr);
	}

	for (int i = 0; i < m_iIocpWorkThreadCount; ++i)
	{
		m_pIocpWorkThread[i].WaitFor();
	}

	delete[] m_pIocpWorkThread;
	m_pIocpWorkThread = nullptr;
	m_iIocpWorkThreadCount = 0;

	CloseHandle(m_hIocp);
	m_hIocp = nullptr;
}

BOOL xIocpManager::Bind(SOCKET socket, DWORD dwBindId)
{
	if (m_hIocp == nullptr)
	{
		setError(-2, "绑定socket失败, 完成端口没有初始化!");
		return FALSE;
	}
	HANDLE hPort = CreateIoCompletionPort((HANDLE)socket, m_hIocp, dwBindId, 0);
	return TRUE;
}
