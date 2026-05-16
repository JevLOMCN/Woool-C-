#pragma once
#include "eventobject.h"

class CEventProcessor;
/**
 * 可见事件类
 * 继承自CEventObject, 用于处理游戏中的可见事件
 * 管理事件在地图上的显示、生命周期和与玩家的交互
 */
class CVisibleEvent : public CEventObject
{
public:
	CVisibleEvent(void);
	virtual ~CVisibleEvent(void);
	/**
	 * 创建可见事件
	 * @param pMap 事件所在地图
	 * @param x 事件x坐标
	 * @param y 事件y坐标
	 * @param dwView 事件视野范围
	 * @param dwRunTick 事件开始运行的时间
	 * @param dwLastTime 事件持续时间
	 * @param pProcessor 事件处理器
	 * @param dwParam1 自定义参数1
	 * @param dwParam2 自定义参数2
	 * @return BOOL 创建是否成功
	 */
	BOOL Create(CLogicMap* pMap, int x, int y, DWORD dwView, DWORD dwRunTick, DWORD dwLastTime, CEventProcessor* pProcessor, DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	// 关闭事件
	VOID Close();
	// 清理事件资源
	VOID Clean();
	/**
	 * 获取进入视野消息
	 * @param pszMsg 消息缓冲区
	 * @param length 消息长度
	 * @param pViewer 观察者对象
	 * @return BOOL 是否成功获取消息
	 */
	BOOL GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr);
	/**
	 * 获取离开视野消息
	 * @param pszMsg 消息缓冲区
	 * @param length 消息长度
	 * @param pViewer 观察者对象
	 * @return BOOL 是否成功获取消息
	 */
	BOOL GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr);
	// 获取对象类型
	e_object_type GetType() { return OBJ_VISIBLEEVENT; }
	// 当对象进入事件范围时触发
	VOID OnEnter(CMapObject* pObject);
	// 当对象离开事件范围时触发
	VOID OnLeave(CMapObject* pObject);
	// 当事件进入地图时触发
	VOID OnEnterMap(CLogicMap* pMap);
	// 当事件离开地图时触发
	VOID OnLeaveMap(CLogicMap* pMap);
	// 更新事件有效性
	BOOL UpdateValid();
	// 设置事件参数
	VOID SetParam(DWORD dwParam1, DWORD dwParam2) { m_dwParam1 = dwParam1; m_dwParam2 = dwParam2; }
	// 获取参数1
	DWORD GetParam1()const { return m_dwParam1; }
	// 获取参数2
	DWORD GetParam2()const { return m_dwParam2; }
	// 获取视野范围
	DWORD GetView()const { return m_dwView; }
	// 获取事件处理器
	CEventProcessor* getProcessor() { return m_pEventProcessor; }
	// 事件处理器是否已经关闭
	BOOL IsClosed()const { return m_bClosed; }
protected:
	DWORD m_dwView; // 事件视野范围
	CServerTimer m_RunTimer; // 事件运行计时器
	CServerTimer m_CloseTimer; // 事件关闭计时器
	CEventProcessor* m_pEventProcessor; // 事件处理器指针
	BOOL m_bClosed; // 事件是否已关闭的标志
private:
	DWORD m_dwParam1; // 自定义参数1
	DWORD m_dwParam2; // 自定义参数2
};