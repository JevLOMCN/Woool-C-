#include "StdAfx.h"
#include ".\marketmanager.h"
#include ".\market.h"
#include ".\humanplayer.h"
#include ".\ItemManager.h"
#include ".\submarket.h"

CMarketManager::CMarketManager(void) : m_pMarketArray{}
{
	m_xItemList.Create(100000);
	m_pszScrollText = nullptr;
	m_nMarketCount = 0;
}

CMarketManager::~CMarketManager(void)
{
	m_xItemList.Destroy();
}

MarketItem* CMarketManager::newItem()
{
	MarketItem* pItem = nullptr;
	UINT id = m_xItemList.New(&pItem);
	if (id == 0 || pItem == nullptr)return nullptr;
	pItem->nId = id;
	return pItem;
}

VOID CMarketManager::deleteItem(MarketItem* pItem)
{
	m_xItemList.Del(pItem->nId);
}

MarketItem* CMarketManager::getItem(UINT nId)
{
	return m_xItemList.Get(nId);
}

VOID CMarketManager::LoadScrollText(const char* pszFilename)
{
	if (m_pszScrollText)
	{
		delete[]m_pszScrollText;
		m_pszScrollText = nullptr;
	}

	this->m_pszScrollText = (char*)LoadFile(pszFilename);
	if (m_pszScrollText == nullptr)
		m_pszScrollText = "欢迎光临\\\\精彩等着您";
	else
	{
		UINT nLength = (UINT)strlen(m_pszScrollText);
		for (UINT i = 0; i < nLength; i++)
		{
			if (m_pszScrollText[i] == '\n' ||
				m_pszScrollText[i] == '\r')
				m_pszScrollText[i] = '\\';
		}
	}
}

CMarket* CMarketManager::AddMarket(UINT nId)
{
	if (this->m_nMarketCount >= 32)
		return nullptr;
	this->m_pMarketArray[this->m_nMarketCount] = new CMarket(nId);
	return this->m_pMarketArray[this->m_nMarketCount++];
}

CMarket* CMarketManager::GetMarket(UINT nMarketId)
{
	for (UINT n = 0; n < this->m_nMarketCount; n++)
	{
		if (this->m_pMarketArray[n] && this->m_pMarketArray[n]->GetId() == nMarketId)
			return this->m_pMarketArray[n];
	}
	return nullptr;
}

inline static UINT GetIdFromPageName(const char* pszPageName)
{
	if (strcmp("首页", pszPageName) == 0)
		return 10;
	if (strcmp("喜庆", pszPageName) == 0)
		return 20;
	if (strcmp("百变", pszPageName) == 0)
		return 30;
	if (strcmp("礼包", pszPageName) == 0)
		return 40;
	if (strcmp("其他", pszPageName) == 0)
		return 50;
	return 60;
}

BOOL CMarketManager::LoadMarkets(const char* pszFilename)
{
	CStringFile sf;
	if (!sf.LoadFile(pszFilename))
		return FALSE;
	if (m_nMarketCount > 0)
	{
		for (UINT i = 0; i < m_nMarketCount; i++)
		{
			if (this->m_pMarketArray[i])
				delete this->m_pMarketArray[i];
		}
		m_nMarketCount = 0;
	}
	for (UINT i = 0; i < (UINT)sf.GetLineCount(); i++)
	{
		char* pLine = TrimEx(sf[i]);
		if (*pLine == '#' || *pLine == 0)continue;
		xStringsExtracter<2> equ(pLine, "=", " \t");
		if (equ.getCount() < 2)continue;
		UINT id = GetIdFromPageName(equ[0]);
		CMarket* pMarket = AddMarket(id);
		if (pMarket == nullptr)continue;
		xStringsExtracter<32> submarket(equ[1], "&", " \t");
		for (UINT j = 0; j < submarket.getCount(); j++)
		{
			if (*submarket[j] == 0)continue;
			xStringsExtracter<2> addsub(submarket[j], "|", " \t");
			if (addsub.getCount() < 2)continue;
			CSubMarket* pSubMarket = pMarket->AddSubMarket((UINT)StringToInteger(addsub[0]), addsub[1]);
		}
	}
	return TRUE;
}

