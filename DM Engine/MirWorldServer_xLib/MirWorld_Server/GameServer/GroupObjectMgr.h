#pragma once
class CHumanPlayer;
class CGroupObject;
class CGroupObjectMgr : public xSingletonClass<CGroupObjectMgr>
{
public:
	CGroupObjectMgr(void);
	virtual ~CGroupObjectMgr(void);
	BOOL CreateGroup(CHumanPlayer* pLeader, CHumanPlayer* pFirstMember);
	VOID DestroyGroup(CGroupObject* pObject);
private:
	xObjectPool<CGroupObject> m_xGroupObject;
};