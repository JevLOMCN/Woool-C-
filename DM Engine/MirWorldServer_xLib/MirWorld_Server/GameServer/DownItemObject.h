#pragma once
#include "mapobject.h"

class CHumanPlayer;
//掉落物品对象
class CDownItemObject : public CMapObject
{
public:
	CDownItemObject(void);
	virtual ~CDownItemObject(void);
	VOID Clean();
	BOOL GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr);
	BOOL GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr);
	BOOL CanRecvMsg() { return FALSE; }
	e_object_type GetType() { return OBJ_DOWNITEM; }
	//获取物品
	ITEM& GetItem() { return m_Item; }
	//设置物品
	VOID SetItem(ITEM& item) { m_Item = item; }
	//获取拥有者ID
	UINT GetOwnerId()const { return m_OwnerId; }
	//设置拥有者ID
	VOID SetOwnerId(UINT id) { m_OwnerId = id; }
	//已掉落监听
	VOID OnDroped();
	//更新验证
	BOOL UpdateValid();
	//物品掉落进入地图
	VOID OnEnterMap(CLogicMap* pMap);
	//物品掉落离开地图
	VOID OnLeaveMap(CLogicMap* pMap);
	//是否是附身物品
	BOOL IsGold()const
	{
		return (m_Item.baseitem.btStdMode == 255);
	}
	//设置物品的动作对象
	VOID SetActionObject(CHumanPlayer* pObject);
private:
	CHumanPlayer* m_pActionObject;
	DWORD m_dwActionObjectInstanceKey;
	ITEMCLASS* m_pClass;
	UINT m_nScriptTimes;
	CServerTimer m_tTimer;
	UINT m_OwnerId;
	ITEM m_Item;
};