#include "StdAfx.h"
#include ".\monitemsmgr.h"
#include ".\itemmanager.h"
#include ".\gameworld.h"

CMonItemsMgr::CMonItemsMgr(void)
{
}

CMonItemsMgr::~CMonItemsMgr(void)
{
}

BOOL CMonItemsMgr::LoadMonItems(const char* pszPath)
{
	return StartFind(pszPath, "*.txt", TRUE);
}

VOID CMonItemsMgr::OnFoundFile(const char* pszFilename, UINT nParam1)
{
	MONITEMS* pItems = nullptr;
	if (nParam1 == 0)
	{
		char szName[64];
		_splitpath(pszFilename, nullptr, nullptr, szName, nullptr);

		pItems = this->GetMonItems(szName);
		if (pItems == nullptr)
		{
			pItems = new MONITEMS;
			_splitpath(pszFilename, nullptr, nullptr, pItems->szMonName, nullptr);
			o_strncpy(pItems->szFilename, pszFilename, 250);
		}
		else
		{
			DOWNITEM* ppp = pItems->pItems;
			while (ppp)
			{
				pItems->pItems = ppp->pNext;
				delete ppp;
				ppp = pItems->pItems;
			}
			PRINT(SUCCESS_GREEN, "更新 %s 的物品掉落文件 %s \n", pItems->szMonName, pItems->szFilename);
		}
	}
	else
		pItems = this->GetMonItems(pszFilename);
	CStringFile sf(pItems->szFilename);
	DOWNITEM* pDownItem = nullptr;
	char* p = nullptr;
	char* p1 = nullptr;
	char* Params[5];
	int	nParam = 0;
	char* pLine = nullptr;
	xCharSet wht(" \t");
	xCharSet delm(" -/\t");
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == '#' || *pLine == 0)continue;
		nParam = ExtractStrings(pLine, wht, delm, Params, 5, FALSE, FALSE);

		if (nParam < 3)continue;
		if (pDownItem == nullptr)pDownItem = new DOWNITEM;
		memset(pDownItem, 0, sizeof(DOWNITEM));
		if (*Params[2] == '*')
		{
			pDownItem->bRandomUpgradeItem = TRUE;
			Params[2]++;
		}
		else
			pDownItem->bRandomUpgradeItem = FALSE;
		if (strcmp(Params[2], CGameWorld::GetInstance()->GetName(ENI_GOLD)) != 0)
		{
			pDownItem->bGold = FALSE;
			ITEMCLASS* pItem = CItemManager::GetInstance()->GetItemClassByName(Params[2]);
			if (pItem == nullptr)
			{
				PRINT(ERROR_RED, "在 %s 的物品掉落文件中 %u 行发现未定义的物品 %s !\n", pItems->szMonName, i + 1, Params[2]);
				continue;
			}
		}
		else
			pDownItem->bGold = TRUE;

		o_strncpy(pDownItem->szName, Params[2], 31);

		pDownItem->nMin = StringToInteger(Params[0]);
		pDownItem->nMax = StringToInteger(Params[1]);

		if (nParam > 3)
			pDownItem->nCount = StringToInteger(Params[3]);
		else
			pDownItem->nCount = 1;

		if (nParam > 4)
			pDownItem->nCountMax = StringToInteger(Params[4]);
		else
			pDownItem->nCountMax = pDownItem->nCount;

		pDownItem->nCycleMax = GetRangeRand(pDownItem->nMin, pDownItem->nMax);
		pDownItem->nCur = Getrand(pDownItem->nCycleMax);
		pDownItem->pNext = pItems->pItems;
		pItems->pItems = pDownItem;
		pDownItem = nullptr;
	}
	m_MonItemsHash.HAdd(pItems->szMonName, (LPVOID)pItems);
}

BOOL CMonItemsMgr::CreateDownItem(DOWNITEM* pDownItem, ITEM& item)
{
	memset(&item, 0, sizeof(item));
	CItemManager* ItemManager = CItemManager::GetInstance();
	if (pDownItem->bGold)
	{
		DWORD dwCount = GetRangeRand(pDownItem->nCount, pDownItem->nCountMax);
		o_strncpy(item.baseitem.szName, CGameWorld::GetInstance()->GetName(ENI_GOLD), 12);
		item.baseitem.btNameLength = (BYTE)strlen(item.baseitem.szName);
		item.baseitem.btStdMode = 255;
		ItemManager->IdentItem(item);
		item.baseitem.wImageIndex = GetGoldImageIndex(dwCount);
		item.wCurDura = dwCount & 0xffff;
		item.wMaxDura = (dwCount & 0xffff0000) >> 16;
	}
	else
	{
		int ratefix = 0;
		if (pDownItem->bRandomUpgradeItem)
		{
			int rate = 0;
			bool isSingleCount = (pDownItem->nCount == 1 && pDownItem->nCountMax == 1);
			if (isSingleCount) rate = CGameWorld::GetInstance()->GetVar(EVI_RANDOMUPGRADEITEMRATE);
			ratefix = isSingleCount ? rate : GetRangeRand(pDownItem->nCount, pDownItem->nCountMax);
		}
		if (!ItemManager->CreateTempItem(pDownItem->szName, item, TRUE, ratefix))return FALSE;
	}
	return TRUE;
}