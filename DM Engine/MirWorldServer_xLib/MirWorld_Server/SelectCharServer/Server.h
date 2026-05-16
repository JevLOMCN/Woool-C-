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
	VOID OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize);
	static CServer* GetInstance()
	{
		if (m_pInstance == nullptr)
			m_pInstance = new CServer;
		return m_pInstance;
	}
	BOOL GetLoginEnterInfo(UINT nLoginId, UINT nSelCharId, LOGINENTER& info);
protected:
	SERVER_ERROR AddEnterAccount(UINT nLoginId, const char* pszAccount, UINT& nSelCharId);
	static CServer* m_pInstance;
	CIndexListEx<LOGINENTER/*, MAX_CLIENTOBJECT*/>	m_EnterObject;
	CIntHash<1024> m_Inthash;
};
