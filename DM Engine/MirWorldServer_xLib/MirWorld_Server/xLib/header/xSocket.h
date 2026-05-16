#pragma once
#include <WinSock2.h>
#include <Mstcpip.h>
#include "xsupport.h"
#include "xerror.h"
#include <atomic>

class xSocket : public xError
{
public:
	enum socket_state
	{
		SS_UNINITED,
		SS_UNUSED,
		SS_ACCEPTED,
		SS_CONNECTED,
		SS_LISTENING,
		SS_DISCONNECTED,
	};

	xSocket();
	xSocket(SOCKET Socket);

	virtual ~xSocket(void);

	BOOL makeSocket(int af = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);
	BOOL setSocket(SOCKET Socket);

	BOOL setSocket(xSocket& socket) { if (!setSocket(socket.getSocketFd()))return FALSE; setState(socket.getState()); setEndPoint(socket.getAddress(), socket.getPort()); return TRUE; }
	BOOL steelSocket(xSocket& socket)
	{
		if (!setSocket(socket)) return FALSE;
		socket.clear();
		return TRUE;
	}

	BOOL connect(const char* cp, UINT nPort);

	BOOL listen(const char* cp, UINT nPort);
	BOOL listen(UINT nPort);
	BOOL setNoBlocking();
	BOOL setBlocking();

	SOCKET getSocketFd()const { return m_Socket; }

	socket_state getState() { return m_state.load(); }
	void setState(socket_state state) { m_state.store(state); }

	VOID close();
	VOID clear() { setSocket(INVALID_SOCKET); }

	BOOL sendEx(LPVOID lpData, int nDatasize, DWORD& dwBytesSent, DWORD dwFlag, LPOVERLAPPED lpOverlapped);
	BOOL recvEx(LPVOID lpDataBuf, int nBufsize, DWORD& dwBytesReceived, DWORD& dwFlag, LPOVERLAPPED lpOverlapped);
	BOOL acceptEx(xSocket& sAccept, LPVOID lpDataBuf, DWORD dwRecvBufferLength, DWORD& dwBytesReceived, LPOVERLAPPED lpOverlapped);

	void setEndPoint(const char* cp, UINT nPort) { strncpy_s(m_szAddress, sizeof(m_szAddress), cp, _TRUNCATE); m_nPort = nPort; }

	const char* getAddress() { return m_szAddress; }
	UINT getPort() { return m_nPort; }

	// 设置 socket 选项
	BOOL setReuseAddr(BOOL reuse = TRUE);
	BOOL setKeepAlive(BOOL keepalive = TRUE, UINT keepalive_time = 7200, UINT keepalive_interval = 75);
	BOOL setTcpNoDelay(BOOL nodelay = TRUE);
	BOOL setSendBuffer(UINT size);
	BOOL setRecvBuffer(UINT size);

	// 获取 socket 统计信息
	struct SocketStats {
		DWORD bytes_sent = 0;
		DWORD bytes_received = 0;
		DWORD packets_sent = 0;
		DWORD packets_received = 0;
		DWORD connection_time = 0;
		DWORD last_activity = 0;
	};
	SocketStats getStats() const { return m_stats; }
	
	// 获取连接时长（毫秒）
	DWORD getConnectionAge() const;
	
	// 检查连接是否超时
	BOOL isConnectionTimeout(DWORD timeout_ms) const;
	
	// 检查空闲时间（毫秒）
	DWORD getIdleTime() const;
private:
	char m_szAddress[40];
	UINT m_nPort;
	SOCKET m_Socket;
	std::atomic<socket_state> m_state;
	SocketStats m_stats;  // 统计状态信息
	// 辅助函数
	BOOL setSocketOption(int level, int optname, const void* optval, int optlen);
	DWORD getCurrentTime() const;
};