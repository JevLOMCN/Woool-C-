#include "StdAfx.h"
#include ".\changemapevent.h"
#include "logicmapmgr.h"
#include "aliveobject.h"
#include ".\humanplayer.h"

xObjectPool<CChangeMapEvent> CChangeMapEvent::m_xObjectPool;
CChangeMapEvent::CChangeMapEvent(void)
{
	Clean();
}

CChangeMapEvent::~CChangeMapEvent(void)
{
}

VOID CChangeMapEvent::Clean()
{
	CMapObject::Clean();
	m_pToMap = nullptr;
	m_nToX = 0;
	m_nToY = 0;
	m_nMapId = 0;
}

CChangeMapEvent* CChangeMapEvent::Create(int mapid, int x, int y, int tomapid, int tox, int toy)
{
	CLogicMap* pToMap = CLogicMapMgr::GetInstance()->GetLogicMapById(tomapid);
	CLogicMap* pFromMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
	if (pToMap == nullptr || pFromMap == nullptr)
		return nullptr;
	CChangeMapEvent* pEvent = newObject();
	if (pEvent)
	{
		if (pEvent->Init(mapid, x, y, tomapid, tox, toy))
			return pEvent;
		deleteObject(pEvent);
		pEvent = nullptr;
	}
	return pEvent;
}

BOOL CChangeMapEvent::Init(int mapid, int x, int y, int tomapid, int tox, int toy)
{
	CLogicMap* pToMap = CLogicMapMgr::GetInstance()->GetLogicMapById(tomapid);
	CLogicMap* pFromMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
	if (pToMap == nullptr || pFromMap == nullptr)
		return FALSE;
	Clean();
	setXY(x, y);
	m_nToX = tox;
	m_nToY = toy;
	m_pToMap = pToMap;
	m_nMapId = tomapid;
	return pFromMap->AddObject(this);
}

VOID CChangeMapEvent::OnEnter(CMapObject* pObject)
{
	if (pObject->GetType() == OBJ_PLAYER)
	{
		CHumanPlayer* pPlayer = ((CHumanPlayer*)pObject);
		if (pPlayer->getX() == getX() && pPlayer->getY() == getY())
		{
			// 只有走路或跑步到达的玩家才触发传送
			actiontype preAction = pPlayer->GetPreActionType();
			if (preAction == AT_WALK || preAction == AT_RUN)
				pPlayer->AddProcess(EP_CHANGEMAP, m_nMapId, m_nToX, m_nToY, 0, 1);
		}
	}
}

VOID CChangeMapEvent::OnLeave(CMapObject* pObject)
{

}

VOID CChangeMapEvent::OnEnterMap(CLogicMap* pMap)
{
	SetEnterFlag(pMap);
	CMapObject::OnEnterMap(pMap);
}

CChangeMapEvent* CChangeMapEvent::newObject()
{
	return m_xObjectPool.newObject();
}

VOID CChangeMapEvent::deleteObject(CChangeMapEvent* pEvent)
{
	if (pEvent == nullptr)return;
	pEvent->Clean();
	m_xObjectPool.deleteObject(pEvent);
}