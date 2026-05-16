#include "StdAfx.h"
#include ".\damageevent.h"
#include ".\visibleevent.h"
#include ".\aliveobject.h"
#include ".\logicmap.h"
#include ".\logicmapmgr.h"
#include ".\eventmanager.h"

xObjectPool<CDamageEvent> CDamageEvent::m_xPool;
CDamageEvent::CDamageEvent(void)
{
	m_pOwner = nullptr;
	m_pVisibleEvent = nullptr;
	m_dwOwnerInstanceKey = 0;
	m_nDamage = 0;
	m_nRange = 0;
	m_DamageType = DT_PHYSICS;
}

CDamageEvent::~CDamageEvent(void)
{
}

CDamageEvent* CDamageEvent::Create(UINT mapid, int x, int y, UINT nRange, UINT nView, DWORD dwParam1, DWORD dwParam2, UINT nDamage, damage_type damagetype, CAliveObject* pOwner, UINT nKeepTime, UINT nRunTime)
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
	if (pMap == nullptr)return nullptr;
	if (x >= pMap->GetWidth())return nullptr;
	if (y >= pMap->GetHeight())return nullptr;
	CDamageEvent* pEvent = NewEvent();
	if (pEvent == nullptr)return nullptr;
	pEvent->m_pVisibleEvent = CEventManager::GetInstance()->NewVisibleEvent(pMap, x, y, nView, nRunTime, nKeepTime, pEvent, dwParam1, dwParam2);
	if (pEvent->m_pVisibleEvent == nullptr)
	{
		DeleteEvent(pEvent);
		return nullptr;
	}
	pEvent->m_pOwner = pOwner;
	if (pOwner)pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();
	pEvent->m_nDamage = nDamage;
	pEvent->m_DamageType = damagetype;
	pEvent->m_nRange = nRange;
	CEventManager::GetInstance()->AddEventProcessor(pEvent);
	return pEvent;
}

VOID CDamageEvent::Update()
{
	if (m_pOwner && this->m_pOwner->GetInstanceKey() != this->m_dwOwnerInstanceKey)
		Destroy();
	if (this->m_pVisibleEvent)this->m_pVisibleEvent->UpdateValid();
}

VOID CDamageEvent::Destroy()
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

VOID CDamageEvent::OnUpdate(CVisibleEvent* pEvent)
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
		//if( x < 0 || x > mw )continue;
		for (int y = nStartY; y <= nEndY; y++)
		{
			//if( y < 0 || y > mh )continue;
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
							pAObj->AddProcess(EP_BEATTACKED, m_nDamage, m_pOwner == nullptr ? 0 : m_pOwner->GetId(), m_DamageType);
						}
					}
					pNode = pNode->getNext();
				}
			}
		}
	}
}

VOID CDamageEvent::OnClose(CVisibleEvent* pEvent)
{
	if (pEvent == m_pVisibleEvent)
	{
		m_pVisibleEvent = nullptr;
		Destroy();
	}
}
