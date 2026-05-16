#include "StdAfx.h"
#include "bluefireevent.h"
#include "aliveobject.h"
#include "visibleevent.h"
#include "logicmapmgr.h"
#include "logicmap.h"
#include "eventmanager.h"

xObjectPool<CBlueFireEvent>	CBlueFireEvent::m_xEventPool;
CBlueFireEvent::CBlueFireEvent(void)
{
	m_nDamage = 0;
	m_pOwner = nullptr;
	m_dwOwnerInstanceKey = 0;
	memset(m_pEvents, 0, sizeof(m_pEvents));
}

CBlueFireEvent::~CBlueFireEvent(void)
{
}

VOID CBlueFireEvent::OnUpdate(CVisibleEvent* pEvent)
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

VOID CBlueFireEvent::OnClose(CVisibleEvent* pEvent)
{
	int	count = 0;
	for (int i = 0; i < BLUEFIRE_EVENT_NUM; i++)
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

CBlueFireEvent* CBlueFireEvent::Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime)
{
	if (pOwner == nullptr) return nullptr;
	CLogicMap* pMap = pOwner->GetMap();
	if (pMap == nullptr) return nullptr;;
	CBlueFireEvent* pEvent = NewObject();
	if (pEvent == nullptr)return nullptr;
	pEvent->m_nDamage = nDamage;
	pEvent->m_pOwner = pOwner;
	pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();

	memset(pEvent->m_pEvents, 0, sizeof(pEvent->m_pEvents));
	static const POINT ptFire[BLUEFIRE_EVENT_NUM] = {
	{ -2, -2 }, { -1, -2 }, { 0, -2 }, { 1, -2 }, { 2, -2 },  // 第一行
	{ -2, -1 }, { -1, -1 }, { 0, -1 }, { 1, -1 }, { 2, -1 },  // 第二行
	{ -2,  0 }, { -1,  0 }, { 0,  0 }, { 1,  0 }, { 2,  0 },  // 第三行（中心在0,0）
	{ -2,  1 }, { -1,  1 }, { 0,  1 }, { 1,  1 }, { 2,  1 },  // 第四行
	{ -2,  2 }, { -1,  2 }, { 0,  2 }, { 1,  2 }, { 2,  2 }   // 第五行
	};
	int successcount = 0;
	for (int i = 0; i < BLUEFIRE_EVENT_NUM; i++)
	{
		int targetX = x + ptFire[i].x;
		int targetY = y + ptFire[i].y;
		// 检查边界
		if (targetX < 0 || targetX >= pMap->GetWidth() || targetY < 0 || targetY >= pMap->GetHeight()) continue;
		pEvent->m_pEvents[i] = CEventManager::GetInstance()->NewVisibleEvent(pOwner->GetMap(), targetX, targetY, VE_BLUEFIRE, dwTick, dwLastTime, pEvent);
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

VOID CBlueFireEvent::Destroy()
{
	for (int i = 0; i < BLUEFIRE_EVENT_NUM; i++)
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

VOID CBlueFireEvent::Update()
{
	for (int i = 0; i < BLUEFIRE_EVENT_NUM; i++)
	{
		if (m_pEvents[i]) m_pEvents[i]->UpdateValid();
	}
}