#include "StdAfx.h"
#include ".\clientobj.h"
#include ".\server.h"

CClientObj::CClientObj(void)
{
	Clean();
}

CClientObj::~CClientObj(void)
{
}

void CClientObj::Clean()
{
	CClientObject::Clean();
	memset(&m_RegInfo, 0, sizeof(m_RegInfo));
}

VOID CClientObj::Update()
{
	CClientObject::Update();
}

VOID CClientObj::OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size)
{
}

VOID CClientObj::OnCodedMsg(xClientObject* pObject, PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	CIOConsole* pConsole = (CIOConsole*)pServer->GetIoConsole();

	switch (pMsg->wCmd)
	{
	case SCM_GETGAMESERVERADDR:
	{
		pMsg->wParam[0] = pServer->FindServer(ST_GAMESERVER, pMsg->data, (FINDSERVER_RESULT*)pMsg->data);
		if (pMsg->wParam[0] == SE_OK)
			datasize = sizeof(FINDSERVER_RESULT);
		SendMsg(pMsg, datasize);
	}
	break;
	case SCM_REGISTERSERVER:
	{
		pMsg->wParam[0] = pServer->RegisterServer(this, (REGISTER_SERVER_INFO*)pMsg->data, (REGISTER_SERVER_RESULT*)pMsg->data);
		SendMsg(pMsg, sizeof(REGISTER_SERVER_RESULT));
		if (pMsg->wParam[0] != SE_OK)
		{
			Disconnect(1000);
		}
	}
	break;
	case SCM_UPDATESERVERINFO:
	{
		m_RegInfo.dwConnections = pMsg->dwFlag;
	}
	break;
	case SCM_FINDSERVER:
	{
		pMsg->wParam[0] = pServer->FindServer((servertype)pMsg->wParam[0], pMsg->data, (FINDSERVER_RESULT*)pMsg->data);
		if (pMsg->wParam[0] == SE_OK)
			datasize = sizeof(FINDSERVER_RESULT);
		SendMsg(pMsg, datasize);
	}
	break;
	case SCM_MSGACROSSSERVER:
	{
		UINT id = pMsg->wParam[2];
		CClientObj* pObj = (CClientObj*)pServer->getObject(id);
		if (pObj)
		{
			pMsg->wParam[1] = m_RegInfo.Ident.bType;
			pMsg->wParam[2] = getId();
			pObj->SendMsg(pMsg, datasize);
		}
	}
	break;
	}
}