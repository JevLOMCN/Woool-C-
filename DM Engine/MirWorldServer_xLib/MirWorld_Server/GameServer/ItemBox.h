#pragma once
class CItemBox
{
public:
	CItemBox(void);
	virtual ~CItemBox(void);
	BOOL Create(int size);
	ITEM* FindItem(DWORD dwMakeIndex, BOOL IsBind = FALSE);
	ITEM* FindItem(const char* pszName, BOOL IsBind = FALSE);
	int FindItems(DWORD dwMakeIndex, ITEM** pItems, int maxcount, BOOL IsBind = FALSE);
	int FindItems(const char* pszName, ITEM** pItems, int maxcount, BOOL IsBind = FALSE);
	BOOL RemoveItem(DWORD dwMakeIndex);
	BOOL AddItem(ITEM& item);
	int	GetItemCount(const char* pszName);
	BOOL TakeItemCount(const char* pszName, int count, ITEM* pArray);
	VOID Clean();
	BOOL SetItems(ITEM* pItems, int count);
	int GetItems(ITEM* pItems, int maxcount);
	int GetClientItems(ITEMCLIENT* pItems, int maxcount);
	int GetItemPos(BAGITEMPOS* pItemPos, int maxcount);
	UINT CalcWeight()
	{
		if (m_nCacheWeight == 0)
		{
			UINT weight = 0;
			for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
			{
				if (m_ItemArray[i] && m_ItemArray[i]->dwMakeIndex != 0)
					weight += m_ItemArray[i]->baseitem.btWeight;
			}
			m_nCacheWeight = weight;
		}
		return m_nCacheWeight;
	}
	int GetFree() { if (m_iCountLimit >= m_ItemArray.GetCount())return (m_iCountLimit - m_ItemArray.GetCount()); return 0; }
	int	GetCountLimit()const { return m_iCountLimit; }
	void SetCountLimit(int iCountLimit) { m_iCountLimit = iCountLimit; }
	int	GetCount() { return m_ItemArray.GetCount(); }
	ITEM* GetItem(int Index) { return m_ItemArray[Index]; }
private:
	VOID SetCache(ITEM* pCacheItem, int pos)
	{
		m_pCachedItem = pCacheItem;
		m_iCachedIndex = pos;
	}
	VOID ClrCache()
	{
		m_pCachedItem = nullptr;
		m_iCachedIndex = -1;
	}
	UINT m_iCount;
	UINT m_iCountLimit;
	xAutoPtrArray<ITEM>	m_ItemArray;
	ITEM* FindItemByIndex(DWORD dwMakeIndex, BOOL IsBind = FALSE);
	ITEM* FindItemByName(const char* pszName, BOOL IsBind = FALSE);
	ITEM* m_pCachedItem;
	UINT m_nCacheWeight;
	UINT m_iCachedIndex;
#ifdef USE_FREE_MEMORY
	static ITEM* NewItem() { return (new ITEM); }
	static VOID DeleteItem(ITEM* pItem) { delete pItem; }
#else
	static ITEM* NewItem() { return m_ItemObjectPool.newObject(); }
	static VOID DeleteItem(ITEM* pItem) { m_ItemObjectPool.deleteObject(pItem); }
	static xObjectPool<ITEM> m_ItemObjectPool;
#endif
};