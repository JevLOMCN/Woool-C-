#pragma once
#include "eventprocessor.h"

#define	BLUEFIRE_EVENT_NUM 25 // 蓝焰数量

class CAliveObject;
// 蓝色火焰燃烧事件类, 继承自CEventProcessor基类
class CBlueFireEvent : public CEventProcessor
{
public:
	CBlueFireEvent(void);
	virtual ~CBlueFireEvent(void);
	CBlueFireEvent(const CBlueFireEvent&) = delete;           // 禁止拷贝构造
	CBlueFireEvent& operator=(const CBlueFireEvent&) = delete; // 禁止拷贝赋值
	// 事件更新处理
	VOID OnUpdate(CVisibleEvent* pEvent);
	// 事件关闭处理
	VOID OnClose(CVisibleEvent* pEvent);
	// 销毁蓝色火焰燃烧事件
	VOID Destroy();
	// 更新事件状态
	VOID Update();
	//VOID OnEnter( CVisibleEvent * pEvent, CMapObject * pObject );
	//VOID OnCreate( CVisibleEvent * pEvent );

	// 静态方法：创建蓝色火焰燃烧事件
	// pOwner: 事件所有者
	// x,y: 事件位置
	// nDamage: 伤害值
	// dwTick: 触发间隔
	// dwLastTime: 持续时间
	static CBlueFireEvent* Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime);
private:
	int	m_nDamage; // 伤害值
	DWORD m_dwOwnerInstanceKey; // 所有者实例键值
	CAliveObject* m_pOwner; // 事件所有者
	CVisibleEvent* m_pEvents[BLUEFIRE_EVENT_NUM]; // 事件数组
	// 对象池相关方法
	static CBlueFireEvent* NewObject() { return m_xEventPool.newObject(); } // 创建新对象
	static void RemoveObject(CBlueFireEvent* pObject) { pObject->Destroy(); m_xEventPool.deleteObject(pObject); } // 删除对象
	static xObjectPool<CBlueFireEvent>	m_xEventPool; // 对象池
};