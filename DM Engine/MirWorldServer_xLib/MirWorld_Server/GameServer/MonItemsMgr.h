#pragma once
#include "findfile.h"

typedef struct tagDownItem
{
	tagDownItem()
	{
		FILLSELF(0);
	}
	BOOL updatecycle()
	{
		nCur++;
		if (nCur >= nCycleMax)
		{
			if (nMax < 5)
				nCycleMax = nMax;
			else
				nCycleMax = GetRangeRand(nMin, nMax);
			nCur = 0;
			return TRUE;
		}
		return FALSE;
	}
	char szName[32];
	int	nCount;
	int nCountMax;
	int	nMax;
	int	nMin;
	int	nCur;
	int	nCycleMax;
	bool bRandomUpgradeItem; // 是否按自定义随机物品属性.
	bool bGold;
	BYTE btFlag[2];
	tagDownItem* pNext;
}DOWNITEM;

typedef struct tagMonItems
{
	tagMonItems()
	{
		FILLSELF(0);
	}
	DOWNITEM* pItems;
	char szMonName[32];
	char szFilename[256];
}MONITEMS;

class CMonItemsMgr : public CFindFile, public xSingletonClass<CMonItemsMgr>
{
public:
	CMonItemsMgr(void);
	virtual ~CMonItemsMgr(void);
	BOOL LoadMonItems(const char* pszPath);
	MONITEMS* GetMonItems(const char* pMonsterName)
	{
		return (MONITEMS*)m_MonItemsHash.HGet(pMonsterName);
	}
	VOID OnFoundFile(const char* pszFilename, UINT nParam1 = 0);
	BOOL CreateDownItem(DOWNITEM* pDownItem, ITEM& item);
private:
	CNameHash m_MonItemsHash;
};