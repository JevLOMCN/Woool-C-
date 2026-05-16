#pragma once
#include "appdb.h"

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
	CAppDB& GetAppDB() { return m_appDB; }
private:
	CAppDB m_appDB;
	char m_szBuffer[65536];
};