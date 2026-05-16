#include "StdAfx.h"
#include <vector>
#include ".\downitemmgr.h"
#include "logicmap.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "systemscript.h"

CDownItemMgr::CDownItemMgr(void) : m_xDelItemQueue(200000)
{
	m_xDownItemList.create(400000);
	m_nCurFreeIndex = 0;
}

CDownItemMgr::~CDownItemMgr(void)
{
}

CDownItemObject* CDownItemMgr::newObject()
{
	CDownItemObject* p = m_xDownItemPool.newObject();
	if (p == nullptr)return p;
	UINT id = m_xDownItemList.addObject(p);
	if (id == 0)
	{
		m_xDownItemPool.deleteObject(p);
		return nullptr;
	}
	p->SetId(id);
	return p;
}

VOID CDownItemMgr::deleteObject(CDownItemObject* p)
{
	UINT id = p->GetId();
	p->Clean();
	m_xDownItemList.delObject(id);
	m_xDownItemPool.deleteObject(p);
}

CDownItemObject* CDownItemMgr::NewDownItemObject(ITEM& item, WORD x, WORD y, DWORD dwOwner)
{
	CDownItemObject* pDownItem = newObject();
	if (pDownItem == nullptr)return nullptr;
	//pDownItem->Clean();
	//pDownItem->SetId( id );
	pDownItem->SetItem(item);
	pDownItem->setXY(x, y);
	pDownItem->SetOwnerId(dwOwner);
	return pDownItem;
}

BOOL CDownItemMgr::DeleteDownItemObject(CDownItemObject* pObject)
{
	if (pObject == nullptr)return FALSE;
	pObject->SetDelTimer();
	if (!m_xDelItemQueue.push(pObject))
		deleteObject(pObject);
	return TRUE;
}

BOOL CDownItemMgr::DeleteDownItemObjectImm(CDownItemObject* pObject)
{
	deleteObject(pObject);
	return TRUE;
}

BOOL CDownItemMgr::DropItem(CLogicMap* pMap, ITEM& item, WORD x, WORD y, BOOL boOwner, CHumanPlayer* pActionObject)
{
	if (item.baseitem.btStdMode == 255)
	{
		//附身物品, 丢地上时, 要改变外观.
		switch (item.baseitem.btShape)
		{
		case 1: // 秒杀
		case 2: // 重击
		case 3: // 神御
		case 4: // 神佑
		{
			item.baseitem.wImageIndex = 497;
		}
		break;
		}
		item.dwMakeIndex |= 0x80000000;//标识物品为新物品
	}
	DWORD dwOwner = 0;
	if (boOwner && pActionObject)
		dwOwner = pActionObject->GetDBId();
	CDownItemObject* pDownItem = NewDownItemObject(item, x, y, dwOwner);
	if (pDownItem == nullptr)return FALSE;
	if (!pMap->AddObject(pDownItem))
	{
		DeleteDownItemObjectImm(pDownItem);
		return FALSE;
	}
	pDownItem->SetActionObject(pActionObject);
	pDownItem->OnDroped();
	CItemManager::GetInstance()->UpdateItemOwner(0, item.dwMakeIndex, IDF_GROUND, 0);
	return TRUE;
}

BOOL CDownItemMgr::DropItem(CLogicMap* pMap, ITEM& item, WORD x, WORD y, DWORD dwOwner)
{
	CDownItemObject* pDownItem = NewDownItemObject(item, x, y, dwOwner);
	if (pDownItem == nullptr)return FALSE;
	if (!pMap->AddObject(pDownItem))
	{
		DeleteDownItemObjectImm(pDownItem);
		return FALSE;
	}
	pDownItem->OnDroped();
	CItemManager::GetInstance()->UpdateItemOwner(0, item.dwMakeIndex, IDF_GROUND, 0);
	return TRUE;
}

