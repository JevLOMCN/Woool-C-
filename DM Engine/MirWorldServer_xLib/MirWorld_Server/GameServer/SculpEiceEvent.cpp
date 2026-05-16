#include "stdafx.h"
#include "SculpEiceEvent.h"
#include "aliveobject.h"
#include "visibleevent.h"
#include "logicmapmgr.h"
#include "logicmap.h"
#include "eventmanager.h"

xObjectPool<SculpEiceEvent>	SculpEiceEvent::m_xEventPool;
SculpEiceEvent::SculpEiceEvent(void)
{
	m_nDamage = 0;
	m_pOwner = nullptr;
	m_pEvent = nullptr;
	m_dwOwnerInstanceKey = 0;
}

SculpEiceEvent::~SculpEiceEvent(void)
{
	Destroy();
}

SculpEiceEvent* SculpEiceEvent::Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime)
{
	if (pOwner == nullptr) return nullptr;
	CLogicMap* pMap = pOwner->GetMap();
	if (pMap == nullptr) return nullptr;
	if (x >= pMap->GetWidth())return nullptr;
	if (y >= pMap->GetHeight())return nullptr;
	SculpEiceEvent* pEvent = NewObject();
	if (pEvent == nullptr)return nullptr;

	pEvent->m_pEvent = CEventManager::GetInstance()->NewVisibleEvent(pOwner->GetMap(), x, y, VE_SCULPEICE, dwTick, dwLastTime, pEvent, 2);
	if (pEvent->m_pEvent == nullptr)
	{
		DeleteEvent(pEvent);
		return nullptr;
	}
	pEvent->m_nDamage = nDamage;
	pEvent->m_pOwner = pOwner;
	pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();
	CEventManager::GetInstance()->AddEventProcessor(pEvent);
	return pEvent;
}

VOID SculpEiceEvent::Update()
{
	if (m_pEvent) m_pEvent->UpdateValid();
}

VOID SculpEiceEvent::Destroy()
{
	CEventManager::GetInstance()->RemoveEventProcessor(this);
	if (m_pEvent)
	{
		m_pEvent->Close();
		m_pEvent = nullptr;
	}
	m_nDamage = 0;
	m_pOwner = nullptr;
}

VOID SculpEiceEvent::OnUpdate(CVisibleEvent* pEvent)
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

VOID SculpEiceEvent::OnClose(CVisibleEvent* pEvent)
{
	if (m_pEvent == pEvent)
	{
		m_pEvent = nullptr;
		Destroy();
	}
}