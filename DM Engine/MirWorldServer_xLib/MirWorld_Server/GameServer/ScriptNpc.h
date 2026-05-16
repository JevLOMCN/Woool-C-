#pragma once
#include "aliveobject.h"
#include "scriptshell.h"
class CScriptPage;
class CHumanPlayer;
struct tagGoods;

class CScriptNpc :
	public CAliveObject,
	public CScriptShell
{
public:
	CScriptNpc(void);
	virtual ~CScriptNpc(void);
	BOOL Init(UINT dbid, const char* pszName, int view, int x, int y, DWORD mapid, CScriptObject* pScriptObject);
	VOID QueryTalk(CHumanPlayer* pPlayer);
	VOID QuerySelectLink(CHumanPlayer* pPlayer, const char* pLink, BOOL bHumanQuery = TRUE);

	//VOID QueryGoodsItemList(CHumanPlayer* pPlayer, const char* pTemplate);
	bool containschar(const std::string& str);

	e_object_type GetType() { return OBJ_NPC; }
	e_class_type GetClassType() { return CLS_NPC; }
	DWORD GetFeather() {
		return MAKEFEATHER(0, m_nView, 0, 0x32);
	}
	DWORD GetHealth() {
		return 0x00640064;
	}
	const char* GetName() { return m_szName; }
	const char* GetViewName() { return m_szLongName; }
	BOOL IsNPC()const { return m_bIsNpc; }
	BOOL CanMove() { return FALSE; }
	BOOL AddItem(ITEM& item);
	VOID Update();
	BOOL BeAttack(CAliveObject* pAttacker, int nDamage);

	DWORD GetItemSellPrice(ITEM& item);
	DWORD GetItemBuyPrice(ITEM& item);
	DWORD GetItemRepairPrice(CHumanPlayer* pPlayer, ITEM& item);
	BOOL RepairItem(CHumanPlayer* pPlayer, ITEM& item);
	BOOL SellItem(CHumanPlayer* pPlayer, ITEM& item);
	BOOL BuyItem(CHumanPlayer* pPlayer, const char* pszName, DWORD dwMakeIndex, DWORD& dwErrorCode);
	BOOL QueryItemList(CHumanPlayer* pPlayer, const char* pszItemName, int ptr);
	VOID SetTalk(BOOL isTalk){ m_bTalk = isTalk; }
	VOID SetDistance(UINT nDistance) { m_nDistance = nDistance; }
	UINT GetDistance()const { return m_nDistance; }
	VOID SetBuyPercent(FLOAT fPercent)
	{
		m_fBuyPercent = fPercent;
	}
	FLOAT GetBuyPercent()const { return m_fBuyPercent; }

	VOID SetSellPercent(FLOAT fPercent)
	{
		m_fSellPercent = fPercent;
	}
	FLOAT GetSellPercent()const { return m_fSellPercent; }
	DWORD GetItemValue(ITEM& item, DWORD dwBasePrice);

	VOID SaveItems();
	VOID LoadItems();

	VOID SetLongName(const char* szLongName)
	{
		o_strncpy(m_szLongName, szLongName, 127);
	}

	VOID OnEnterMap(CLogicMap* pMap);

	VOID SetView(int nView) { this->m_nView = nView; }
	int GetView()const { return m_nView; }
	const char* GetTitleName() { return GetName(); }
	UINT GetTitleId() { return GetId(); }
	UINT GetStoreId()const { return m_StoreId; }
	VOID Clean();
	//CScriptPage * GetPage(){ return m_pPage;}
	BOOL IsSandCityMerchant()const { return m_fSandCityMerchant; }

	VOID SendMerChantJsonMsg(CScriptTarget* pTarget, const char* pWords, UINT nType);
	VOID SendDayExpMain(CHumanPlayer* pPlayer, const char* pWords);
	VOID SendDayExpHelp(CHumanPlayer* pPlayer, const char* pWords);
	VOID SendActivityMain(CHumanPlayer* pPlayer, const char* pWords);
	VOID SendCreateGuildHelp(CHumanPlayer* pPlayer, const char* pWords);
	VOID SendCustomUIWnd(CHumanPlayer* pPlayer, const char* pWords);
protected:
	BOOL OnPageShow(CScriptTarget* pTarget, CScriptView* pView, const char* pszPage);
	NpcGoodsList* FindGoodsList(ITEM& item);
	NpcGoodsList* FindGoodsList(const char* pszName);
	int ProcPageVars(CHumanPlayer* pPlayer, char* pszPage, int size, int maxsize);
	VOID SendClosePage(CHumanPlayer* pPlayer);
	VOID SendGoodsList(CHumanPlayer* pPlayer);
	VOID SendGoodsItemList(CHumanPlayer* pPlayer, const char* pTemplate, int ptr);

	VOID DeleteNpcGoodsList(NpcGoodsList* pList);
	VOID DeleteNpcGoodsItemList(NpcGoodsList* pList, NpcGoodsItemList* pItemList);
	BOOL InitGoods(tagGoods* pGoodsList);

	char m_szName[32];
	char m_szViewName[128];
	int	m_nView;
	UINT m_StoreId;
	NpcGoodsList* m_pSellGoodsList;
	BYTE m_btSellList[100];
	int	m_iSellListCount;
	float m_fSellPercent;
	float m_fBuyPercent;
	BOOL m_fChanged;
	CServerTimer m_tmrUpdateItem;
	DWORD m_dwTimeOut;
	BOOL m_fSandCityMerchant;
	BOOL m_bTalk;
	BOOL m_bIsNpc;
	UINT m_nDistance;
};