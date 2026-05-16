#pragma once
#include "eventprocessor.h"
// 最大逆魔火墙事件数量
constexpr int MAX_FIRENIMOEVENT_COUNT = 13;

class CAliveObject;
// 逆魔火墙事件类, 继承自CEventProcessor基类
class CFireNimoBurnEvent : public CEventProcessor
{
public:
	CFireNimoBurnEvent(void);
	virtual ~CFireNimoBurnEvent(void);
	CFireNimoBurnEvent(const CFireNimoBurnEvent&) = delete;           // 禁止拷贝构造
	CFireNimoBurnEvent& operator=(const CFireNimoBurnEvent&) = delete; // 禁止拷贝赋值
	// 事件更新处理
	VOID OnUpdate(CVisibleEvent* pEvent);
	// 事件关闭处理
	VOID OnClose(CVisibleEvent* pEvent);
	// 销毁逆魔火墙事件
	VOID Destroy();
	// 更新事件状态
	VOID Update();
	// 创建火墙事件
	// pOwner: 事件所有者
	// x,y: 事件位置
	// type: 火墙类型(1=直线3个, 2=身边3个, 3=随机3个, 4=5x5火墙)
	// nDamage: 伤害值
	// dwTick: 触发间隔
	// dwLastTime: 持续时间
	static CFireNimoBurnEvent* Create(CAliveObject* pOwner, int x, int y, int type, int nDamage, DWORD dwTick, DWORD dwLastTime);
private:
	int	m_nDamage; // 伤害值
	DWORD m_dwOwnerInstanceKey; // 所有者实例键值
	CAliveObject* m_pOwner; // 事件所有者
	CVisibleEvent* m_pEvents[MAX_FIRENIMOEVENT_COUNT]; // 事件数组
	// 对象池相关方法
	static CFireNimoBurnEvent* NewObject() { return m_xEventPool.newObject(); } // 创建新对象
	static void RemoveObject(CFireNimoBurnEvent* e) { e->Destroy(); m_xEventPool.deleteObject(e); } // 删除对象
	static xObjectPool<CFireNimoBurnEvent> m_xEventPool; // 对象池
};