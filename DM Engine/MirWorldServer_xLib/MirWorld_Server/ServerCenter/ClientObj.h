#pragma once
typedef struct tagREGISTEREDSERVER
{
	char szName[64];
	ServerId Ident;
	SERVERADDR Addr;
	int iSendDBCount;
	DWORD dwConnections;
}REGISTEREDSERVER;

class CClientObj :
	public CClientObject
{
public:
	CClientObj(void);
	virtual ~CClientObj(void);
	VOID Clean();
	VOID Update();
	VOID OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size);
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	REGISTEREDSERVER* GetRegInfo() { return &m_RegInfo; }
private:
	REGISTEREDSERVER m_RegInfo;
};