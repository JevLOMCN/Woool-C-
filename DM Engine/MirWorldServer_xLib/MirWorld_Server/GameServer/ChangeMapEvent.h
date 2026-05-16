#pragma once
#include "eventobject.h"

// 地图切换事件类, 继承自CEventObject基类
class CChangeMapEvent : public CEventObject
{
public:
	CChangeMapEvent(void);
	virtual ~CChangeMapEvent(void);
	CChangeMapEvent(const CChangeMapEvent&) = delete;           // 禁止拷贝构造
	CChangeMapEvent& operator=(const CChangeMapEvent&) = delete; // 禁止拷贝赋值
    // 对象进入事件区域
	virtual VOID OnEnter(CMapObject* pObject);
	// 对象离开事件区域
	virtual VOID OnLeave(CMapObject* pObject);
	// 进入地图事件
	VOID OnEnterMap(CLogicMap* pMap);
	// 清理事件数据
	VOID Clean();
	// 静态方法：创建地图切换事件
	// mapid: 当前地图ID
	// x,y: 事件触发位置
	// tomapid: 目标地图ID
	// tox,toy: 目标地图位置
	static CChangeMapEvent* Create(int mapid, int x, int y, int tomapid, int tox, int toy);
protected:
	BOOL Init(int mapid, int x, int y, int tomapid, int tox, int toy);
private:
	CLogicMap* m_pToMap; // 目标地图指针
	int m_nToX; // 目标X坐标
	int m_nToY; // 目标Y坐标
	int m_nMapId; // 当前地图ID
	// 对象池相关方法
	static CChangeMapEvent* newObject(); // 创建新对象
	static VOID deleteObject(CChangeMapEvent*); // 删除对象
	static xObjectPool<CChangeMapEvent> m_xObjectPool; // 对象池
};