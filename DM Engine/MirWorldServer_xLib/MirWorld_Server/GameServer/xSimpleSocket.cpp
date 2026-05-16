#include "StdAfx.h"
#include "xsimplesocket.h"

xSimpleSocket::xSimpleSocket(void)
{
	m_Socket = INVALID_SOCKET;
	m_Status = SS_EMPTY;
}

xSimpleSocket::~xSimpleSocket(void)
{
	Close();
}

xSimpleSocket::xSimpleSocket(SOCKET sInit, e_socket_status status)
{
	m_Socket = sInit;
	m_Status = status;
}

xSimpleSocket::xSimpleSocket(xSimpleSocket& socket)
{
	m_Socket = socket.m_Socket;
	m_Status = socket.m_Status;
}

VOID xSimpleSocket::SetSocket(SOCKET s, e_socket_status status)
{
	Close();
	m_Socket = s;
	m_Status = status;
}

BOOL xSimpleSocket::Socket(int af, int type, int protocol)
{
	if (m_Socket != INVALID_SOCKET)
		Close();

	m_Socket = socket(af, type, protocol);
	if (m_Socket == INVALID_SOCKET)
	{
		OnError("Socket() º¯ÊýÖ´ÐÐÊ§°Ü!");
		return TRUE;
	}
	return TRUE;
}

BOOL xSimpleSocket::SetNoBlocking()
{
	unsigned long mode = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) == SOCKET_ERROR)
	{
		OnError("SetNoBlocking() ÖÐ ioctlsocket() Ê§°Ü!");
		return FALSE;
	}
	return TRUE;
}

BOOL xSimpleSocket::SetBlocking()
{
	unsigned long mode = 0;
	if (ioctlsocket(m_Socket, FIONBIO, &mode) == SOCKET_ERROR)
	{
		OnError("SetBlocking() ÖÐ ioctlsocket() Ê§°Ü!");
		return FALSE;
	}
	return TRUE;
}

BOOL xSimpleSocket::Select(PBOOL pbRead, PBOOL pbWrite, PBOOL pbExcept, DWORD dwTimeOut)
{
	fd_set fdRead = { 0 };
	fd_set fdWrite = { 0 };
	fd_set fdExcept = { 0 };

	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdExcept);

	fd_set* pfdRead = &fdRead, * pfdWrite = &fdWrite, * pfdExcept = &fdExcept;

	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = dwTimeOut;

	if (pbRead == nullptr)
		pfdRead = nullptr;
	else
	{
		FD_SET(m_Socket, pfdRead);
		//*pbRead = TRUE;
	}
	if (pbWrite == nullptr)
		pfdWrite = nullptr;
	else
	{
		FD_SET(m_Socket, pfdWrite);
		//*pbWrite = TRUE;
	}
	if (pbExcept == nullptr)
		pfdExcept = nullptr;
	else
	{
		FD_SET(m_Socket, pfdExcept);
		//*pbExcept = TRUE;
	}

	int ierr = select(0, pfdRead, pfdWrite, pfdExcept, &tv);
	if (ierr == SOCKET_ERROR)
	{
		OnError("Select() ÖÐµÄ select Ö´ÐÐÊ§°Ü!");
		return FALSE;
	}

	if (pfdRead && FD_ISSET(m_Socket, pfdRead))
		*pbRead = TRUE;
	if (pfdWrite && FD_ISSET(m_Socket, pfdWrite))
		*pbWrite = TRUE;
	if (pfdExcept && FD_ISSET(m_Socket, pfdExcept))
		*pbExcept = TRUE;
	return TRUE;
}

