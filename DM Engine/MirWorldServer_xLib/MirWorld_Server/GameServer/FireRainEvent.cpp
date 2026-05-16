#include "stdafx.h"
#include "FireRainEvent.h"
#include ".\visibleevent.h"
#include ".\aliveobject.h"
#include ".\logicmap.h"
#include ".\logicmapmgr.h"
#include ".\eventmanager.h"

xObjectPool<FireRainEvent> FireRainEvent::m_xPool;
FireRainEvent::FireRainEvent(void) : m_pVisibleEvent(nullptr), m_pOwner(nullptr), m_dwOwnerInstanceKey(0), m_nDamage(0), m_nRange(0)
{
}

FireRainEvent::~FireRainEvent(void)
{
	Destroy();
}

FireRainEvent* FireRainEvent::Create(CAliveObject* pOwner, int x, int y, UINT nRange, UINT nDamage, UINT nKeepTime, UINT nRunTime)
{
	if (pOwner == nullptr) return nullptr;
	CLogicMap* pMap = pOwner->GetMap();
	if (pMap == nullptr)return nullptr;
	if (x >= pMap->GetWidth())return nullptr;
	if (y >= pMap->GetHeight())return nullptr;
	FireRainEvent* pEvent = NewEvent();
	if (pEvent == nullptr)return nullptr;
	if (!CEventManager::GetInstance()->CheckNearEvent(pMap, x, y, 1, VE_FIRERAIN))
		pEvent->m_pVisibleEvent = CEventManager::GetInstance()->NewVisibleEvent(pMap, x, y, VE_FIRERAIN, nRunTime, nKeepTime, pEvent);
	if (pEvent->m_pVisibleEvent == nullptr)
	{
		DeleteEvent(pEvent);
		return nullptr;
	}
	pEvent->m_pOwner = pOwner;
	if (pOwner)pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();
	pEvent->m_nDamage = nDamage;
	pEvent->m_nRange = nRange;
	CEventManager::GetInstance()->AddEventProcessor(pEvent);
	return pEvent;
}

VOID FireRainEvent::Update()
{
	if (m_pOwner && this->m_pOwner->GetInstanceKey() != this->m_dwOwnerInstanceKey)
		Destroy();
	if (this->m_pVisibleEvent)this->m_pVisibleEvent->UpdateValid();
}

VOID FireRainEvent::Destroy()
{
	CEventManager::GetInstance()->RemoveEventProcessor(this);
	if (this->m_pVisibleEvent)
	{
		m_pVisibleEvent->Close();
		m_pVisibleEvent = nullptr;
	}
	m_pOwner = nullptr;
	m_dwOwnerInstanceKey = 0;
	m_nDamage = 0;
	m_nRange = 0;
}

VOID FireRainEvent::OnUpdate(CVisibleEvent* pEvent)
{
	CLogicMap* pMap = pEvent->GetMap();
	if (pMap == nullptr)return;
	int mw = pMap->GetWidth() - 1;
	int mh = pMap->GetHeight() - 1;
	int dx = pEvent->getX(), dy = pEvent->getY();
	int nStartX = dx - m_nRange;
	int nStartY = dy - m_nRange;
	int nEndX = dx + m_nRange;
	int nEndY = dy + m_nRange;

	CMapCellInfo* pInfo = nullptr;

	if (nStartX < 0)nStartX = 0;
	if (nEndX > mw)nEndX = mw;
	if (nEndX < nStartX)return;
	if (nStartY < 0)nStartY = 0;
	if (nEndY > mh)nEndY = mh;
	if (nEndY < nStartY)return;

	for (int x = nStartX; x <= nEndX; x++)
	{
		for (int y = nStartY; y <= nEndY; y++)
		{
			pInfo = pMap->GetMapCellInfo(x, y);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					if (pNode->getObject() && pNode->getObject()->GetClassType() == CLS_ALIVEOBJECT)
					{
						CAliveObject* pAObj = (CAliveObject*)pNode->getObject();
						if (pAObj != m_pOwner && !pAObj->IsDeath() && m_pOwner->IsProperTarget(pAObj))
						{
							UINT nDamage = Getrand(m_nDamage);
							pAObj->AddProcess(EP_BEATTACKED, nDamage, m_pOwner == nullptr ? 0 : m_pOwner->GetId(), DT_DIRECT);
						}
					}
					pNode = pNode->getNext();
				}
			}
		}
	}
}

VOID FireRainEvent::OnClose(CVisibleEvent* pEvent)
{
	if (pEvent == m_pVisibleEvent)
	{
		m_pVisibleEvent = nullptr;
		Destroy();
	}
}