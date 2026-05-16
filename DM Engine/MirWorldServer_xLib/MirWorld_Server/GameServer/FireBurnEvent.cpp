#include "StdAfx.h"
#include ".\fireburnevent.h"
#include "aliveobject.h"
#include "visibleevent.h"
#include ".\logicmapmgr.h"
#include "logicmap.h"
#include "eventmanager.h"
//CFireBurnEvent g_FireBurnEvent;
xObjectPool<CFireBurnEvent>	CFireBurnEvent::m_xEventPool;
CFireBurnEvent::CFireBurnEvent(void)
{
	m_nDamage = 0;
	m_pOwner = nullptr;
	m_dwOwnerInstanceKey = 0;
	memset(m_pEvents, 0, sizeof(m_pEvents));
}

CFireBurnEvent::~CFireBurnEvent(void)
{
}

VOID CFireBurnEvent::OnUpdate(CVisibleEvent* pEvent)
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
	CMapCellInfo* pInfo = pEvent->GetMap()->GetMapCellInfo(pEvent->getX(), pEvent->getY());
	if (pInfo)
	{
		xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
		CMapObject* pObject = nullptr;
		while (pNode)
		{
			pObject = pNode->getObject();
			if (pObject && pObject->GetClassType() == CLS_ALIVEOBJECT && pObject != m_pOwner && !((CAliveObject*)pObject)->IsDeath() && m_pOwner->IsProperTarget((CAliveObject*)pObject))
			{
				UINT nDamage = Getrand(m_nDamage);
				((CAliveObject*)pObject)->AddProcess(EP_BEATTACKED, nDamage, m_pOwner == nullptr ? 0 : m_pOwner->GetId(), DT_DIRECT);
			}
			pNode = pNode->getNext();
		}
	}
}

VOID CFireBurnEvent::OnClose(CVisibleEvent* pEvent)
{
	int	count = 0;
	for (int i = 0; i < 5; i++)
	{
		if (m_pEvents[i] != nullptr)
		{
			if (m_pEvents[i] == pEvent)
				m_pEvents[i] = nullptr;
			else
				count++;
		}
	}
	if (count == 0)
		Destroy();
}

CFireBurnEvent* CFireBurnEvent::Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime)
{
	if (pOwner == nullptr) return nullptr;
	CLogicMap* pMap = pOwner->GetMap();
	if (pMap == nullptr) return nullptr;;
	CFireBurnEvent* pEvent = NewObject();
	if (pEvent == nullptr)return nullptr;
	pEvent->m_nDamage = nDamage;
	pEvent->m_pOwner = pOwner;
	pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();

	memset(pEvent->m_pEvents, 0, sizeof(pEvent->m_pEvents));
	static const POINT	ptFire[5] = {
		{ 0, 0 },//中
		{ -1, 0 }, { 1, 0 },//左、右
		{ 0, -1 }, { 0, 1 },//上、下
	};
	int successcount = 0;
	for (int i = 0; i < 5; i++)
	{
		int targetX = x + ptFire[i].x;
		int targetY = y + ptFire[i].y;
		// 检查边界
		if (targetX < 0 || targetX >= pMap->GetWidth() || targetY < 0 || targetY >= pMap->GetHeight()) continue;
		pEvent->m_pEvents[i] = CEventManager::GetInstance()->NewVisibleEvent(pOwner->GetMap(), targetX, targetY, VE_FIREWALL, dwTick, dwLastTime, pEvent);
		if (pEvent->m_pEvents[i] != nullptr)
			successcount++;
	}
	if (successcount == 0)
	{
		RemoveObject(pEvent);
		pEvent = nullptr;
	}
	else
		CEventManager::GetInstance()->AddEventProcessor(pEvent);
	return pEvent;
}

VOID CFireBurnEvent::Destroy()
{
	for (int i = 0; i < 5; i++)
	{
		if (m_pEvents[i])
		{
			m_pEvents[i]->Close();
			m_pEvents[i] = nullptr;
		}
	}
	m_nDamage = 0;
	m_pOwner = nullptr;
	CEventManager::GetInstance()->RemoveEventProcessor(this);
}

VOID CFireBurnEvent::Update()
{
	for (int i = 0; i < 5; i++)
	{
		if (m_pEvents[i]) m_pEvents[i]->UpdateValid();
	}
}