BOOL xSimpleSocket::Connect(const char* pszAddress, UINT nPort)
{
	struct sockaddr_in ServAddr;
	LPHOSTENT pHost = gethostbyname(pszAddress);
	if (pHost == nullptr)
	{
		OnError("Connect() ÖÐ gethostbyname() Ê§°Ü!");
		return FALSE;
	}

	if (!Socket())
		return FALSE;

	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = *(ULONG*)pHost->h_addr_list[0];
	ServAddr.sin_port = htons(nPort);

	int erri = connect(m_Socket, (struct sockaddr*)&ServAddr, sizeof(ServAddr));
	if (erri == SOCKET_ERROR)
	{
		//closesocket( s );
		OnError("Connect() ÖÐ connect() Ê§°Ü!");
		return FALSE;
	}
	m_Status = SS_CONNECTED;
	return TRUE;
}

BOOL xSimpleSocket::ConnectNoBlocking(const char* pszAddress, UINT nPort)
{
	struct sockaddr_in ServAddr;
	LPHOSTENT pHost = gethostbyname(pszAddress);
	if (pHost == nullptr)
	{
		OnError("ConnectNoBlocking() ÖÐ gethostbyname() Ê§°Ü!");
		return FALSE;
	}

	if (!Socket())
		return FALSE;
	if (!SetNoBlocking())
		return FALSE;

	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = *(ULONG*)pHost->h_addr_list[0];
	ServAddr.sin_port = htons(nPort);

	int erri = connect(m_Socket, (struct sockaddr*)&ServAddr, sizeof(ServAddr));
	if (erri == SOCKET_ERROR)
	{
		//closesocket( s );
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			OnError("ConnectNoBlocking() ÖÐ connect() Ê§°Ü!");
			return FALSE;
		}
	}
	m_Status = SS_CONNECTING;
	return TRUE;
}

BOOL xSimpleSocket::Connected()
{
	BOOL bSuccess = FALSE;
	if (Select(nullptr, &bSuccess, nullptr, 1))
	{
		if (bSuccess)
		{
			m_Status = SS_CONNECTED;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL xSimpleSocket::Close()
{
	if (m_Socket != INVALID_SOCKET)
	{
		int ierr = closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		if (ierr == SOCKET_ERROR)
		{
			OnError("Close() ÖÐ closesocket() Ê§°Ü!");
			return FALSE;
		}
	}
	if (m_Status != SS_EMPTY)
	{
		m_Status = SS_EMPTY;
		//OnClose();
	}
	return TRUE;
}

int	xSimpleSocket::Send(LPVOID lpBuffer, int iSize)const
{
	return send(m_Socket, (char*)lpBuffer, iSize, 0);
}

int	xSimpleSocket::Recv(LPVOID lpBuffer, int iSize)const
{
	return recv(m_Socket, (char*)lpBuffer, iSize, 0);
}

BOOL xSimpleSocket::Listen(const char* pszAddress, UINT nPort)
{
	struct sockaddr_in serveraddr;
	if (!this->Socket())return FALSE;

	memset((void*)&serveraddr, 0, sizeof(struct sockaddr_in));

	serveraddr.sin_family = PF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(nPort);

	if (::bind(m_Socket, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{
		this->Close();
		return FALSE;
	}
	if (listen(m_Socket, 64) == SOCKET_ERROR)
	{
		this->Close();
		return FALSE;
	}
	m_Status = SS_LISTEN;
	return TRUE;
}

BOOL xSimpleSocket::Accept(xSimpleSocket& socket)
{
	sockaddr_in	addr = { 0 };
	int addrlen;
	addrlen = sizeof(addr);
	SOCKET sAccept = accept(m_Socket, (sockaddr*)&addr, &addrlen);
	if (sAccept == INVALID_SOCKET)return FALSE;
	socket.Close();
	socket.m_Socket = sAccept;
	socket.m_Status = SS_ACCEPTED;
	return TRUE;
}

VOID xSimpleSocket::NetStartup()
{
	WSADATA	wsa;
	int result = WSAStartup(0x101, &wsa);
	if (result != 0)
	{
		OnError("NetStartup() ÖÐ WSAStartup failed with error: " + result);
		return;
	}
}

VOID xSimpleSocket::NetCleanup()
{
	WSACleanup();
}

VOID xSimpleSocket::Clear()
{
	m_Socket = INVALID_SOCKET;
	m_Status = SS_EMPTY;
}
