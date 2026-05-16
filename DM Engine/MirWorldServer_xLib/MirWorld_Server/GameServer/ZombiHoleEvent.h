#pragma once
#include "eventprocessor.h"

class CAliveObject;
/**
 * 僵尸洞事件类
 * 继承自CEventProcessor, 用于处理游戏中的僵尸洞事件
 * 管理僵尸洞的创建、更新和销毁, 以及相关伤害效果
 */
class ZombiHoleEvent : public CEventProcessor
{
public:
	ZombiHoleEvent(void);
	virtual ~ZombiHoleEvent(void);
	ZombiHoleEvent(const ZombiHoleEvent&) = delete;           // 禁止拷贝构造
	ZombiHoleEvent& operator=(const ZombiHoleEvent&) = delete; // 禁止拷贝赋值
	// 更新可见事件状态
	VOID OnUpdate(CVisibleEvent* pEvent);
	// 关闭可见事件
	VOID OnClose(CVisibleEvent* pEvent);
	// 销毁事件实例
	VOID Destroy();
	// 更新事件状态
	VOID Update();
	/**
	 * 创建僵尸洞事件实例
	 * @param pOwner 事件所有者
	 * @param x 事件x坐标
	 * @param y 事件y坐标
	 * @param nDamage 伤害值
	 * @param dwTick 触发时间
	 * @param dwLastTime 持续时间
	 * @return ZombiHoleEvent* 创建的事件实例指针
	 */
	static ZombiHoleEvent* Create(CAliveObject* pOwner, int x, int y, int nDamage, DWORD dwTick, DWORD dwLastTime);
private:
	int	m_nDamage; // 伤害值
	DWORD m_dwOwnerInstanceKey; // 所有者实例键值
	CAliveObject* m_pOwner; // 事件所有者
	CVisibleEvent* m_pVisibleEvent; // 事件
	static ZombiHoleEvent* NewEvent() { return m_xPool.newObject(); }
	static VOID DeleteEvent(ZombiHoleEvent* e) { e->Destroy(); m_xPool.deleteObject(e); }
	static xObjectPool<ZombiHoleEvent> m_xPool;
};