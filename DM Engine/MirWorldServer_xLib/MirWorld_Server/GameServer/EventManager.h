#pragma once
class CVisibleEvent;
class CLogicMap;
class CEventProcessor;
class CMapObject;

// 事件管理器类, 采用单例模式
class CEventManager : public xSingletonClass<CEventManager>
{
public:
	CEventManager(void);
	virtual ~CEventManager(void);
	// 创建新的可见事件
	// pMap: 事件所在地图
	// x,y: 事件位置
	// dwView: 事件类型
	// dwRunTick: 运行间隔
	// dwLastTime: 持续时间
	// pProcessor: 事件处理器
	// dwParam1,dwParam2: 事件参数
	CVisibleEvent* NewVisibleEvent(CLogicMap* pMap, int x, int y, DWORD dwView, DWORD dwRunTick, DWORD dwLastTime, CEventProcessor* pProcessor, DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	// 删除可见事件
	VOID DelVisibleEvent(CVisibleEvent* pEvent);
	// 预删除可见事件（加入删除队列）
	VOID PreDelVisibleEvent(CVisibleEvent* pEvent);
	// 更新待删除对象队列
	VOID UpdateDeleteObject();
	// 添加事件处理器
	VOID AddEventProcessor(CEventProcessor* pProcessor);
	// 移除事件处理器
	VOID RemoveEventProcessor(CEventProcessor* pProcessor);
	// 检测周边是否有指定事件
	// pMap: 事件所在地图
	// x,y: 事件位置
	// nRange: 范围大小
	// dwView: 事件类型
	BOOL CheckNearEvent(CLogicMap* pMap, int x, int y, int nRange, DWORD dwView);
	// 更新所有事件
	VOID UpdateEvents();
private:
	xObjectPool<CVisibleEvent> m_VisibleEventPool; // 可见事件对象池
	xPtrQueue<CVisibleEvent> m_DeleteObjectQueue; // 待删除对象队列
	xListHost<CEventProcessor> m_xProcessorList; // 事件处理器列表
	xListHost<CMapObject> m_xVisibleEventList; // 可见事件列表
	xListHost<CMapObject>::xListNode* m_pCurUpdateEvent; // 当前更新的事件节点
	CEventProcessor* m_pCurUpdateProcessor; // 当前更新的处理器
};