static thread_local char g_szTempString[65536];
VOID CMarketManager::OpenMarket(CHumanPlayer* pPlayer)
{
	xPacket packet(g_szTempString, 65535);
	UINT id = pPlayer->GetId();
	//	first send scrolltexts
	pPlayer->SendMsg(id, 0x1000, 0, 0, 0, (LPVOID)this->GetMarketScrollText());
	//	second send toplist
	packet.push((LPVOID)"&", 1);
	for (UINT i = 0; i < 5; i++)
	{
		MarketItem* pItem = getItem(i + 1);
		if (pItem)
		{
			//&50101266|825|100|灵符(包)|5
			sprintf((char*)packet.getfreebuf(), "%u%u%u|%u|100|%s|%u&",
				pItem->nMarketId, pItem->nSubMarketId, pItem->nId, pItem->nImage, pItem->szName, pItem->wPrice);
			packet.addsize((int)strlen(packet.getfreebuf()));
		}
	}
	pPlayer->SendMsg(id, 0x1000, 1, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	if (this->m_nMarketCount > 0 && this->m_pMarketArray[0])
		QueryMarket(pPlayer, m_pMarketArray[0]->GetId());
}

VOID CMarketManager::QueryMarket(CHumanPlayer* pPlayer, UINT nMarketId)
{
	CMarket* pMarket = GetMarket(nMarketId);
	if (pMarket)
	{
		pMarket->SendSubMarket(pPlayer);
	}
	else
	{
		char szError[200];
		sprintf(szError, "%u该商城道具不存在", nMarketId);
		pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 2, 1, 0, (LPVOID)szError);
	}
}

VOID CMarketManager::QueryItemTips(CHumanPlayer* pPlayer, UINT nItemId)
{
	MarketItem* pItem = getItem(nItemId);
	xPacket packet(g_szTempString, 65535);
	if (pItem)
	{
		sprintf((char*)packet.getbuf(), "%u&", pItem->nId);
		packet.addsize((int)strlen(packet.getbuf()));
		packet.push((LPVOID)pItem->pszTips, (int)strlen(pItem->pszTips));
		pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 4, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	}
	else
	{
		pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 4, 1, 0, (LPVOID)"该商城道具不存在");
	}
}

VOID CMarketManager::QueryBuyItem(CHumanPlayer* pPlayer, UINT nItemId)
{
	MarketItem* pItem = getItem(nItemId);
	if (pItem)
	{
		if (pItem->wPrice > pPlayer->GetMoney(MT_YUANBAO))
		{
			pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 5, 0x1b, 0, (LPVOID)"你身上的元宝不够!");
			return;
		}
		if ((int)pItem->wCount > pPlayer->GetBag().GetFree())
		{
			pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 5, 0x1b, 0, (LPVOID)"你的背包没有足够空间!");
			return;
		}
		if (!pPlayer->CostMoney(MT_YUANBAO, pItem->wPrice))
		{
			pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 5, 0x1b, 0, (LPVOID)"你身上的元宝不够!");
			return;
		}
		for (UINT i = 0; i < pItem->wCount; i++)
		{
			ITEM item = pItem->itemTemplate;
			CItemManager::GetInstance()->IdentItem(item);
			item.SetBind(pItem->bBind); // 商城购买物品属于绑定物品
			pPlayer->AddBagItem(item, 0, 0, 0);
		}
		pPlayer->SendWeightChanged();
		pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 5, 0, 0, (LPVOID)"恭喜您购买成功");
	}
	else
	{
		pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 5, 0x1b, 0, (LPVOID)"该商城道具不存在");
	}
}

VOID CMarketManager::OnClientMsg(CHumanPlayer* pPlayer, WORD wCmd, WORD wParam1, WORD wParam2, char* pszData, int iDatasize)
{
	if (pPlayer == nullptr)return;
	switch (wCmd)
	{
	case 5: //商城-购买物品 3&2
	{
		int iItemId = 0;
		int iCount = 1;
		std::string str = pszData;
		if (str.find('&') != std::string::npos)
		{
			char* Params[2];
			int nParam = SearchParam(pszData, Params, 2, '&');
			iItemId = StringToInteger(Params[0]);
			iCount = StringToInteger(Params[1]);
		}
		else
			iItemId = StringToInteger(pszData);
		for (int i = 0; i < iCount; i++)
		{
			QueryBuyItem(pPlayer, iItemId);
		}
	}
	break;
	case 1: // 商城-打开
	{
		OpenMarket(pPlayer);
	}
	break;
	case 2:
	{
		UINT nId = (UINT)StringToInteger(pszData);
		QueryMarket(pPlayer, nId);
	}
	break;
	case 4:
	{
		UINT nId = (UINT)StringToInteger(pszData);
		QueryItemTips(pPlayer, nId);
	}
	break;
	case 3:
	{
		UINT nId = (UINT)StringToInteger(pszData);
		UINT nMarketId = nId / 100;
		UINT nSubMarketId = nId % 100;
		CMarket* pMarket = GetMarket(nMarketId);
		if (pMarket)
		{
			CSubMarket* pSubMarket = pMarket->GetSubMarket(nSubMarketId);
			if (pSubMarket)
			{
				pSubMarket->QueryItems(pPlayer, nMarketId);
				break;
			}
		}
		pPlayer->SendMsg(pPlayer->GetId(), 0x1000, 3, 1, 0, (LPVOID)"该商城道具不存在");
	}
	break;
	}
}