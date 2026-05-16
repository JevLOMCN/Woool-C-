#pragma once
#include "eventprocessor.h"

class CAliveObject;
class FireRainEvent :
	public CEventProcessor
{
public:
	FireRainEvent(void);
	virtual ~FireRainEvent(void);
	FireRainEvent(const FireRainEvent&) = delete;           // ½ûÖ¹¿½±´¹¹Ôì
	FireRainEvent& operator=(const FireRainEvent&) = delete; // ½ûÖ¹¿½±´¸³Öµ
	static FireRainEvent* Create(CAliveObject* pOwner, int x, int y, UINT nRange, UINT nDamage, UINT nKeepTime, UINT nRunTime);
	VOID Destroy();
	VOID Update();
	VOID OnUpdate(CVisibleEvent* pEvent);
	VOID OnClose(CVisibleEvent* pEvent);
protected:
	CAliveObject* m_pOwner;
	DWORD m_dwOwnerInstanceKey;
	UINT m_nDamage;
	UINT m_nRange;
	CVisibleEvent* m_pVisibleEvent;
	static FireRainEvent* NewEvent() { return m_xPool.newObject(); }
	static VOID DeleteEvent(FireRainEvent* e) { e->Destroy(); m_xPool.deleteObject(e); }
	static xObjectPool<FireRainEvent> m_xPool;
};