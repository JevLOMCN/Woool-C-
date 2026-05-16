#include "StdAfx.h"
#include ".\server.h"

CServer* CServer::m_pInstance = nullptr;
CServer::CServer(void)
{
	m_pInstance = this;
}

CServer::~CServer(void)
{
}

CClientObject* CServer::NewClientObject()
{
	return newObject();
}

VOID CServer::DeleteClientObject(CClientObject* pObject)
{
	deleteObject((CClientObj*)pObject);
}

BOOL CServer::InitServer(CSettingFile& s)
{
	int maxconnection = s.GetInteger(m_pServerName, "MaxConnection", MAX_CLIENTOBJECT);
	create(maxconnection);
	m_EnterObject.Create(maxconnection);
	CBaseServer::GetIoConsole()->OutPut(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);
	return TRUE;
}

VOID CServer::CleanServer()
{
}

VOID CServer::Update()
{
	CClientObj* pObject = m_ObjectPool.First();
	while (pObject)
	{
		pObject->Update();
		pObject = m_ObjectPool.Next();
	}
	UpdateSCServer();
	UpdateDBServer();
}

VOID CServer::OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize)
{
	switch (wCmd)
	{
	case MAS_RESTARTGAME:
	{
		ENTERGAMESERVER* pEnterInfo = (ENTERGAMESERVER*)pszData;
		SERVER_ERROR ret = SE_OK;
		ret = AddEnterAccount(pEnterInfo->nLoginId, pEnterInfo->szAccount, pEnterInfo->nSelCharId);
		GetIoConsole()->OutPut(SUCCESS_GREEN, "%s 重新开始游戏!\n", pEnterInfo->szName);
	}
	break;
	case MAS_ENTERSELCHARSERVER:
	{
		//	id/lid/account
		char* Params[5];
		char szRetString[200] = "";
		int nParam = SearchParam((char*)pszData, Params, 5, '/');
		SERVER_ERROR ret = SE_OK;
		UINT nClientId = static_cast<UINT>(atoi(Params[0]));
		if (nParam == 3)
		{
			UINT nLoginId = static_cast<UINT>(atoi(Params[1]));
			UINT nSelCharId = 0;
			if ((ret = AddEnterAccount(nLoginId, Params[2], nSelCharId)) == SE_OK)
			{
				//	data = "lid/sid/account"
				sprintf_s(szRetString, sizeof(szRetString), "%u/%u/%s",
					nLoginId, nSelCharId, Params[2]);
				m_SCClientObj.SendMsgAcrossServer(nClientId, MAS_ENTERSELCHARSERVER, MST_SINGLE, wIndex, szRetString, static_cast<int>(strlen(szRetString)));
				break;
			}
		}
		else
			ret = SE_INVALIDPARAM;

		sprintf_s(szRetString, sizeof(szRetString), "FAIL/%d", ret);
		m_SCClientObj.SendMsgAcrossServer(nClientId, MAS_ENTERSELCHARSERVER, MST_SINGLE, wIndex, szRetString, static_cast<int>(strlen(szRetString)));
	}
	break;
	}
}

SERVER_ERROR CServer::AddEnterAccount(UINT nLoginId, const char* pszAccount, UINT& nSelCharId)
{
	LOGINENTER* pEnter = nullptr;
	UINT id = m_EnterObject.New(&pEnter);
	if (id == 0 || pEnter == nullptr)
		return	SE_SERVERFULL;
	if (!m_Inthash.HAdd(nLoginId, id))
	{
		m_EnterObject.Del(id);
		return SE_SERVERFULL;
	}
	pEnter->nListId = id;
	pEnter->nLid = nLoginId;
	pEnter->nSid = nSelCharId == 0 ? static_cast<UINT>(Getrand(100000000)) : nSelCharId;
	strncpy(pEnter->szAccount, pszAccount, 10);
	pEnter->szAccount[10] = 0;
	pEnter->dwEnterTime = timeGetTime();

	nSelCharId = pEnter->nSid;
	return SE_OK;
}

BOOL CServer::GetLoginEnterInfo(UINT nLoginId, UINT nSelCharId, LOGINENTER& info)
{
	UINT id = (UINT)m_Inthash.HGet(nLoginId);
	if (id == 0)return FALSE;
	LOGINENTER* pInfo = m_EnterObject.Get(id);
	//memcpy( &info, pInfo, sizeof( LOGINENTER ));
	if (pInfo == nullptr || pInfo->nSid != nSelCharId || pInfo->nLid != nLoginId)
		return FALSE;
	info = *pInfo;
	m_EnterObject.Del(id);
	m_Inthash.HDel(nLoginId);
	return TRUE;
}

