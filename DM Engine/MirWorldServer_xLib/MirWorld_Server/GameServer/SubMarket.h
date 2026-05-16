#pragma once
class CHumanPlayer;

class CSubMarket
{
public:
	CSubMarket(UINT nMarketId, UINT nId, const char* pszName);
	virtual ~CSubMarket(void);
	UINT GetId()const{return m_Id;}
	const char* GetName(){return m_pszName;}
	BOOL LoadSubMarket(const char* pszFilename);
	BOOL InitItemArray(UINT nSize);
	BOOL AddItem(UINT nImage, UINT nShowImage, const char* pszViewName, const char* pszItemName, UINT nPrice, UINT nCount, BOOL bBind, const char* pszTips);
	VOID QueryItems(CHumanPlayer* pPlayer, UINT nMarketId);
protected:
	char* ProcItemTips(const char* pszTips);
	UINT m_Id;
	char* m_pszName;
	MarketItem** m_pItemArray;
	UINT m_nItemCount;
	UINT m_nMarketId;
};