#include "StdAfx.h"
#include ".\clientobject.h"

static thread_local char g_szTempBuffer1[65536];
static thread_local char g_szTempBuffer2[65536];

CClientObject::CClientObject(void)
{
	m_xParsePacket.create(65536);
	SetMsgProcessor(this);
}

CClientObject::~CClientObject(void)
{
	m_xParsePacket.destroy();
}

VOID CClientObject::Clean(void)
{
	xClientObject::Clean();
	m_dwClientKey = timeGetTime();
	m_Pingtimer.Savetime();
	m_bPingNoRet = FALSE;
	m_dwPing = 0;
	m_xParsePacket.clear();
}

BOOL CClientObject::SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData, int datasize)
{
	int length = EncodeMsg(g_szTempBuffer1, dwFlag, wCmd, w1, w2, w3, lpData, datasize);
	return PostMsg((char*)g_szTempBuffer1, length);
}

BOOL CClientObject::SendMsg(PMIRMSG pMsg, int datasize)
{
	g_szTempBuffer1[0] = '#';
	int length = _CodeGameCode((BYTE*)pMsg, datasize + sizeof(MIRMSGHEADER), (BYTE*)(g_szTempBuffer1 + 1));
	g_szTempBuffer1[length + 1] = '!';
	return PostMsg((char*)g_szTempBuffer1, length + 2);
}

BOOL CClientObject::PostMsg(const char* pszCodedMsg, int size)
{
	return postSend((LPVOID)pszCodedMsg, size);
}

VOID CClientObject::Update()
{
	xClientObject::Update();
}

VOID CClientObject::OnClientPingRet()
{
	m_dwPing = timeGetTime() - m_Pingtimer.GetSavedTime();
}

VOID CClientObject::OnSCMsg_T(PMIRMSG pMsg, int datasize)
{
	switch (pMsg->wCmd)
	{
	case SCM_MSGACROSSSERVER:
	{
		OnMASMsg(pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
	}
	break;
	default:
		OnSCMsg(pMsg, datasize);
		break;
	}
}

void CClientObject::OnDataPacket(xPacket* pPacket)
{
	if (!m_xParsePacket.push((LPVOID)pPacket->getbuf(), static_cast<int>(pPacket->getsize())))
	{
		Disconnect();
		return;
	}
	char* pszMsg = (char*)m_xParsePacket.getbuf();
	int msgptr = 0;
	int parsesize = 0;
	int allsize = static_cast<int>(m_xParsePacket.getsize());
	if (allsize == 0)return;
	do
	{
		parsesize = ParseMessage(pszMsg + msgptr, allsize - msgptr);
		msgptr += parsesize;
	} while (parsesize > 0 && allsize > msgptr);

	if (msgptr >= allsize)
		m_xParsePacket.clear();
	else
		m_xParsePacket.free(msgptr);
}

int	CClientObject::ParseMessage(char* pszMsg, int iSize)
{
	char* pStart = nullptr;
	int ParsedSize = 0;
	for (int i = 0;i < iSize;i++)
	{
		if (pszMsg[i] == '*')
		{
			if (m_bPingNoRet)
			{
				m_bPingNoRet = FALSE;
				OnClientPingRet();
			}
			ParsedSize = i + 1;
		}
		else if (pszMsg[i] == '#')
		{
			pStart = pszMsg + i + 1;
		}
		else if (pszMsg[i] == '!')
		{
			if (pStart != nullptr)
			{
				pszMsg[i] = 0;
				if (*pStart == '+')
				{
					//	OnUnCodedMsg( pStart );
					if (m_pMsgProcessor)
						m_pMsgProcessor->OnUnCodeMsg(this, pStart, static_cast<int>(pszMsg + i - pStart));
				}
				else
				{
					if (*pStart >= '0' && *pStart <= '9')pStart++;
					int length = static_cast<int>(_UnGameCode(pStart, (BYTE*)g_szTempBuffer2));
					if (m_pMsgProcessor)
						m_pMsgProcessor->OnCodedMsg(this, (MIRMSG*)g_szTempBuffer2, length - sizeof(MIRMSGHEADER));
				}
				pszMsg[i] = '!';
				pStart = nullptr;
			}
			ParsedSize = i + 1;
		}
	}
	return ParsedSize;
}