BOOL CDownItemMgr::PickupItem(CLogicMap* pMap, CDownItemObject* pObject, CHumanPlayer* pActionObject)
{
	if (!pMap->RemoveObject(pObject))
		return FALSE;
	ITEM item = pObject->GetItem();
	ITEMCLASS* pClass = CItemManager::GetInstance()->GetItemClass(item);
	BOOL bFlag = TRUE;

	BOOL bDelDownItem = FALSE;
	UINT x = pObject->getX(), y = pObject->getY();
	if ((item.dwMakeIndex & 0x80000000) && item.baseitem.btStdMode == 255)
	{
		switch (item.baseitem.btShape)
		{
		case 0:	// 金钱
		{
			DWORD dwGold = item.wCurDura | (item.wMaxDura << 16);
			if (!pActionObject->AddGold(dwGold))
			{
				pActionObject->SaySystem(getstrings(SD_STDERROR_CANNOTPICKUPMOREGOLD));
				bFlag = FALSE;
			}
		}
		break;
		case 1: // 秒杀
		case 2: // 重击
		case 3: // 神御
		case 4: // 神佑
		{
			DWORD dwParam = (item.baseitem.btMinDef << 16) | item.baseitem.btShape;
			pActionObject->SetSystemFlag(SF_GODBLESS, TRUE, dwParam, item.baseitem.btSpecialpower * 60000);
		}
		break;
		}

		if (bFlag || pMap->AddObject(pObject) == FALSE)
		{
			CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
			DeleteDownItemObject(pObject);
		}
	}
	else
	{
		if (!pActionObject->AddBagItem(item, FALSE, TRUE))
			bFlag = FALSE;
		if (bFlag)
			DeleteDownItemObject(pObject);
		else
		{
			if (pMap->AddObject(pObject) == FALSE)
			{
				CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
				DeleteDownItemObject(pObject);
			}
		}
	}
	if (bFlag)
	{
		ITEM* p = pActionObject->FindBagItem(item.dwMakeIndex);
		if (p == nullptr)p = &item;
		pActionObject->OnPickupItem(*p, x, y);
		if (pClass && pClass->wPickupPageId != 0)
		{
			const char* pszPickupPage = CItemManager::GetInstance()->GetStringFromPool(pClass->wPickupPageId);
			if (pszPickupPage[0] != 0)
			{
				pActionObject->SetUsingItem(p);
				p->dwParam[3] = UR_NORESULT;
				CSystemScript::GetInstance()->Execute(pActionObject->GetScriptTarget(), pszPickupPage, 0);
			}
			pActionObject->SetUsingItem(nullptr);
			if (p->dwParam[3] == UR_DELETED)
			{
				if (p != &item)
				{
					pActionObject->DeleteBagItem(p->dwMakeIndex);
					CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
					pActionObject->SendTakeBagItem(&item);
				}
			}
			else if (item.dwParam[3] == UR_UPDATED)
			{
				if (p != &item)
					pActionObject->SendUpdateItem(*p);
			}
		}
	}
	return bFlag;
}

BOOL CDownItemMgr::DeleteGroundItem(CLogicMap* pMap, CDownItemObject* pObject, BOOL bDeleteItem)
{
	if (!pMap->RemoveObject(pObject))
		return FALSE;
	if (bDeleteItem)
		CItemManager::GetInstance()->DeleteItem(pObject->GetItem().dwMakeIndex);
	DeleteDownItemObject(pObject);
	return TRUE;
}

VOID CDownItemMgr::UpdateDeletedObject()
{
	const UINT MAX_PROCESS_PER_TICK = 100;
	UINT processed = 0;
	std::vector<CDownItemObject*> pendingObjects;
	pendingObjects.reserve(MAX_PROCESS_PER_TICK);
	while (processed < MAX_PROCESS_PER_TICK)
	{
		CDownItemObject* pObject = m_xDelItemQueue.pop();
		if (pObject == nullptr)
			break;
		if (pObject->IsDelTimerTimeOut(3 * 1000))
			DeleteDownItemObjectImm(pObject);
		else
			pendingObjects.push_back(pObject);
		processed++;
	}
	for (auto pObject : pendingObjects)
	{
		if (m_xDelItemQueue.getcount() < 180000)
			m_xDelItemQueue.push(pObject);
		else
			DeleteDownItemObjectImm(pObject);
	}
}

VOID CDownItemMgr::UpdateDownItem()
{
	CDownItemObject* pMonster = nullptr;
	if (m_xDownItemList.getCount() == 0) return;
	for (UINT i = 0; i < 100; i++)
	{
		if (m_nCurFreeIndex > m_xDownItemList.getCurPtr())
			m_nCurFreeIndex = 0;
		pMonster = m_xDownItemList.getObject(m_nCurFreeIndex);
		if (pMonster != nullptr && pMonster->GetMap() != nullptr)
			pMonster->UpdateValid();
		m_nCurFreeIndex++;
		if (i >= m_xDownItemList.getCount()) return;
	}
}