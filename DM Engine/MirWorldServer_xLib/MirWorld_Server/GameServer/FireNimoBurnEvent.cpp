#include "StdAfx.h"
#include ".\FireNimoBurnEvent.h"
#include "aliveobject.h"
#include "visibleevent.h"
#include ".\logicmapmgr.h"
#include "logicmap.h"
#include "eventmanager.h"

xObjectPool<CFireNimoBurnEvent>	CFireNimoBurnEvent::m_xEventPool;
CFireNimoBurnEvent::CFireNimoBurnEvent(void)
{
	m_nDamage = 0;
	m_pOwner = nullptr;
	m_dwOwnerInstanceKey = 0;
	memset(m_pEvents, 0, sizeof(m_pEvents));
}

CFireNimoBurnEvent::~CFireNimoBurnEvent(void)
{
}

VOID CFireNimoBurnEvent::Destroy()
{
	for (int i = 0; i < MAX_FIRENIMOEVENT_COUNT; i++)
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

VOID CFireNimoBurnEvent::Update()
{
	for (int i = 0; i < MAX_FIRENIMOEVENT_COUNT; i++)
	{
		if (m_pEvents[i]) m_pEvents[i]->UpdateValid();
	}
}

VOID CFireNimoBurnEvent::OnUpdate(CVisibleEvent* pEvent)
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
				((CAliveObject*)pObject)->AddProcess(EP_BEATTACKED, m_nDamage, m_pOwner == nullptr ? 0 : m_pOwner->GetId(), DT_MAGIC);
			}
			pNode = pNode->getNext();
		}
	}
}

VOID CFireNimoBurnEvent::OnClose(CVisibleEvent* pEvent)
{
	int	count = 0;
	for (int i = 0;i < MAX_FIRENIMOEVENT_COUNT;i++)
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

CFireNimoBurnEvent* CFireNimoBurnEvent::Create(CAliveObject* pOwner, int x, int y, int type, int nDamage, DWORD dwTick, DWORD dwLastTime)
{
	if (pOwner == nullptr) return nullptr;
	CLogicMap* pMap = pOwner->GetMap();
	if (pMap == nullptr) return nullptr;;
	CFireNimoBurnEvent* pEvent = NewObject();
	if (pEvent == nullptr)return nullptr;
	pEvent->m_nDamage = nDamage;
	pEvent->m_pOwner = pOwner;
	pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();

	static const POINT ptFire[MAX_FIRENIMOEVENT_COUNT] = {
		{ 0, 0 },
		{ 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 },
		{ 2, 0 }, { 1, 1 }, { 0, 2 }, { -1, 1 }, { -2, 0 }, { -1, -1 }, { 0, -2 }, { 1, -1 }
	};
	static const int dir1[3] = { -1, 1, 3 };
	int dir2[3] = { 0 };
	GenerateRandomNumbers(-2, 2, 3, dir2);

	int dir = pOwner->GetDirection();
	int successcount = 0;
	int forNum = 3;
	if (type == 4)forNum = MAX_FIRENIMOEVENT_COUNT;
	for (int i = 0;i < forNum;i++)
	{
		int targetX = 0;
		int targetY = 0;
		if (type == 1) //朝向直线3个
		{ 
			GETNEXTPOS(x, y, dir);
			targetX = x;
			targetY = y;
		}
		if (type == 2) //朝向身边3个
		{ 
			GETNEXTPOS(x, y, dir + dir1[i]);
			targetX = x;
			targetY = y;
		}
		if (type == 3) //随机
		{ 
			GETNEXTPOS(x, y, dir + dir2[i]);
			targetX = x;
			targetY = y;
		}
		if (type == 4) //大火
		{
			targetX = x + ptFire[i].x;
			targetY = y + ptFire[i].y;
		}
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