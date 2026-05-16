#pragma once
#include "eventprocessor.h"

class CAliveObject;
//破壳事件类, 继承自CEventProcessor基类
class SculpEiceEvent : public CEventProcessor
{
public:
	SculpEiceEvent(void);
	virtual ~SculpEiceEvent(void);
	SculpEiceEvent(const SculpEiceEvent&) = delete;           // 禁止拷贝构造
	SculpEiceEvent& operator=(const SculpEiceEvent&) = delete; // 禁止拷贝赋值
	// 更新可见事件状态
	VOID OnUpdate(CVisibleEvent* pEvent);
	// 关闭可见事件
	VOID OnClose(CVisibleEvent* pEvent);
	// 销毁事件实例
	VOID Destroy();
	// 更新事件状态
	VOID Update();
	/**
	 * 创建破壳事件实例
	 * @param pOwner 事件所有者
	 * @param x 事件x坐标
	 * @param y 事件y坐标
	 * @param nDamage 伤害值
	 * @param dwTick 触发时间
	 * @param dwLastTime 持续时间
	 * @return SculpEiceEvent* 创建的事件实例指针
	 */
	static SculpEiceEvent* Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime);
private:
	int	m_nDamage; // 伤害值
	DWORD m_dwOwnerInstanceKey; // 所有者实例键值
	CAliveObject* m_pOwner; // 事件所有者
	CVisibleEvent* m_pEvent; // 事件对象
	static SculpEiceEvent* NewObject() { return m_xEventPool.newObject(); } // 创建新对象
	static VOID DeleteEvent(SculpEiceEvent* e) { e->Destroy(); m_xEventPool.deleteObject(e); } // 消耗对象
	static xObjectPool<SculpEiceEvent> m_xEventPool; // 对象池
};