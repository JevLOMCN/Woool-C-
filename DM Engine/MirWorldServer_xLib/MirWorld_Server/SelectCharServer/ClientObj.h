#pragma once
typedef struct tagLOGINENTER
{
	tagLOGINENTER()
	{
		FILLSELF(0);
	}
	CHAR szAccount[12];
	UINT nLid;
	UINT nSid;
	DWORD dwEnterTime;
	UINT nListId;
}LOGINENTER;

typedef struct tagCHARLISTNODE
{
	char szName[32];
	BYTE btLevel;
	BYTE btHair;
	BYTE btSex;
	BYTE btWhat;
}CHARLISTNODE;

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
protected:
	UINT m_nLoginId;
	UINT m_nSelectId;
	BOOL QueryCharList();
	VOID SendCharList(tQueryCharList_Result* pResult);
	VOID SendDelCharList(tQueryCharList_Result* pResult);
	LOGINENTER m_EnterInfo;
	selcharserverstate m_State;
	FINDSERVER_RESULT m_GameServerAddr;
	UINT m_nFailCount;
	CServerTimer m_TimeOut;
	BOOL m_bSelected;
	CHAR m_szCharName[32];
	BOOL m_bWaitForVerify;
	CServerTimer m_WaitForVerifyTimer;
};