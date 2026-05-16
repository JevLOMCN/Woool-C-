#include "StdAfx.h"
#include ".\downitemobject.h"
#include "ItemManager.h"
#include "downitemmgr.h"
#include "gameworld.h"
#include "logicmap.h"
#include "aliveobject.h"
#include "systemscript.h"
#include "autoscriptmanager.h"
#include "humanplayer.h"

CDownItemObject::CDownItemObject(void)
{
	Clean();
}

CDownItemObject::~CDownItemObject(void)
{
}

VOID CDownItemObject::Clean()
{
	m_Id = 0;
	this->m_pClass = nullptr;
	memset(&m_Item, 0, sizeof(ITEM));
	SetActionObject(nullptr);
	m_nScriptTimes = 0;
}

BOOL CDownItemObject::GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	int tempSize = 0;
	SmartEncodeMessage(pszMsg, tempSize, m_Item.dwMakeIndex, SM_DOWNITEMAPPEAR,
		m_wX, m_wY, m_Item.baseitem.wImageIndex,
		m_Item.baseitem.szName, m_Item.baseitem.btNameLength);    //23.9.6暂时改成14, 后续更新
	length = tempSize;
	return TRUE;
}

BOOL CDownItemObject::GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	int tempSize = 0;
	SmartEncodeMessage(pszMsg, tempSize, m_Item.dwMakeIndex, SM_DOWNITEMDISAPPEAR, m_wX, m_wY, 0, nullptr);
	length = tempSize;
	return TRUE;
}

VOID CDownItemObject::SetActionObject(CHumanPlayer* pObject)
{
	this->m_pActionObject = pObject;
	if (pObject)
		this->m_dwActionObjectInstanceKey = pObject->GetInstanceKey();
	else
		this->m_dwActionObjectInstanceKey = 0;
}

BOOL CDownItemObject::UpdateValid()
{
	if (m_pClass && m_pClass->wDropPageId != 0 && m_tTimer.IsTimeOut(m_pClass->dwDropPageDelay * m_nScriptTimes * 1000))
	{
		if (this->m_pActionObject && this->m_pActionObject->GetInstanceKey() != this->m_dwActionObjectInstanceKey)
			this->m_pActionObject = nullptr;

		CHumanPlayer* pExecuter = (m_pActionObject != nullptr ? m_pActionObject : CAutoScriptManager::GetInstance()->GetScriptTarget());
		if (pExecuter)
		{
			const char* pszDropPage = CItemManager::GetInstance()->GetStringFromPool(m_pClass->wDropPageId);
			if (pszDropPage[0] != 0)
			{
				CSystemScript::GetInstance()->Execute(pExecuter->GetScriptTarget(), pszDropPage, FALSE);
				pExecuter->SetParam(0, getX());
				pExecuter->SetParam(1, getY());
				pExecuter->SetParam(2, GetMapId());
				pExecuter->SetParam(3, m_nScriptTimes);
				pExecuter->SetUsingItem(&m_Item);
			}
			pExecuter->SetUsingItem(nullptr);
			pExecuter->ClearParam();
			if (m_Item.dwParam[3] == UR_DELETED)
			{
				CDownItemMgr::GetInstance()->DeleteGroundItem(m_pMap, this);
				return FALSE;
			}
		}
		m_nScriptTimes++;
		if (m_nScriptTimes >= m_pClass->dwDropPageExecuteTimes)
			m_pClass = nullptr;
		else
			return TRUE;
	}
	if (m_tTimer.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_ITEMUPDATETIME)))
	{
		if (this->m_OwnerId == 0)
		{
			//	删除自己
			//CItemManager::GetInstance()->DeleteItem(m_Item.dwMakeIndex);1
			CDownItemMgr::GetInstance()->DeleteGroundItem(m_pMap, this);
			return FALSE;
		}
		else
		{
			this->m_OwnerId = 0;
			m_tTimer.Savetime();
		}
	}
	return TRUE;
}

VOID CDownItemObject::OnEnterMap(CLogicMap* pMap)
{
	CMapObject::OnEnterMap(pMap);
	int mx = getX(), my = getY();
	for (int x = -12; x <= 12; x++)
	{
		for (int y = -12; y <= 12; y++)
		{
			CMapCellInfo* pInfo = pMap->GetMapCellInfo(mx + x, my + y);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					if (pNode->getObject()->GetClassType() == CLS_ALIVEOBJECT)
					{
						if (((CAliveObject*)pNode->getObject())->GetVisibleObjectFlag() & (1 << GetType()))
							((CAliveObject*)pNode->getObject())->UpdateVisibleObject(this);
					}
					pNode = pNode->getNext();
				}
			}
		}
	}
	m_pClass = CItemManager::GetInstance()->GetItemClass(this->m_Item);
}

VOID CDownItemObject::OnLeaveMap(CLogicMap* pMap)
{
	CMapObject::OnLeaveMap(pMap);
	int mx = getX(), my = getY();
	for (int x = -12; x <= 12; x++)
	{
		for (int y = -12; y <= 12; y++)
		{
			CMapCellInfo* pInfo = pMap->GetMapCellInfo(mx + x, my + y);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					if (pNode->getObject()->GetClassType() == CLS_ALIVEOBJECT)
					{
						if (((CAliveObject*)pNode->getObject())->GetVisibleObjectFlag() & (1 << GetType()))
							((CAliveObject*)pNode->getObject())->RemoveVisibleObject(this);
					}
					pNode = pNode->getNext();
				}
			}
		}
	}
}

VOID CDownItemObject::OnDroped()
{
	m_tTimer.Savetime();
	UpdateValid();
}