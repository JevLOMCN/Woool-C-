#pragma once
#include "downitemobject.h"
class CHumanPlayer;

//掉落物品管理
class CDownItemMgr : public xSingletonClass<CDownItemMgr>
{
public:
	CDownItemMgr(void);
	virtual ~CDownItemMgr(void);
	//新建掉落物品对象
	CDownItemObject* NewDownItemObject(ITEM& item, WORD x, WORD y, DWORD dwOwner = 0);
	//删除掉落物品对象
	BOOL DeleteDownItemObject(CDownItemObject* pObject);
	//删除掉落物品对象内存
	BOOL DeleteDownItemObjectImm(CDownItemObject* pObject);
	//掉落物品
	BOOL DropItem(CLogicMap* pMap, ITEM& item, WORD x, WORD y, DWORD dwOwner = 0);
	//掉落物品
	BOOL DropItem(CLogicMap* pMap, ITEM& item, WORD x, WORD y, BOOL boOwner, CHumanPlayer* pActionObject = nullptr);
	//拾取物品
	BOOL PickupItem(CLogicMap* pMap, CDownItemObject* pObject, CHumanPlayer* pActionObject);
	//删除周围物品
	BOOL DeleteGroundItem(CLogicMap* pMap, CDownItemObject* pObject, BOOL bDeleteItem = TRUE);
	//更新删除物品对象
	VOID UpdateDeletedObject();
	//获取掉落物品数量
	int	getCount()
	{
		return m_xDownItemList.getCount();
	}
	//更新掉落物品
	VOID UpdateDownItem();
protected:
	CDownItemObject* newObject();
	VOID deleteObject(CDownItemObject*);
private:
	UINT m_nCurFreeIndex;
	xObjectPool<CDownItemObject> m_xDownItemPool;
	xIndexPtrList<CDownItemObject> m_xDownItemList;
	xPtrQueue<CDownItemObject>	m_xDelItemQueue;
};