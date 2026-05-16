#pragma once
#include "eventprocessor.h"

class CAliveObject;
// 火焰燃烧事件类, 继承自CEventProcessor基类
class CFireBurnEvent : public CEventProcessor
{
public:
	CFireBurnEvent(void);
	virtual ~CFireBurnEvent(void);
	CFireBurnEvent(const CFireBurnEvent&) = delete;           // 禁止拷贝构造
	CFireBurnEvent& operator=(const CFireBurnEvent&) = delete; // 禁止拷贝赋值
	// 事件更新处理
	VOID OnUpdate(CVisibleEvent* pEvent);
	// 事件关闭处理
	VOID OnClose(CVisibleEvent* pEvent);
	// 销毁火焰燃烧事件
	VOID Destroy();
	// 更新事件状态
	VOID Update();
	//VOID OnEnter( CVisibleEvent * pEvent, CMapObject * pObject );
	//VOID OnCreate( CVisibleEvent * pEvent );

	// 静态方法：创建火焰燃烧事件
	// pOwner: 事件所有者
	// x,y: 事件位置
	// nDamage: 伤害值
	// dwTick: 触发间隔
	// dwLastTime: 持续时间
	static CFireBurnEvent* Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime);
private:
	int	m_nDamage; // 伤害值
	DWORD m_dwOwnerInstanceKey; // 所有者实例键值
	CAliveObject* m_pOwner; // 事件所有者
	CVisibleEvent* m_pEvents[5]; // 事件数组（最多支持5个事件点）
	// 对象池相关方法
	static CFireBurnEvent* NewObject() { return m_xEventPool.newObject(); } // 创建新对象
	static void RemoveObject(CFireBurnEvent* pObject) { pObject->Destroy(); m_xEventPool.deleteObject(pObject); } // 删除对象
	static xObjectPool<CFireBurnEvent>	m_xEventPool; // 对象池
};