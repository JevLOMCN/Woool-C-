#include "StdAfx.h"
#include ".\eventobject.h"
#include "logicmap.h"

CEventObject::CEventObject(void)
{
	m_bDisabled = FALSE;
}

CEventObject::~CEventObject(void)
{
}

VOID CEventObject::Clean()
{
	m_bDisabled = FALSE;
	CMapObject::Clean();
}

VOID CEventObject::SetEnterFlag(CLogicMap* pMap)
{
	CMapCellInfo* pInfo = pMap->GetMapCellInfo(m_wX, m_wY);
	if (pInfo == nullptr)return;
	//	设置事件标记
	pInfo->wEventFlag = pInfo->wEventFlag | EVENTFLAG_ENTEREVENT;
}

VOID CEventObject::SetLeaveFlag(CLogicMap* pMap)
{
	CMapCellInfo* pInfo = pMap->GetMapCellInfo(m_wX, m_wY);
	if (pInfo == nullptr)return;
	//	设置事件标记
	pInfo->wEventFlag = pInfo->wEventFlag | EVENTFLAG_LEAVEEVENT;
}

VOID CEventObject::OnLeaveMap(CLogicMap* pMap)
{
	CMapCellInfo* pInfo = pMap->GetMapCellInfo(m_wX, m_wY);
	if (pInfo)
	{
		xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
		CMapObject* pObject = nullptr;
		int	count = 0;
		if ((pInfo->wEventFlag & EVENTFLAG_ENTEREVENT) != 0 || (pInfo->wEventFlag & EVENTFLAG_LEAVEEVENT) != 0)
		{
			while (pNode)
			{
				pObject = pNode->getObject();
				pNode = pNode->getNext();
				if (pObject && pObject->GetClassType() == CLS_EVENT && pObject != this)
					count++;
			}
			if (count == 0)
			{
				if (pInfo->wEventFlag & EVENTFLAG_ENTEREVENT)
					pInfo->wEventFlag ^= EVENTFLAG_ENTEREVENT;
				if (pInfo->wEventFlag & EVENTFLAG_LEAVEEVENT)
					pInfo->wEventFlag ^= EVENTFLAG_LEAVEEVENT;
			}
		}
	}
}