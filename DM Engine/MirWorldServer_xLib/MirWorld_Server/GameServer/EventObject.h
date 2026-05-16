#pragma once
#include "mapobject.h"
class CMapObject;
// 事件对象基类, 继承自CMapObject类
class CEventObject : public CMapObject
{
public:
	CEventObject(void);
	virtual ~CEventObject(void);
	// 清理事件对象数据
	virtual VOID Clean();
	// 对象进入事件区域（虚函数, 由派生类实现）
	virtual VOID OnEnter(CMapObject* pObject) {}
	// 对象离开事件区域（虚函数, 由派生类实现）
	virtual VOID OnLeave(CMapObject* pObject) {}
	// 禁用事件（虚函数, 由派生类实现）
	virtual VOID Disable() { m_bDisabled = TRUE; }
	// 启用事件（虚函数, 由派生类实现）
	virtual VOID Enable() { m_bDisabled = FALSE; }
	// 获取对象类型（返回事件类型）
	e_object_type GetType() { return OBJ_EVENT; }
	// 获取类类型（返回事件类）
	e_class_type GetClassType() { return CLS_EVENT; }
	// 检查事件是否被禁用
	BOOL IsDisabled()const { return m_bDisabled; }
	// 设置进入标志
	VOID SetEnterFlag(CLogicMap* pMap);
	// 设置离开标志
	VOID SetLeaveFlag(CLogicMap* pMap);
	// 离开地图事件（虚函数, 由派生类实现）
	virtual VOID OnLeaveMap(CLogicMap* pMap);
protected:
	BOOL m_bDisabled; // 事件是否被禁用
};