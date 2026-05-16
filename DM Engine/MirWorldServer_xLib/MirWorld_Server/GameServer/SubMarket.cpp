#include "StdAfx.h"
#include ".\submarket.h"
#include ".\itemmanager.h"
#include ".\marketmanager.h"
#include ".\humanplayer.h"

CSubMarket::CSubMarket(UINT nMarketId, UINT nId, const char* pszName)
{
	m_nMarketId = nMarketId;
	this->m_Id = nId;
	if (pszName == nullptr || *pszName == 0)
		m_pszName = "";
	else
		m_pszName = copystring(pszName);
	m_pItemArray = nullptr;
	m_nItemCount = 0;
}

CSubMarket::~CSubMarket(void)
{
	if (m_pszName && m_pszName[0] != 0)
		delete[]m_pszName;
	if (m_pItemArray)
	{
		for (UINT i = 0; i < m_nItemCount; i++)
		{
			if (m_pItemArray[i])
			{
				CMarketManager::GetInstance()->deleteItem(m_pItemArray[i]);
			}
		}
		delete[]m_pItemArray;
	}
}

BOOL CSubMarket::InitItemArray(UINT nSize)
{
	m_pItemArray = new MarketItem * [nSize];
	m_nItemCount = 0;
	if (m_pItemArray)return TRUE;
	return FALSE;
}

BOOL CSubMarket::AddItem(UINT nImage, UINT nShowImage, const char* pszViewName, const char* pszItemName, UINT nPrice, UINT nCount, BOOL bBind, const char* pszTips)
{
	ITEM itemTemplate;
	if (!CItemManager::GetInstance()->MakeupTemplateItem(pszItemName, itemTemplate))
	{
		PRINT(ERROR_RED, "商城中发现不存在的物品 %s \n", pszItemName);
		return FALSE;
	}
	MarketItem* pItem = CMarketManager::GetInstance()->newItem();
	if (pItem)
	{
		pItem->nMarketId = this->m_nMarketId;
		pItem->nSubMarketId = this->m_Id;
		pItem->itemTemplate = itemTemplate;
		pItem->nImage = nImage;
		pItem->nSellCount = 0;
		pItem->nShowImage = nShowImage;
		pItem->pszTips = ProcItemTips(pszTips);
		o_strncpy(pItem->szItemName, pszItemName, 60);
		o_strncpy(pItem->szName, pszViewName, 60);
		pItem->wCount = nCount;
		pItem->wPrice = nPrice;
		pItem->bBind = bBind;
		this->m_pItemArray[this->m_nItemCount++] = pItem;
		return TRUE;
	}
	return FALSE;
}

static char g_szTempString[65536];
char* CSubMarket::ProcItemTips(const char* pszTips)
{
	UINT nWidth = 0;
	UINT nOutPutPtr = 0;
	char* p = (char*)pszTips;
	BOOL bHz = FALSE;
	while (*p)
	{
		if (bHz)
			bHz = FALSE;
		else
		{
			if (*p < 0)
				bHz = TRUE;
		}

		g_szTempString[nOutPutPtr++] = *p;
		nWidth++;
		if (nWidth == 20)
		{
			if (bHz)
			{
				g_szTempString[nOutPutPtr - 1] = '\\';
				g_szTempString[nOutPutPtr++] = '\\';
				g_szTempString[nOutPutPtr++] = *p;
			}
			else
			{
				g_szTempString[nOutPutPtr++] = '\\';
				g_szTempString[nOutPutPtr++] = '\\';
			}
			nWidth = 0;
		}
		p++;
	}
	g_szTempString[nOutPutPtr] = 0;
	return copystring(g_szTempString);
}

BOOL CSubMarket::LoadSubMarket(const char* pszFilename)
{
	CStringFile sf;
	if (!sf.LoadFile(pszFilename))
		return TRUE;
	if (!InitItemArray(sf.GetLineCount()))
		return TRUE;
	for (UINT i = 0; i < (UINT)sf.GetLineCount(); i++)
	{
		char* pLine = TrimEx(sf[i]);
		if (*pLine == '#' || *pLine == 0)continue;
		xStringsExtracter<32> itemdef(pLine, "|", " \t");
		if (itemdef.getCount() < 8)continue;
		AddItem((UINT)StringToInteger(itemdef[0]),
			(UINT)StringToInteger(itemdef[1]),
			itemdef[2],
			itemdef[6],
			(UINT)StringToInteger(itemdef[3]),
			(UINT)StringToInteger(itemdef[4]),
			(BOOL)StringToInteger(itemdef[5]),
			itemdef[7]);
	}
	return TRUE;
}

VOID CSubMarket::QueryItems(CHumanPlayer* pPlayer, UINT nMarketId)
{
	xPacket packet(g_szTempString, 65536);
	//1268|827|00109|双倍经验晚间卡（天）|4|1
	sprintf((char*)packet.getfreebuf(), "%02u%02u&", nMarketId, GetId());
	packet.addsize((int)strlen(packet.getfreebuf()));
	UINT nItemId = 0;
	for (UINT i = 0; i < this->m_nItemCount; i++)
	{
		if (this->m_pItemArray[i])
		{
			sprintf((char*)packet.getfreebuf(), "%u|%u|%05u|%s|%u|%u&",
				this->m_pItemArray[i]->nId,
				this->m_pItemArray[i]->nImage,
				this->m_pItemArray[i]->nShowImage,
				this->m_pItemArray[i]->szName,
				this->m_pItemArray[i]->wPrice,
				this->m_pItemArray[i]->wCount);
			if (nItemId == 0)nItemId = this->m_pItemArray[i]->nId;
			packet.addsize((int)strlen(packet.getfreebuf()));
		}
	}
	pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 3, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	if (nItemId != 0)
		CMarketManager::GetInstance()->QueryItemTips(pPlayer, nItemId);
}