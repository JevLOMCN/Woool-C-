#pragma once
class CSubMarket;
class CHumanPlayer;

class CMarket
{
public:
	CMarket(UINT id);
	virtual ~CMarket(void);
	CSubMarket* AddSubMarket(UINT nId, const char* pszName);
	CSubMarket* GetSubMarket(UINT nId);
	UINT GetId()const { return m_Id; }

	VOID SendSubMarket(CHumanPlayer* pPlayer);
protected:
	CSubMarket* m_pSubMarketArray[32];
	UINT m_nSubMarketCount;
	UINT m_Id;
};