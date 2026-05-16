#pragma once
class CVisibleEvent;
class CMapObject;
// 事件处理器基类, 继承自链表节点类
class CEventProcessor : public xListHost<CEventProcessor>::xListNode
{
public:
	CEventProcessor(void);
	virtual ~CEventProcessor(void);
	// 更新处理器状态（虚函数, 由派生类实现）
	virtual VOID Update() {}
	// 对象进入事件区域（虚函数, 由派生类实现）
	// pEvent: 触发的事件
	// pObject: 进入的对象
	virtual VOID OnEnter(CVisibleEvent* pEvent, CMapObject* pObject) {}
	// 对象离开事件区域（虚函数, 由派生类实现）
	// pEvent: 触发的事件
	// pObject: 离开的对象
	virtual VOID OnLeave(CVisibleEvent* pEvent, CMapObject* pObject) {}
	// 事件更新（虚函数, 由派生类实现）
	// pEvent: 需要更新的事件
	virtual VOID OnUpdate(CVisibleEvent* pEvent) {}
	// 事件关闭（虚函数, 由派生类实现）
	// pEvent: 关闭的事件
	virtual VOID OnClose(CVisibleEvent* pEvent) {}
	// 事件创建（虚函数, 由派生类实现）
	// pEvent: 新创建的事件
	virtual VOID OnCreate(CVisibleEvent* pEvent) {}
};