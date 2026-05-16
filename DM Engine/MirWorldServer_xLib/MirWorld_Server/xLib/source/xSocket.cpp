#include "..\header\xinc.h"
#include "..\header\xsocket.h"

xSocket::xSocket() : m_state(SS_UNINITED), m_Socket(INVALID_SOCKET), m_nPort(0)
{
	m_szAddress[0] = 0;
	memset(&m_stats, 0, sizeof(m_stats));
	m_stats.connection_time = getCurrentTime();
}

xSocket::xSocket(SOCKET Socket) : m_state(SS_UNINITED), m_Socket(INVALID_SOCKET), m_nPort(0)
{
	m_szAddress[0] = 0;
	setSocket(Socket);
	memset(&m_stats, 0, sizeof(m_stats));
	m_stats.connection_time = getCurrentTime();
}

xSocket::~xSocket(void)
{
	close();
}

BOOL xSocket::makeSocket(int af, int type, int protocol)
{
	m_Socket = socket(af, type, protocol);
	if (m_Socket == INVALID_SOCKET)
	{
		setError(WSAGetLastError(), "socket( %d, %d, %d ) 调用失败!", af, type, protocol);
		setState(SS_UNINITED);
		return FALSE;
	}
	setState(SS_UNUSED);
	
	// 自动应用性能优化设置
	setReuseAddr(TRUE);
	setTcpNoDelay(TRUE);
	setKeepAlive(TRUE, 30, 5);
	setSendBuffer(8192);     // 8KB发送缓冲区（低延迟优先）
	setRecvBuffer(65536);    // 64KB接收缓冲区
	
	return TRUE;
}

BOOL xSocket::setSocket(SOCKET Socket)
{
	m_Socket = Socket;
	if (Socket != INVALID_SOCKET)
		setState(SS_UNUSED);
	else
	{
		m_szAddress[0] = 0;
		m_nPort = 0;
		setState(SS_UNINITED);
	}
	return TRUE;
}

VOID xSocket::close()
{
	//m_szAddress[0] = 0;
	m_nPort = 0;
	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;
	setState(SS_UNINITED);
}

BOOL xSocket::sendEx(LPVOID lpData, int nDatasize, DWORD& dwBytesSent, DWORD dwFlag, LPOVERLAPPED lpOverlapped)
{
	WSABUF wsabuf{};
	wsabuf.buf = static_cast<char*>(lpData);
	wsabuf.len = nDatasize;

	dwBytesSent = 0;
	
	if (WSASend(m_Socket, &wsabuf, 1, &dwBytesSent, dwFlag, lpOverlapped, nullptr) == SOCKET_ERROR)
	{
		int code = WSAGetLastError();
		if (code != WSA_IO_PENDING)
		{
			setError(code, "WSASend() 失败!");
			return FALSE;
		}
		// 异步操作, 稍后会通过完成端口通知
	} else {
		// 同步完成
		m_stats.bytes_sent += dwBytesSent;
		m_stats.packets_sent++;
		m_stats.last_activity = getCurrentTime();
	}
	return TRUE;
}

BOOL xSocket::recvEx(LPVOID lpDataBuf, int nBufsize, DWORD& dwBytesReceived, DWORD& dwFlag, LPOVERLAPPED lpOverlapped)
{
	WSABUF	wsabuf;
	wsabuf.buf = static_cast<char*>(lpDataBuf);
	wsabuf.len = nBufsize;

	if (WSARecv(m_Socket, &wsabuf, 1, &dwBytesReceived, &dwFlag, lpOverlapped, nullptr) == SOCKET_ERROR)
	{
		int code = WSAGetLastError();
		if (code != WSA_IO_PENDING)
		{
			setError(code, "WSARecv() 失败!");
			return FALSE;
		}
	} else {
		// 同步完成, 更新统计
		m_stats.bytes_received += dwBytesReceived;
		m_stats.packets_received++;
		m_stats.last_activity = getCurrentTime();
	}
	return TRUE;
}

BOOL xSocket::acceptEx(xSocket& sAccept, LPVOID lpDataBuf, DWORD dwRecvBufferLength, DWORD& dwBytesReceived, LPOVERLAPPED lpOverlapped)
{
	if (sAccept.getState() != SS_UNUSED)
	{
		sAccept.close();
		sAccept.makeSocket();
	}

	if (AcceptEx(m_Socket, sAccept.getSocketFd(), lpDataBuf, dwRecvBufferLength, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytesReceived, lpOverlapped))
	{
		sAccept.setState(SS_ACCEPTED);
		return TRUE;
	}

	if (WSAGetLastError() == ERROR_IO_PENDING)
		return TRUE;

	setError(WSAGetLastError(), "AcceptEx() 失败!");

	return FALSE;
}

