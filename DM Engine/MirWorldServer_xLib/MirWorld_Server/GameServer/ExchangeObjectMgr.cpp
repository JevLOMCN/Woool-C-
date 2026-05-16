#include "StdAfx.h"
#include ".\exchangeobjectmgr.h"
#include ".\exchangeobj.h"

xObjectPool<CExchangeObj> CExchangeObjectMgr::m_xExchangeObjPool;
CExchangeObjectMgr::CExchangeObjectMgr(void)
{
}

CExchangeObjectMgr::~CExchangeObjectMgr(void)
{
}

BOOL CExchangeObjectMgr::BeginExchange(CHumanPlayer* p1, CHumanPlayer* p2)
{
	CExchangeObj* pObj = newObject();
	if (pObj == nullptr)return FALSE;
	if (pObj->Begin(p1, p2))return TRUE;
	deleteObject(pObj);
	return FALSE;
}

VOID CExchangeObjectMgr::EndExchange(CExchangeObj* pObj)
{
	deleteObject(pObj);
}