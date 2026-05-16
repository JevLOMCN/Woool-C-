#pragma once
class CMarket;
class CSubMarket;
typedef struct tagMarketItem
{
	tagMarketItem()
	{
		FILLSELF(0);
	}
	UINT nSellCount;
	UINT nId;
	UINT nImage;
	UINT nShowImage;
	char szName[64];
	char szItemName[64];
	WORD wCount;
	WORD wPrice;
	BOOL bBind;
	char* pszTips;
	ITEM itemTemplate;
	UINT nMarketId;
	UINT nSubMarketId;
}MarketItem;