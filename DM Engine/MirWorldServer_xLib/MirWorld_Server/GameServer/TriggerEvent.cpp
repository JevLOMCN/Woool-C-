#include "stdafx.h"
#include "TriggerEvent.h"
#include "visibleevent.h"
#include "aliveobject.h"
#include "humanplayer.h"
#include "logicmap.h"
#include "logicmapmgr.h"
#include "eventmanager.h"
#include "systemscript.h"

xObjectPool<TriggerEvent> TriggerEvent::m_xPool;
TriggerEvent::TriggerEvent(void) : m_pVisibleEvent(nullptr)
{
	m_szMapName[0] = '\0';
	m_szPage[0] = '\0';
}

TriggerEvent::~TriggerEvent(void)
{
	Destroy();
}

TriggerEvent* TriggerEvent::Create(CAliveObject* pOwner, const char* szMapName, int x, int y, DWORD dwType, const char* m_szPage, UINT nKeepTime, UINT nRunTime)
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapByTitle(szMapName);
	if (pMap == nullptr)return nullptr;
	if (x >= pMap->GetWidth())return nullptr;
	if (y >= pMap->GetHeight())return nullptr;
	TriggerEvent* pEvent = NewEvent();
	if (pEvent == nullptr)return nullptr;
	if (!CEventManager::GetInstance()->CheckNearEvent(pMap, x, y, 1, dwType))
		pEvent->m_pVisibleEvent = CEventManager::GetInstance()->NewVisibleEvent(pMap, x, y, dwType, nRunTime, nKeepTime, pEvent);
	if (pEvent->m_pVisibleEvent == nullptr)
	{
		DeleteEvent(pEvent);
		return nullptr;
	}
	pEvent->m_pOwner = pOwner;
	if (pOwner)pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();
	o_strncpy(pEvent->m_szMapName, szMapName, 64);
	o_strncpy(pEvent->m_szPage, m_szPage, 64);
	CEventManager::GetInstance()->AddEventProcessor(pEvent);
	return pEvent;
}

VOID TriggerEvent::Update()
{
	if (m_pOwner && this->m_pOwner->GetInstanceKey() != this->m_dwOwnerInstanceKey)
		Destroy();
	if (this->m_pVisibleEvent) this->m_pVisibleEvent->UpdateValid();
}

VOID TriggerEvent::Destroy()
{
	CEventManager::GetInstance()->RemoveEventProcessor(this);
	if (this->m_pVisibleEvent)
	{
		m_pVisibleEvent->Close();
		m_pVisibleEvent = nullptr;
	}
	m_pOwner = nullptr;
	m_dwOwnerInstanceKey = 0;
}

VOID TriggerEvent::OnUpdate(CVisibleEvent* pEvent)
{
	CLogicMap* pMap = pEvent->GetMap();
	if (pMap == nullptr)return;
	int dx = pEvent->getX(), dy = pEvent->getY();
	CMapCellInfo* pInfo = pMap->GetMapCellInfo(dx, dy);
	if (pInfo && pInfo->m_xObjectList.getCount() > 0)
	{
		xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
		while (pNode)
		{
			if (pNode->getObject() && pNode->getObject()->GetClassType() == CLS_ALIVEOBJECT)
			{
				CAliveObject* pAObj = (CAliveObject*)pNode->getObject();
				if (pAObj->GetType() == OBJ_PLAYER && !pAObj->IsDeath())
				{
					CHumanPlayer* pPlayer = (CHumanPlayer*)pAObj;
					CSystemScript::GetInstance()->Execute(pPlayer->GetScriptTarget(), this->m_szPage, FALSE);
				}
			}
			pNode = pNode->getNext();
		}
	}
}

VOID TriggerEvent::OnClose(CVisibleEvent* pEvent)
{
	if (pEvent == m_pVisibleEvent)
	{
		m_pVisibleEvent = nullptr;
		Destroy();
	}
}