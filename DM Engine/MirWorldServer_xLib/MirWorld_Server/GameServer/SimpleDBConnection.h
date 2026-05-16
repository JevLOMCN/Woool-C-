#pragma once
#include "xsimplesocket.h"

class CSimpleDBConnection :
	public xSimpleSocket
{
public:
	CSimpleDBConnection(void);
	virtual ~CSimpleDBConnection(void);
	VOID Update();
	VOID SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData = nullptr, int datasize = -1);
	virtual VOID OnMsg(MIRMSG* pMsg, int datasize) {}
protected:
	int	ParseMessage(char* pszMsg, int iSize);

	xPacket	m_xSendPacket;
	xPacket m_xRecvPacket;
};
