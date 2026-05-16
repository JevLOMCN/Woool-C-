#pragma once
#include "clientobj.h"

constexpr UINT MAX_CLIENTOBJECT = 2048;

class CServer :
	public CBaseServer,
	public xIndexObjectPool<CClientObj>
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
	BOOL RegisterDisabled() {
		return m_bDisableRegister;
	}
	const char* GetServerTips() { return m_pszServerTips == nullptr ? "" : m_pszServerTips; }
	const char* GetLoginOkTips() { return m_pszLoginOkTips == nullptr ? "" : m_pszLoginOkTips; }
	const char* GetRegisterTips() { return m_pszRegisterTips == nullptr ? "" : m_pszRegisterTips; }
	const char* GetWAddr() 
	{ 
		return m_pszWAddr == nullptr ? "" : m_pszWAddr; 
	}
	const int GetWPort()const { return m_nWPort; }
protected:
	char* m_pszServerTips;
	char* m_pszLoginOkTips;
	char* m_pszRegisterTips;
	BOOL m_bDisableRegister;
	char* m_pszWAddr; // Î¢¶ËIPµØÖ·
	int m_nWPort; // Î¢¶Ë¶Ë¿Ú
	static CServer* m_pInstance;
};
