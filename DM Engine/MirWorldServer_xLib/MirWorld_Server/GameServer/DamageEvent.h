#pragma once
#include "eventprocessor.h"

class CAliveObject;
class CDamageEvent : public CEventProcessor
{
public:
	CDamageEvent(void);
	virtual ~CDamageEvent(void);
	CDamageEvent(const CDamageEvent&) = delete;           // ½ûÖ¹¿½±´¹¹Ôì
	CDamageEvent& operator=(const CDamageEvent&) = delete; // ½ûÖ¹¿½±´¸³Öµ
	static CDamageEvent* Create(UINT mapid, int x, int y, UINT nRange, UINT nView, DWORD dwParam1, DWORD dwParam2, UINT nDamage, damage_type damagetype, CAliveObject* pOwner, UINT nKeepTime, UINT nRunTime);
	VOID Destroy();
	VOID Update();
	VOID OnUpdate(CVisibleEvent* pEvent);
	VOID OnClose(CVisibleEvent* pEvent);

protected:
	CAliveObject* m_pOwner;
	DWORD m_dwOwnerInstanceKey;
	UINT m_nDamage;
	UINT m_nRange;
	damage_type m_DamageType;
	CVisibleEvent* m_pVisibleEvent;
	static CDamageEvent* NewEvent() { return m_xPool.newObject(); }
	static VOID DeleteEvent(CDamageEvent* e) { e->Destroy(); m_xPool.deleteObject(e); }
	static xObjectPool<CDamageEvent> m_xPool;
};