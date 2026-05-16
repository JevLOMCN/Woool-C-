#include "StdAfx.h"
#include ".\scriptevent.h"
#include ".\logicmapmgr.h"
#include ".\logicmap.h"
#include ".\systemscript.h"
#include ".\humanplayer.h"

xObjectPool<CScriptEvent> CScriptEvent::m_xScriptEventPool;
xListHost<CScriptEvent> CScriptEvent::m_xList;

CScriptEvent::CScriptEvent(void) : m_xNode(this), m_dwFlag(0)
{
	m_szScriptPage[0] = '\0';
}

CScriptEvent::~CScriptEvent(void)
{
	//	Release();
}

CScriptEvent* CScriptEvent::Create(UINT nMapId, UINT x, UINT y, DWORD dwFlag, const char* pszPage)
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
	if (pMap == nullptr)return nullptr;
	if (pMap->IsPhysicsBlocked(x, y))return nullptr;
	if (dwFlag == 0)return nullptr;
	if (pszPage[0] == 0)return nullptr;
	CScriptEvent* pEvent = m_xScriptEventPool.newObject();
	//pEvent->m_szScriptPage[0] = 0;
	o_strncpy(pEvent->m_szScriptPage, pszPage, 250);
	pEvent->m_dwFlag = dwFlag;
	pEvent->SetMapId(nMapId);
	pEvent->setXY(x, y);
	if (!pMap->AddObject(pEvent))
	{
		m_xScriptEventPool.deleteObject(pEvent);
		return nullptr;
	}
	m_xList.addNode(&pEvent->m_xNode);
	return pEvent;
}

VOID CScriptEvent::Release()
{
	if (m_pMap)
	{
		m_pMap->RemoveObject(this);
	}
	m_dwFlag = 0;
	m_szScriptPage[0] = 0;
	m_xScriptEventPool.deleteObject(this);
	m_xList.removeNode(&m_xNode);
}

VOID CScriptEvent::OnEnter(CMapObject* pObject)
{
	if (pObject->GetType() == OBJ_PLAYER && (m_dwFlag & SEF_ENTER) != 0)
	{
		CHumanPlayer* pPlayer = (CHumanPlayer*)pObject;
		CSystemScript::GetInstance()->Execute(pPlayer->GetScriptTarget(), this->m_szScriptPage, FALSE);
	}
}

VOID CScriptEvent::OnLeave(CMapObject* pObject)
{
	if (pObject->GetType() == OBJ_PLAYER && (m_dwFlag & SEF_LEAVE) != 0)
	{
		CHumanPlayer* pPlayer = (CHumanPlayer*)pObject;
		CSystemScript::GetInstance()->Execute(pPlayer->GetScriptTarget(), this->m_szScriptPage, FALSE);
	}
}

VOID CScriptEvent::OnEnterMap(CLogicMap* pMap)
{
	if (m_dwFlag & SEF_ENTER)
		SetEnterFlag(pMap);
	if (m_dwFlag & SEF_LEAVE)
		SetLeaveFlag(pMap);
	CMapObject::OnEnterMap(pMap);
}
