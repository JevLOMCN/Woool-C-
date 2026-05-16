#include "StdAfx.h"
#include ".\groupobjectmgr.h"
#include "humanplayer.h"
#include "humanplayermgr.h"
#include "groupobject.h"

CGroupObjectMgr::CGroupObjectMgr(void)
{
}

CGroupObjectMgr::~CGroupObjectMgr(void)
{
}

BOOL CGroupObjectMgr::CreateGroup(CHumanPlayer* pLeader, CHumanPlayer* pFirstMember)
{
	if (pLeader->GetGroupObject() != nullptr || pFirstMember->GetGroupObject() != nullptr)
		return FALSE;
	CGroupObject* pObj = m_xGroupObject.newObject();
	if (pObj == nullptr)return FALSE;
	if (pObj->Create(pLeader, pFirstMember))
	{
		pObj->SendMsg(nullptr, 0, SM_GROUPCREATE, 0, 0, 0);
		return TRUE;
	}
	return FALSE;
}

VOID CGroupObjectMgr::DestroyGroup(CGroupObject* pObject)
{
	m_xGroupObject.deleteObject(pObject);
}