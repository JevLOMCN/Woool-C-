#pragma once

class CClientObj :
	public CClientObject
{
public:
	CClientObj(void);
	virtual ~CClientObj(void);
	VOID Clean();
	VOID Update();
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	VOID OnDBMsg(PMIRMSG pMsg, int datasize);
	VOID OnSCMsg(PMIRMSG pMsg, int datasize);
	VOID OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize);
	VOID OnConnection();
private:
	VOID SendLoginSuccess(UINT nLoginId);
	VOID SendLoginFail(DWORD error);
	VOID SendSelectServerOk();
	FINDSERVER_RESULT	m_SelectCharServer;
	CHAR m_szAccount[20];
	UINT m_nLid;
	UINT m_nSid;
	UINT m_nFailCount;
	CServerTimer m_TimeOut;
};