#include "StdAfx.h"
#include <vector>
#include ".\eventmanager.h"
#include ".\logicmap.h"
#include "eventprocessor.h"
#include "visibleevent.h"

CEventManager::CEventManager(void) : m_DeleteObjectQueue(20000)
{
	m_pCurUpdateEvent = nullptr;
	m_pCurUpdateProcessor = nullptr;
}

CEventManager::~CEventManager(void)
{
}

CVisibleEvent* CEventManager::NewVisibleEvent(CLogicMap* pMap, int x, int y, DWORD dwView, DWORD dwRunTick, DWORD dwLastTime, CEventProcessor* pProcessor, DWORD dwParam1, DWORD dwParam2)
{
	if (pMap->FindEventObject(x, y, dwView) != nullptr)return nullptr;
	CVisibleEvent* pEvent = m_VisibleEventPool.newObject();
	if (!pEvent->Create(pMap, x, y, dwView, dwRunTick, dwLastTime, pProcessor, dwParam1, dwParam2))
	{
		m_VisibleEventPool.deleteObject(pEvent);
		return nullptr;
	}
	if (pProcessor == nullptr)
	{
		xListHost<CMapObject>::xListNode* pNode = pEvent->GetLinkNode(LNI_WORLD);
		if (pNode) m_xVisibleEventList.addNode(pNode);
	}
	return pEvent;
}

VOID CEventManager::DelVisibleEvent(CVisibleEvent* pEvent)
{
	pEvent->Close();
	pEvent->Clean();
	m_VisibleEventPool.deleteObject(pEvent);
}

VOID CEventManager::PreDelVisibleEvent(CVisibleEvent* pEvent)
{
	if (pEvent->getProcessor() == nullptr)
	{
		xListHost<CMapObject>::xListNode* pNode = pEvent->GetLinkNode(LNI_WORLD);
		if (pNode)
			m_xVisibleEventList.removeNode(pNode);
	}
	pEvent->SetDelTimer();
	if (!m_DeleteObjectQueue.push(pEvent))
		DelVisibleEvent(pEvent);
}

VOID CEventManager::UpdateDeleteObject()
{
	const UINT MAX_PROCESS_PER_TICK = 100;
	UINT processed = 0;
	std::vector<CVisibleEvent*> pendingObjects;
	pendingObjects.reserve(MAX_PROCESS_PER_TICK);
	while (processed < MAX_PROCESS_PER_TICK)
	{
		CVisibleEvent* pEvent = m_DeleteObjectQueue.pop();
		if (pEvent == nullptr)
			break;
		if (pEvent->IsDelTimerTimeOut(3 * 1000))
			DelVisibleEvent(pEvent);
		else
			pendingObjects.push_back(pEvent);
		processed++;
	}
	for (auto pEvent : pendingObjects)
	{
		if (m_DeleteObjectQueue.getcount() < 18000)
			m_DeleteObjectQueue.push(pEvent);
		else
			DelVisibleEvent(pEvent);
	}
}

VOID CEventManager::UpdateEvents()
{
	xListHost<CMapObject>::xListNode* pCur = m_pCurUpdateEvent;
	xListHost<CMapObject>::xListNode* pNext = nullptr;
	if (pCur == nullptr)pCur = this->m_xVisibleEventList.getHead();
	UINT nCount = 0;
	while (pCur)
	{
		pNext = pCur->getNext();
		((CVisibleEvent*)pCur->getObject())->UpdateValid();
		pCur = pNext;
		++nCount;
		if (nCount >= 100)break;
	}
	m_pCurUpdateEvent = pCur;

	CEventProcessor* pCurProcessor = m_pCurUpdateProcessor;
	CEventProcessor* pNextProcessor = nullptr;
	if (pCurProcessor == nullptr)pCurProcessor = (CEventProcessor*)this->m_xProcessorList.getHead();
	nCount = 0;
	while (pCurProcessor)
	{
		pNextProcessor = (CEventProcessor*)pCurProcessor->getNext();
		pCurProcessor->Update();
		pCurProcessor = pNextProcessor;
		++nCount;
		if (nCount >= 100)break;
	}
	m_pCurUpdateProcessor = pCurProcessor;
}

VOID CEventManager::AddEventProcessor(CEventProcessor* pProcessor)
{
	this->m_xProcessorList.addNode(pProcessor);
}

VOID CEventManager::RemoveEventProcessor(CEventProcessor* pProcessor)
{
	this->m_xProcessorList.removeNode(pProcessor);
}

BOOL CEventManager::CheckNearEvent(CLogicMap* pMap, int x, int y, int nRange, DWORD dwView)
{
	if (pMap == nullptr) return FALSE;
	int nStartX = x - nRange;
	int nStartY = y - nRange;
	int nEndX = x + nRange;
	int nEndY = y + nRange;
	// 检查坐标边界
	int mw = pMap->GetWidth() - 1;
	int mh = pMap->GetHeight() - 1;
	if (nStartX < 0) nStartX = 0;
	if (nEndX > mw) nEndX = mw;
	if (nStartY < 0) nStartY = 0;
	if (nEndY > mh) nEndY = mh;
	for (int cx = nStartX; cx <= nEndX; cx++)
	{
		for (int cy = nStartY; cy <= nEndY; cy++)
		{
			CMapObject* pEventObj = pMap->FindEventObject(cx, cy, dwView);
			if (pEventObj != nullptr) return TRUE;  // 找到事件
		}
	}
	return FALSE;  // 未找到事件
}