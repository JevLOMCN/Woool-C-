#pragma once
#include "eventprocessor.h"
#include "eventobject.h"

class CMonsterTrapper;
class CLogicMap;
class CHumanPlayer;
// 陷阱事件类, 继承自CEventObject基类
class CTrapperEvent : public CEventObject
{
public:
	CTrapperEvent(CMonsterTrapper* pTrapper);
	~CTrapperEvent();
	// 清理陷阱事件数据
	VOID Clean();
	// 对象进入陷阱事件
	VOID OnEnter(CMapObject* pObject);
	//VOID OnLeave( CMapObject * pObject );

	// 进入地图
	VOID EnterMap(CLogicMap* pMap, UINT x, UINT y);
	// 离开地图
	VOID LeaveMap();
	// 进入地图事件
	VOID OnEnterMap(CLogicMap* pMap);
	// 离开地图事件
	VOID OnLeaveMap(CLogicMap* pMap);
protected:
	CMonsterTrapper* m_pTrapper; // 陷阱所有者指针
};


class CAliveObject;
// 怪物陷阱类, 继承自CEventProcessor基类
class CMonsterTrapper : public CEventProcessor
{
public:
	CMonsterTrapper(void);
	virtual ~CMonsterTrapper(void);
	CMonsterTrapper(const CMonsterTrapper&) = delete;           // 禁止拷贝构造
	CMonsterTrapper& operator=(const CMonsterTrapper&) = delete; // 禁止拷贝赋值
	// 事件更新处理
	VOID OnUpdate(CVisibleEvent* pEvent);
	// 事件关闭处理
	VOID OnClose(CVisibleEvent* pEvent);
	//VOID OnEnter( CVisibleEvent * pEvent, CMapObject * pObject );
	//VOID OnCreate( CVisibleEvent * pEvent );

	// 静态方法：创建怪物陷阱
	// pOwner: 陷阱所有者
	// x,y: 陷阱位置
	// nDamage: 伤害值
	// dwColor: 颜色
	// dwLastTime: 持续时间
	static CMonsterTrapper* Create(CHumanPlayer* pOwner, int x, int y, int nDamage, DWORD dwColor, DWORD dwLastTime);
	// 销毁陷阱
	VOID Destroy();
	// 更新陷阱状态
	VOID Update();
	// 获取陷阱所有者
	CHumanPlayer* GetOwner() { return m_pOwner; }
	// 获取伤害值
	int	GetDamage()const { return m_nDamage; }
	// 获取持续时间
	DWORD GetLastTime()const { return m_dwLastTime; }
	// 设置陷阱失败状态
	VOID SetFailed() { m_bFailed = TRUE; }
	// 获取捕获的怪物数量
	DWORD GetTrappedCount()const { return m_dwMonsterCount; }
	// 增加捕获的怪物数量
	VOID AddTrappedCount() { m_dwMonsterCount++; }
private:
	DWORD m_dwMonsterCount; // 捕获的怪物数量
	BOOL m_bFailed; // 是否失败
	int	m_nDamage; // 伤害值
	DWORD m_dwLastTime; // 持续时间
	DWORD m_dwOwnerInstanceKey; // 所有者实例键值
	CHumanPlayer* m_pOwner; // 陷阱所有者
	CVisibleEvent* m_pEvents[8]; // 事件数组
	CTrapperEvent* m_pTrapped[9]; // 被捕获的怪物数组
	// 对象池相关方法
	static CMonsterTrapper* NewObject() { return m_xEventPool.newObject(); } // 创建新对象
	static void RemoveObject(CMonsterTrapper* pObject) { m_xEventPool.deleteObject(pObject); } // 删除对象
	static xObjectPool<CMonsterTrapper>	m_xEventPool; // 对象池
};