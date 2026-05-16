#include "StdAfx.h"
#include "ItemBox.h"
#include "ItemManager.h"
#ifndef USE_FREE_MEMORY
xObjectPool<ITEM> CItemBox::m_ItemObjectPool;
#endif

CItemBox::CItemBox(void)
{
	m_iCountLimit = 0;
	m_iCount = 0;
	m_pCachedItem = nullptr;
	m_iCachedIndex = -1;
	m_nCacheWeight = 0;
}

CItemBox::~CItemBox(void)
{
}

VOID CItemBox::Clean()
{
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		DeleteItem(m_ItemArray[i]);
	}
	m_ItemArray.Clean();
	m_iCountLimit = m_ItemArray.GetMaxCount();
	m_iCount = 0;
	m_pCachedItem = nullptr;
	m_iCachedIndex = -1;
	m_nCacheWeight = 0;
}

BOOL CItemBox::Create(int size)
{
	if (!m_ItemArray.Create(size))return FALSE;
	SetCountLimit(size);
	return TRUE;
}

ITEM* CItemBox::FindItem(DWORD dwMakeIndex, BOOL IsBind)
{
	return FindItemByIndex(dwMakeIndex, IsBind);
}

int CItemBox::FindItems(DWORD dwMakeIndex, ITEM** pItems, int maxcount, BOOL IsBind)
{
	int rcount = 0;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		if (m_ItemArray[i] && m_ItemArray[i]->dwMakeIndex == dwMakeIndex)
		{
			if (IsBind && m_ItemArray[i]->IsBind())
			{
				pItems[rcount++] = m_ItemArray[i];
				if (rcount >= maxcount) return rcount;
			}
			else
			{
				pItems[rcount++] = m_ItemArray[i];
				if (rcount >= maxcount) return rcount;
			}
		}
	}
	return rcount;
}

ITEM* CItemBox::FindItem(const char* pszName, BOOL IsBind)
{
	return FindItemByName(pszName, IsBind);
}

int CItemBox::FindItems(const char* pszName, ITEM** pItems, int maxcount, BOOL IsBind)
{
	int rcount = 0;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		char* szFullName = CItemManager::GetInstance()->GetItemClassPtr(*m_ItemArray[i])->szFullName;
		if (m_ItemArray[i] && strcmp(szFullName, pszName) == 0)
		{
			if (IsBind && m_ItemArray[i]->IsBind())
			{
				pItems[rcount++] = m_ItemArray[i];
				if (rcount >= maxcount) return rcount;
			}
			else
			{
				pItems[rcount++] = m_ItemArray[i];
				if (rcount >= maxcount) return rcount;
			}
		}
	}
	return rcount;
}

BOOL CItemBox::RemoveItem(DWORD dwMakeIndex)
{
	ITEM* pItem = FindItemByIndex(dwMakeIndex);
	if (pItem)
	{
		if (pItem == m_pCachedItem && m_ItemArray[m_iCachedIndex] == pItem)
		{
			m_ItemArray.Del(m_iCachedIndex);
			ClrCache();
		}
		else
		{
			if (pItem == m_pCachedItem)
			{
				ClrCache();
			}
			m_ItemArray.Del(pItem);
		}
		if (m_nCacheWeight > pItem->baseitem.btWeight)
		{
			m_nCacheWeight -= pItem->baseitem.btWeight;
		}
		else
		{
			m_nCacheWeight = 0;
			m_nCacheWeight = CalcWeight();
		}
		pItem->dwMakeIndex = 0;
		DeleteItem(pItem);
		return TRUE;
	}
	return FALSE;
}

BOOL CItemBox::AddItem(ITEM& item)
{
	if (m_ItemArray.GetCount() >= m_iCountLimit)
		return FALSE;
	ITEM* pItem = NewItem();
	if (pItem == nullptr)return FALSE;
	*pItem = item;
	if (m_ItemArray.Add(pItem) == 0xffffffff)
	{
		DeleteItem(pItem);
		return FALSE;
	}
	m_nCacheWeight += pItem->baseitem.btWeight;
	return TRUE;
}

