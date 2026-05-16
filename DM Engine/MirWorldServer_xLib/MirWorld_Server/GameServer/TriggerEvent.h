#pragma once
#include "eventprocessor.h"

class CAliveObject;
// 用于创建一个触发事件
class TriggerEvent : public CEventProcessor
{
public:
	TriggerEvent(void);
	virtual ~TriggerEvent(void);
	TriggerEvent(const TriggerEvent&) = delete;           // 禁止拷贝构造
	TriggerEvent& operator=(const TriggerEvent&) = delete; // 禁止拷贝赋值
	// 静态方法：脚本命令创建事件
	// szMapName: 地图名
	// x,y: 事件位置
	// dwType: 事件类型
	// m_szPage: 触发page
	// nRunTime: 触发间隔
	// nKeepTime: 持续时间
	static TriggerEvent* Create(CAliveObject* pOwner, const char* szMapName, int x, int y, DWORD dwType, const char* m_szPage, UINT nKeepTime, UINT nRunTime);
	VOID Destroy();
	VOID Update();
	VOID OnUpdate(CVisibleEvent* pEvent);
	VOID OnClose(CVisibleEvent* pEvent);
protected:
	CAliveObject* m_pOwner;
	DWORD m_dwOwnerInstanceKey;
	char m_szMapName[64];
	char m_szPage[64];
	CVisibleEvent* m_pVisibleEvent;
	static TriggerEvent* NewEvent() { return m_xPool.newObject(); }
	static VOID DeleteEvent(TriggerEvent* e) { e->Destroy(); m_xPool.deleteObject(e); }
	static xObjectPool<TriggerEvent> m_xPool;
};