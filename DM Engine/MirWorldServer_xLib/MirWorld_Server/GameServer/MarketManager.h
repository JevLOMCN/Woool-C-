#pragma once
class CMarket;
class CHumanPlayer;

class CMarketManager :
	public xSingletonClass<CMarketManager>
{
public:
	CMarketManager(void);
	virtual ~CMarketManager(void);

	const char* GetMarketScrollText() { return m_pszScrollText; }

	MarketItem* getItem(UINT nId);
	VOID LoadScrollText(const char* pszFilename);

	BOOL LoadMarkets(const char* pszFilename);

	CMarket* AddMarket(UINT nId);
	CMarket* GetMarket(UINT nMarketId);
	MarketItem* newItem();
	VOID deleteItem(MarketItem* pItem);

	VOID OpenMarket(CHumanPlayer* pPlayer);
	VOID QueryMarket(CHumanPlayer* pPlayer, UINT nMarketId);
	VOID QueryItemTips(CHumanPlayer* pPlayer, UINT nItemId);

	VOID QueryBuyItem(CHumanPlayer* pPlayer, UINT nItemId);

	VOID OnClientMsg(CHumanPlayer* pPlayer, WORD wCmd, WORD wParam1, WORD wParam2, char* pszData, int iDatasize);
protected:
	CIndexListEx<MarketItem> m_xItemList;
	char* m_pszScrollText;

	CMarket* m_pMarketArray[32];
	UINT m_nMarketCount;
};