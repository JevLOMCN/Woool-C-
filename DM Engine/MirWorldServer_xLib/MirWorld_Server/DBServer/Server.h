#pragma once
#include "clientobj.h"

constexpr UINT MAX_CLIENTOBJECT = 64;

class CServer : public CBaseServer, public xIndexObjectPool<CClientObj>
{
public:
	CServer(void);
	virtual ~CServer(void);
public:
	void OnInput(const char* pString) { CBaseServer::OnInput(pString); }
	CClientObject* GetClientObject(UINT id) { return getObject(id); }
	CClientObject* NewClientObject();
	VOID DeleteClientObject(CClientObject* pObject);
	BOOL InitServer(CSettingFile&);
	VOID CleanServer();
	VOID Update();
	static CServer* GetInstance()
	{
		if (m_pInstance == nullptr)
			m_pInstance = new CServer;
		return m_pInstance;
	}
	VOID OnParseMsg(WORD wMsg);
protected:
	CServerTimer m_xxShowTimer;
	DWORD m_dwMsgTimes[DM_END];
	CAppDB m_appDB;
	static CServer* m_pInstance;
	char m_szServer[64];// = (char*)s.GetString( m_pServerName, "server", "localhost" );
	char m_szPort[64];// = (char*)s.GetString( m_pServerName, "port", "3306" );
	char m_szDatabase[64];// = (char*)s.GetString( m_pServerName, "database", "mirworlddb" );
	char m_szAccount[64];// = (char*)s.GetString( m_pServerName, "account", "root" );
	char m_szPassword[64];// = (char*)s.GetString( m_pServerName, "password", "root" );
};
