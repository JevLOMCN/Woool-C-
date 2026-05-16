#pragma once
class CHumanPlayer;

enum e_endtype
{
	ET_CANCEL,
	ET_CONFIRM,
};

enum e_exchangestate
{
	EE_PUTITEMS,
	EE_WAITFOROTHER,
	EE_MAX,
};

struct exchange_side
{
	CHumanPlayer* player = nullptr;
	ITEM m_Items[10] = {};
	DWORD dwGold = 0;
	DWORD dwYuanbao = 0;
	BOOL bReady = FALSE;
};

class CExchangeObj
{
public:
	CExchangeObj();
	virtual ~CExchangeObj(void);
	//- Begin( CPlayer * player1, CPlayer * player2 )
	BOOL Begin(CHumanPlayer* p1, CHumanPlayer* p2);
	//- PutItem( player, ITEM & item )
	BOOL PutItem(CHumanPlayer* p, ITEM& item);
	//- TakeItem( player, ITEM & item );
	BOOL TakeItem(CHumanPlayer* p, DWORD dwMakeIndex, ITEM& item);
	//- PutMoney( player, count, type );
	BOOL PutMoney(CHumanPlayer* p, money_type type, DWORD dwCount);
	//- TakeMoney( player, count, type );
	BOOL TakeMoney(CHumanPlayer* p, money_type type, DWORD dwCount);
	//- End(type)
	BOOL End(CHumanPlayer* p, e_endtype type);
public:
	const char* GetErrorMsg() { return m_pErrorMsg; }
	e_exchangestate GetState()const { return m_State; }
	CHumanPlayer* GetOtherSidePlayer(CHumanPlayer* player) { const exchange_side* p = GetOtherSide(player);if (p)return p->player; return nullptr; }
	DWORD GetGold(CHumanPlayer* pPlayer, money_type type)
	{
		const exchange_side* p = GetSide(pPlayer);
		if (p)
			return type == MT_GOLD ? p->dwGold : p->dwYuanbao;
		return 0;
	}
	//是否是快速交易模式
	BOOL GetFastExchange() const { return m_boFastExchange; }
	//设置是否是快速交易
	VOID SetFastExchange(BOOL boFastExchaneg) { m_boFastExchange = boFastExchaneg; }
private:
	exchange_side* GetSide(CHumanPlayer* p)
	{
		if (m_Sides->player == p)return m_Sides;
		if ((m_Sides + 1)->player == p)return (m_Sides + 1);
		return nullptr;
	}
	exchange_side* GetOtherSide(CHumanPlayer* p)
	{
		if (m_Sides->player == p)return (m_Sides + 1);
		if ((m_Sides + 1)->player == p)return m_Sides;
		return nullptr;
	}

	BOOL DoExchange(exchange_side* pActionSide, exchange_side* pOtherSide);
	BOOL DoCancel(exchange_side* pActionSide, exchange_side* pOtherSide);
	BOOL m_boFastExchange; //是否是快速交易, 就是用头像框右键开始交易
	exchange_side m_Sides[2];
	char* m_pErrorMsg;
	e_exchangestate m_State;
};