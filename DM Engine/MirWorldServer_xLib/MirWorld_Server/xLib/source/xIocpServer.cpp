#include "..\header\xiocpserver.h"
#include "..\header\xiocpunit.h"
#include "..\header\xpacket.h"

xIocpServer::xIocpServer(void) : m_xAcceptConnectionQueue(4096), m_xDisconnectQueue(4096), m_bWSAInitialized(FALSE)
{
	m_dwSendKBytes = 0;
	m_dwSendBytes = 0;
	m_dwRecvKBytes = 0;
	m_dwRecvBytes = 0;
	
	// 初始化WSA
	WSADATA	wsad;
	memset(&wsad, 0, sizeof(wsad));
	int wsast = WSAStartup(0x202, &wsad);
	if (wsast != 0)
	{
		switch (wsast)
		{
		case WSASYSNOTREADY:
			setError(wsast, "WSAStartup失败: 网络子系统不可用");
			break;
		case WSAVERNOTSUPPORTED:
			setError(wsast, "WSAStartup失败: 不支持的Winsock版本 %d.%d", LOBYTE(0x202), HIBYTE(0x202));
			break;
		case WSAEINPROGRESS:
			setError(wsast, "WSAStartup失败: Winsock初始化正在进行中");
			break;
		case WSAEPROCLIM:
			setError(wsast, "WSAStartup失败: 达到任务限制");
			break;
		case WSAEFAULT:
			setError(wsast, "WSAStartup失败: 无效的lpWSAData指针");
			break;
		default:
			setError(wsast, "WSAStartup失败: 未知错误代码 %d", wsast);
			break;
		}
		m_bWSAInitialized = FALSE;
		return;
	}
	
	// 验证Winsock版本
	if (wsad.wVersion != 0x202)
	{
		setError(-1, "Winsock版本不匹配: 期望 %d.%d, 实际 %d.%d", 
			LOBYTE(0x202), HIBYTE(0x202), LOBYTE(wsad.wVersion), HIBYTE(wsad.wVersion));
		WSACleanup(); // 清理已初始化的资源
		m_bWSAInitialized = FALSE;
		return;
	}
	
	// WSA初始化成功
	m_bWSAInitialized = TRUE;
}

xIocpServer::~xIocpServer(void)
{
	// 安全停止服务器
	stop();
	
	// 只有当WSA初始化成功时才调用WSACleanup
	if (m_bWSAInitialized)
	{
		WSACleanup();
		m_bWSAInitialized = FALSE;
	}
}

BOOL xIocpServer::postListen(const char* cp, UINT nPort, int nPostAccept, UINT id)
{
	// 检查WSA初始化状态
	if (!m_bWSAInitialized)
	{
		setError(-1, "无法监听端口 %d: WSA未正确初始化", nPort);
		return FALSE;
	}
	
	//	1- 分配listen对象
	xListenObject* pListenObject = m_xListenObjectPool.newObject();
	if (pListenObject == nullptr)
	{
		setError(-4, "无法分配新的监听对象!");
		return FALSE;
	}
	pListenObject->setServer(this);
	pListenObject->setId(id);
	//	2-	开始监听
	if (!pListenObject->listen( /*cp, */nPort))
	{
		setError(*pListenObject);
		return FALSE;
	}
	//	3-	绑定到完成端口
	if (!m_xIocpManager.Bind(pListenObject->getSocketFd(), 0xffffffff))
	{
		setError(m_xIocpManager);
		return FALSE;
	}
	//	4-	发送accept请求, 增加并发数提升性能
	if (nPostAccept < 128) {
		pListenObject->postAccept(128);  // 最少128个并发Accept
	} else {
		pListenObject->postAccept(nPostAccept);
	}
	return TRUE;
}

BOOL xIocpServer::start()
{
	// 检查WSA是否已正确初始化
	if (!m_bWSAInitialized)
	{
		setError(-1, "无法启动服务器: WSA未正确初始化, 请检查网络子系统");
		return FALSE;
	}
	
	//	iocp 管理器开始
	if (!m_xIocpManager.Start())
	{
		setError(m_xIocpManager);
		return FALSE;
	}
	return TRUE;
}

BOOL xIocpServer::stop()
{
	m_xIocpManager.Stop();
	return TRUE;
}

VOID xIocpServer::update()
{
	// 批量处理连接
	for (int i = 0; i < 16; i++)  // 一次处理16个连接
	{
		xTempClient* pTempSocket = m_xAcceptConnectionQueue.pop();
		if (!pTempSocket) break;
		
		if (!pTempSocket->isPreDeleted())
		{
			sendEvent(ISE_ONCONNECTION, pTempSocket->getId(), (LPVOID)pTempSocket);
			pTempSocket->preDelete(1000);  // 缩短延迟
			m_xAcceptConnectionQueue.push(pTempSocket);
		}
		else
		{
			if (!pTempSocket->deleteTimeOut())
			{
				m_xAcceptConnectionQueue.push(pTempSocket);
			}
			else
			{
				pTempSocket->close();
				m_xTempClientPool.deleteObject(pTempSocket);
			}
		}
	}
	
	// 批量处理断开连接
	for (int i = 0; i < 8; i++)  // 一次处理8个断开
	{
		xSocket* pSocket = m_xDisconnectQueue.pop();
		if (!pSocket) break;
		sendEvent(ISE_ONDISCONNECTION, 0, pSocket);
	}
}

xPacket* xIocpServer::newPacket()
{
	xPacket* pPacket = m_xPacketPool.newObject();
	if (pPacket->notcreated())
		pPacket->create(DEF_PACKET_SIZE); // 从4KB增加到8KB
	return pPacket;
}

void xIocpServer::releasePacket(xPacket* pPacket)
{
	pPacket->clear();
	m_xPacketPool.deleteObject(pPacket);
}

xIocpUnit* xIocpServer::newIocpUnit()
{
	return m_xIocpUnitPool.newObject();
}

void xIocpServer::releaseIocpUnit(xIocpUnit* pIocpUnit)
{
	pIocpUnit->setType(IO_NOTSET);
	pIocpUnit->setData(nullptr);
	pIocpUnit->setEventListener(nullptr);
	m_xIocpUnitPool.deleteObject(pIocpUnit);
}

void xIocpServer::onConnection(xSocket* pSocket, UINT id)
{
	xTempClient* pTempSocket = m_xTempClientPool.newObject();
	pTempSocket->Clean();
	pTempSocket->steelSocket(*pSocket);
	m_xIocpManager.Bind(pTempSocket->getSocketFd(), 0);
	pTempSocket->setId(id);
	
	// 性能优化设置
	pTempSocket->setReuseAddr(TRUE);
	pTempSocket->setTcpNoDelay(TRUE);
	pTempSocket->setKeepAlive(TRUE, 30, 5);
	pTempSocket->setSendBuffer(65536);    // 64KB发送缓冲区
	pTempSocket->setRecvBuffer(131072);   // 128KB接收缓冲区
	
	//pTempSocket->postRecv();
	pTempSocket->setServer(this);
	m_xAcceptConnectionQueue.push(pTempSocket);
}

void xIocpServer::onDisconnect(xSocket* pSocket)
{
	m_xDisconnectQueue.push(pSocket);
}

BOOL xIocpServer::postConnection(const char* cp, UINT nPort, xSocket& socket)
{
	if (socket.connect(cp, nPort))
	{
		if (m_xIocpManager.Bind(socket.getSocketFd(), 0))
			return TRUE;
		socket.close();
	}
	return FALSE;
}
