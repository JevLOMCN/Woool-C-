#include "stdafx.h"
#include "ZombiHoleEvent.h"
#include "aliveobject.h"
#include "visibleevent.h"
#include "logicmapmgr.h"
#include "logicmap.h"
#include "eventmanager.h"

xObjectPool<ZombiHoleEvent>	ZombiHoleEvent::m_xPool;
ZombiHoleEvent::ZombiHoleEvent(void)
{
	m_nDamage = 0;
	m_pOwner = nullptr;
	m_dwOwnerInstanceKey = 0;
	m_pVisibleEvent = nullptr;
}

ZombiHoleEvent::~ZombiHoleEvent(void)
{
}

ZombiHoleEvent* ZombiHoleEvent::Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime)
{
	if (pOwner == nullptr) return nullptr;
	CLogicMap* pMap = pOwner->GetMap();
	if (pMap == nullptr)return nullptr;
	if (x >= pMap->GetWidth())return nullptr;
	if (y >= pMap->GetHeight())return nullptr;
	ZombiHoleEvent* pEvent = NewEvent();
	if (pEvent == nullptr)return nullptr;
	if (!CEventManager::GetInstance()->CheckNearEvent(pMap, x, y, 1, VE_FIRERAIN))
		pEvent->m_pVisibleEvent = CEventManager::GetInstance()->NewVisibleEvent(pOwner->GetMap(), x, y, VE_ZOMBIHOLE, dwTick, dwLastTime, pEvent);
	if (pEvent->m_pVisibleEvent == nullptr)
	{
		DeleteEvent(pEvent);
		return nullptr;
	}
	pEvent->m_pOwner = pOwner;
	if (pOwner)pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();
	pEvent->m_nDamage = nDamage;
	CEventManager::GetInstance()->AddEventProcessor(pEvent);
	return pEvent;
}

VOID ZombiHoleEvent::Destroy()
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
}

VOID ZombiHoleEvent::Update()
{
	if (m_pOwner && this->m_pOwner->GetInstanceKey() != this->m_dwOwnerInstanceKey)
		Destroy();
	if (this->m_pVisibleEvent)this->m_pVisibleEvent->UpdateValid();
}

VOID ZombiHoleEvent::OnUpdate(CVisibleEvent* pEvent)
{
	CLogicMap* pMap = pEvent->GetMap();
	if (pMap == nullptr)return;
	if (m_pOwner != nullptr)
	{
		if (m_pOwner->GetInstanceKey() != m_dwOwnerInstanceKey)
		{
			m_pOwner = nullptr;
			Destroy();
			return;
		}
	}
}

VOID ZombiHoleEvent::OnClose(CVisibleEvent* pEvent)
{
	if (pEvent == m_pVisibleEvent)
	{
		m_pVisibleEvent = nullptr;
		Destroy();
	}
}