ITEM* CItemBox::FindItemByIndex(DWORD dwMakeIndex, BOOL IsBind)
{
	if (m_pCachedItem != nullptr && m_pCachedItem->dwMakeIndex == dwMakeIndex)
		return m_pCachedItem;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		if (m_ItemArray[i] && m_ItemArray[i]->dwMakeIndex == dwMakeIndex)
		{
			if (IsBind && m_ItemArray[i]->IsBind())
			{
				SetCache(m_ItemArray[i], i);
				return m_ItemArray[i];
			}
			else
			{
				SetCache(m_ItemArray[i], i);
				return m_ItemArray[i];
			}
		}
	}
	return nullptr;
}

ITEM* CItemBox::FindItemByName(const char* pszName, BOOL IsBind)
{
	char* szFullName = CItemManager::GetInstance()->GetItemClassPtr(*m_pCachedItem)->szFullName;
	if (m_pCachedItem != nullptr && strcmp(pszName, szFullName) == 0)
		return m_pCachedItem;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		char* szFullName = CItemManager::GetInstance()->GetItemClassPtr(*m_ItemArray[i])->szFullName;
		if (m_ItemArray[i] && strcmp(pszName, szFullName) == 0)
		{
			if (IsBind && m_ItemArray[i]->IsBind())
			{
				SetCache(m_ItemArray[i], i);
				return m_ItemArray[i];
			}
			else
			{
				SetCache(m_ItemArray[i], i);
				return m_ItemArray[i];
			}
		}
	}
	return nullptr;
}

BOOL CItemBox::SetItems(ITEM* pItems, int count)
{
	for (int i = 0; i < count; i++)
	{
		AddItem(pItems[i]);
	}
	return TRUE;
}

int CItemBox::GetItems(ITEM* pItems, int maxcount)
{
	int rcount = 0;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		if (m_ItemArray[i] && m_ItemArray[i]->dwMakeIndex != 0)
		{
			pItems[rcount++] = *m_ItemArray[i];
			if (rcount >= maxcount)return rcount;
		}
	}
	return rcount;
}

int	CItemBox::GetClientItems(ITEMCLIENT* pItems, int maxcount)
{
	int rcount = 0;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		if (m_ItemArray[i] && m_ItemArray[i]->dwMakeIndex != 0)
		{
			pItems[rcount++] = *(ITEMCLIENT*)m_ItemArray[i];
			if (rcount >= maxcount)return rcount;
		}
	}
	return rcount;
}

int CItemBox::GetItemPos(BAGITEMPOS* pItemPos, int maxcount)
{
	int rcount = 0;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		if (m_ItemArray[i] && m_ItemArray[i]->dwMakeIndex != 0)
		{
			pItemPos[rcount].wPos = rcount;
			pItemPos[rcount].ItemId = m_ItemArray[i]->dwMakeIndex;
			rcount++;
			if (rcount >= maxcount)return rcount;
		}
	}
	return rcount;
}

int	CItemBox::GetItemCount(const char* pszName)
{
	int count = 0;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		char* szFullName = CItemManager::GetInstance()->GetItemClassPtr(*m_ItemArray[i])->szFullName;
		if (m_ItemArray[i] && strcmp(szFullName, pszName) == 0)
			count++;
	}
	return count;
}

BOOL CItemBox::TakeItemCount(const char* pszName, int count, ITEM* pArray)
{
	int rcount = 0;
	for (UINT i = 0; i < m_ItemArray.GetCount(); i++)
	{
		char* szFullName = CItemManager::GetInstance()->GetItemClassPtr(*m_ItemArray[i])->szFullName;
		if (m_ItemArray[i] && strcmp(szFullName, pszName) == 0)
		{
			pArray[rcount++] = *m_ItemArray[i];
			if (rcount >= count)break;
		}
	}
	if (rcount >= count)
	{
		for (int i = 0; i < count; i++)
		{
			RemoveItem(pArray[i].dwMakeIndex);
		}
		return TRUE;
	}
	return FALSE;
}