BOOL xSocket::connect(const char* cp, UINT nPort)
{
	struct	sockaddr_in ServAddr;
	LPHOSTENT m_pHost = gethostbyname(cp);
	if (m_pHost == nullptr)
	{
		setError(WSAGetLastError(), "connect() 中 gethostbyname( %s ) 失败!", cp);
		return FALSE;
	}

	if (getState() != SS_UNUSED)
	{
		close();
		makeSocket();
	}

	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = *reinterpret_cast<ULONG*>(m_pHost->h_addr_list[0]);
	ServAddr.sin_port = htons(nPort);

	int erri = ::connect(m_Socket, (struct sockaddr*)&ServAddr, sizeof(ServAddr));
	if (erri == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "connect() 中 connect() 失败!");
		return FALSE;
	}
	setEndPoint(cp, nPort);
	setState(SS_CONNECTED);
	return	TRUE;
}

BOOL xSocket::listen(UINT nPort)
{
	struct	sockaddr_in	serveraddr;
	if (getState() != SS_UNUSED)
	{
		close();
		makeSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	memset(static_cast<void*>(&serveraddr), 0, sizeof(struct sockaddr_in));

	serveraddr.sin_family = PF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(nPort);

	if (::bind(m_Socket, reinterpret_cast<struct sockaddr*>(&serveraddr), sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "listen() 中 bind() 失败!");
		return FALSE;
	}
	if (::listen(m_Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "listen() 中 listen() 失败!");
		return FALSE;
	}
	setEndPoint("0.0.0.0", nPort);
	setState(SS_LISTENING);
	return TRUE;
}

BOOL xSocket::listen(const char* cp, UINT nPort)
{
	struct	sockaddr_in	serveraddr;
	LPHOSTENT m_pHost = gethostbyname(cp);
	if (m_pHost == nullptr)
	{
		setError(WSAGetLastError(), "listen( %s, %u ) 中 gethostbyname( %s ) 失败!", cp, nPort, cp);
		return FALSE;
	}

	if (getState() != SS_UNUSED)
	{
		close();
		makeSocket();
	}

	memset(static_cast<void*>(&serveraddr), 0, sizeof(struct sockaddr_in));

	serveraddr.sin_family = PF_INET;
	serveraddr.sin_addr.s_addr = *reinterpret_cast<ULONG*>(m_pHost->h_addr_list[0]);
	serveraddr.sin_port = htons(nPort);

	if (::bind(m_Socket, reinterpret_cast<struct sockaddr*>(&serveraddr), sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "listen() 中 bind() 失败!");
		return FALSE;
	}
	if (::listen(m_Socket, SOMAXCONN) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "listen() 中 listen() 失败!");
		return FALSE;
	}
	setEndPoint(cp, nPort);
	setState(SS_LISTENING);
	return TRUE;
}

BOOL xSocket::setNoBlocking()
{
	unsigned long mode = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "setNoBlocking() 中 ioctlsocket() 失败!");
		return FALSE;
	}
	return TRUE;
}

BOOL xSocket::setBlocking()
{
	unsigned long mode = 0;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) == SOCKET_ERROR)
	{
		setError(WSAGetLastError(), "setBlocking() 中 ioctlsocket() 失败!");
		return FALSE;
	}
	return TRUE;
}

BOOL xSocket::setReuseAddr(BOOL reuse)
{
	BOOL opt = reuse;
	return setSocketOption(SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

BOOL xSocket::setKeepAlive(BOOL keepalive, UINT keepalive_time, UINT keepalive_interval)
{
	if (!setSocketOption(SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive))) {
		return FALSE;
	}
	
	if (keepalive) {
		// Windows 特定的 keepalive 设置
		tcp_keepalive keepalive_vals;
		keepalive_vals.onoff = 1;
		keepalive_vals.keepalivetime = keepalive_time * 1000;  // 转换为毫秒
		keepalive_vals.keepaliveinterval = keepalive_interval * 1000;  // 转换为毫秒
		
		DWORD bytes_returned;
		if (WSAIoctl(m_Socket, SIO_KEEPALIVE_VALS, &keepalive_vals, sizeof(keepalive_vals),
					 nullptr, 0, &bytes_returned, nullptr, nullptr) == SOCKET_ERROR) {
			setError(WSAGetLastError(), "setKeepAlive() 中 WSAIoctl() 失败!");
			return FALSE;
		}
	}
	
	return TRUE;
}

BOOL xSocket::setTcpNoDelay(BOOL nodelay)
{
	BOOL opt = nodelay;
	return setSocketOption(IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

BOOL xSocket::setSendBuffer(UINT size)
{
	return setSocketOption(SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

BOOL xSocket::setRecvBuffer(UINT size)
{
	return setSocketOption(SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

BOOL xSocket::setSocketOption(int level, int optname, const void* optval, int optlen)
{
	if (setsockopt(m_Socket, level, optname, static_cast<const char*>(optval), optlen) == SOCKET_ERROR) {
		setError(WSAGetLastError(), "setSocketOption() 失败!level=%d, optname=%d", level, optname);
		return FALSE;
	}
	return TRUE;
}

DWORD xSocket::getCurrentTime() const
{
	return timeGetTime();
}

DWORD xSocket::getConnectionAge() const
{
	DWORD current_time = getCurrentTime();
	return current_time - m_stats.connection_time;
}

BOOL xSocket::isConnectionTimeout(DWORD timeout_ms) const
{
	return getConnectionAge() >= timeout_ms;
}

DWORD xSocket::getIdleTime() const
{
	DWORD current_time = getCurrentTime();
	return current_time - m_stats.last_activity;
}
