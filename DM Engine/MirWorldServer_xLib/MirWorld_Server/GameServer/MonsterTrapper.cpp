#include "StdAfx.h"
#include ".\monstertrapper.h"
#include "logicmap.h"
#include "humanplayer.h"
#include "monsterex.h"
#include "eventmanager.h"
#include "visibleevent.h"

CTrapperEvent::CTrapperEvent(CMonsterTrapper* pTrapper)
{
	this->m_pTrapper = pTrapper;
}

CTrapperEvent::~CTrapperEvent()
{
}

VOID CTrapperEvent::Clean()
{
}

VOID CTrapperEvent::OnEnter(CMapObject* pObject)
{
	if (pObject->GetType() == OBJ_PLAYER)
		if (m_pTrapper)m_pTrapper->Destroy();
}

VOID CTrapperEvent::EnterMap(CLogicMap* pMap, UINT x, UINT y)
{
	setXY(x, y);
	SetMapId(pMap->GetIndex());
	if (pMap->AddObject(this))
		SetEnterFlag(pMap);
}

VOID CTrapperEvent::LeaveMap()
{
	if (m_pMap)
		m_pMap->RemoveObject(this);
}

VOID CTrapperEvent::OnLeaveMap(CLogicMap* pMap)
{
	CMapCellInfo* pInfo = pMap->GetMapCellInfo(getX(), getY());
	if (pInfo)
	{
		xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
		while (pNode)
		{
			if (pNode->getObject() && pNode->getObject()->GetType() == OBJ_MONSTER)
			{
				CMonsterEx* pMonster = (CMonsterEx*)pNode->getObject();
				if (pMonster->IsDeath() ||
					pMonster->GetPropValue(PI_LEVEL) > this->m_pTrapper->GetDamage())
				{
					pNode = pNode->getNext();
					continue;
				}
				if (pMonster->IsSystemFlagSeted(SF_SLEEP) &&
					pMonster->GetSystemFlagParam(SF_SLEEP) == static_cast<DWORD>(reinterpret_cast<uintptr_t>(this)))
					pMonster->SetSystemFlag(SF_SLEEP, FALSE);
			}
			pNode = pNode->getNext();
		}
	}
	CEventObject::OnLeaveMap(pMap);
}

VOID CTrapperEvent::OnEnterMap(CLogicMap* pMap)
{
	CMapObject::OnEnterMap(pMap);
	CMapCellInfo* pInfo = pMap->GetMapCellInfo(getX(), getY());
	if (pInfo)
	{
		xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
		while (pNode)
		{
			if (pNode->getObject() && pNode->getObject()->GetType() == OBJ_MONSTER)
			{
				CMonsterEx* pMonster = (CMonsterEx*)pNode->getObject();
				if (pMonster->IsDeath() ||
					pMonster->GetPropValue(PI_LEVEL) > this->m_pTrapper->GetDamage())
				{
					pNode = pNode->getNext();
					continue;
				}
				this->m_pTrapper->AddTrappedCount();
				pMonster->SetSystemFlag(SF_SLEEP, TRUE, (DWORD)this, m_pTrapper->GetLastTime());
			}

			if (pNode->getObject() && pNode->getObject()->GetType() == OBJ_PLAYER)
				this->m_pTrapper->SetFailed();
			pNode = pNode->getNext();
		}
	}
}


xObjectPool<CMonsterTrapper> CMonsterTrapper::m_xEventPool;
CMonsterTrapper::CMonsterTrapper(void):
	m_dwMonsterCount(0),
	m_bFailed(FALSE),
	m_nDamage(0),
	m_dwLastTime(0),
	m_dwOwnerInstanceKey(0),
	m_pOwner(nullptr)
{
	memset(m_pEvents, 0, sizeof(m_pEvents));
	for (int i = 0; i < 9; i++)
	{
		m_pTrapped[i] = new CTrapperEvent(this);
	}
}

CMonsterTrapper::~CMonsterTrapper(void)
{
}

VOID CMonsterTrapper::OnUpdate(CVisibleEvent* pEvent)
{
}

VOID CMonsterTrapper::OnClose(CVisibleEvent* pEvent)
{
	int	count = 0;
	for (int i = 0; i < 8; i++)
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

static const POINT ptVE[8] =
{
	{ -1, -2}, { -2, -1}, { -2, 1}, { -1, 2 },
	{ 2, -1}, { 1, -2 }, { 2, 1}, { 1, 2 }
};

static const POINT ptEE[9] =
{
	{0, 0 }, {1, 0}, {1,1 }, {1, -1},
	{ 0, 1}, { 0, -1}, { -1, 1 }, { -1, 0 }, { -1, -1 }
};

CMonsterTrapper* CMonsterTrapper::Create(CHumanPlayer* pOwner, int x, int y, int nDamage, DWORD dwColor, DWORD dwLastTime)
{
	CMonsterTrapper* pEvent = NewObject();
	if (pEvent == nullptr)return nullptr;

	pEvent->m_nDamage = nDamage;
	pEvent->m_pOwner = pOwner;
	pEvent->m_dwLastTime = dwLastTime;
	pEvent->m_bFailed = FALSE;
	pEvent->m_dwMonsterCount = 0;
	if (pOwner != nullptr)
		pEvent->m_dwOwnerInstanceKey = pOwner->GetInstanceKey();

	for (int i = 0; i < 9; i++)
	{
		if (pEvent->m_pTrapped[i])
			pEvent->m_pTrapped[i]->EnterMap(pOwner->GetMap(), x + ptEE[i].x, y + ptEE[i].y);
	}
	if (pEvent->m_bFailed || pEvent->GetTrappedCount() == 0)
	{
		pEvent->Destroy();
		return nullptr;
	}

	memset(pEvent->m_pEvents, 0, sizeof(pEvent->m_pEvents));
	int successcount = 0;
	for (int i = 0; i < 8; i++)
	{
		pEvent->m_pEvents[i] = CEventManager::GetInstance()->NewVisibleEvent(pOwner->GetMap(), x + ptVE[i].x, y + ptVE[i].y, VE_SOULWALL, dwLastTime, dwLastTime, pEvent, 0, dwColor);
		if (pEvent->m_pEvents[i] != nullptr)
			successcount++;
	}
	if (successcount == 0)
	{
		pEvent->Destroy();
		return nullptr;
	}
	CEventManager::GetInstance()->AddEventProcessor(pEvent);
	return pEvent;
}

VOID CMonsterTrapper::Destroy()
{
	for (int i = 0; i < 8; i++)
	{
		if (m_pEvents[i])
		{
			m_pEvents[i]->Close();
			m_pEvents[i] = nullptr;
		}
	}
	for (int i = 0; i < 9; i++)
	{
		if (this->m_pTrapped[i])
			this->m_pTrapped[i]->LeaveMap();
	}
	m_nDamage = 0;
	m_pOwner = nullptr;
	//	delete self
	CEventManager::GetInstance()->RemoveEventProcessor(this);
	RemoveObject(this);
}

VOID CMonsterTrapper::Update()
{
	for (int i = 0; i < 8; i++)
	{
		if (m_pEvents[i])
			m_pEvents[i]->UpdateValid();
	}
}