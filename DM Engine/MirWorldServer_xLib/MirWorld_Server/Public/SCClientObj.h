#pragma once
#include "clientobject.h"

class CSCClientObj :
	public CClientObject
{
public:
	CSCClientObj(void);
	virtual ~CSCClientObj(void);
	void Update();
	//	×¢²á·þÎñÆ÷
	VOID RegisterServer(SERVERADDR& Addr, ServerId& Id, const char* pszNickName);
	VOID UpdateInfo(DWORD dwConnections);
	VOID FindServer(UINT id, servertype type, const char* pszServerName);
	VOID SendMsgAcrossServer(UINT id, WORD wCmd, MAS_SENDTYPE sendtype, WORD wParam, const char* pszData, int datasize);
	VOID GetGameServerAddr(UINT id, const char* pszServerName, const char* pszMapInfo);
	xPacket* GetPacket() { return m_xPacketQueue.pop(); }
	//int	GetSendBufferSize(){ return (2048 * 1024);}
	//int	GetRecvBufferSize(){ return (2048 * 1024);}
};