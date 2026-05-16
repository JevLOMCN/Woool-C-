#include "StdAfx.h"
#include ".\exchangeobj.h"
#include "humanplayer.h"
#include ".\exchangeobjectmgr.h"

CExchangeObj::CExchangeObj(void) :m_State(EE_PUTITEMS)
{
	m_pErrorMsg = "交易成功";
	m_State = EE_PUTITEMS;
	memset(m_Sides, 0, sizeof(m_Sides));
	m_boFastExchange = FALSE;
}

CExchangeObj::~CExchangeObj(void)
{
}

BOOL CExchangeObj::Begin(CHumanPlayer* p1, CHumanPlayer* p2)
{
	if (p1 == nullptr || p2 == nullptr)return FALSE;
	memset(m_Sides, 0, sizeof(m_Sides));
	m_Sides[0].player = p1;
	m_Sides[1].player = p2;
	m_State = EE_PUTITEMS;

	p1->SendMsg(0, SM_TRADESTART, 0, 0, 0, (LPVOID)p2->GetName());
	p2->SendMsg(0, SM_TRADESTART, 0, 0, 0, (LPVOID)p1->GetName());

	p1->SetExchangeObject(this);
	p1->SaySystemAttrib(CC_EXCHANGE, "%s和你开始交易了", p2->GetName());
	p2->SetExchangeObject(this);
	p2->SaySystemAttrib(CC_EXCHANGE, "%s和你开始交易了", p1->GetName());
	return TRUE;
}

BOOL CExchangeObj::PutItem(CHumanPlayer* p, ITEM& item)
{
	exchange_side* pSide = GetSide(p), * pOtherSide = GetOtherSide(p);
	int i = 0;
	if (m_State != EE_PUTITEMS)
	{
		m_pErrorMsg = "无法放入物品, 对方已经按下交易按钮!";
		p->SendMsg(0, 0x2a4, 0, 0, 0);
		return FALSE;
	}
	for (int i = 0; i < 10; i++)
	{
		if (pSide->m_Items[i].dwMakeIndex == 0)
		{
			pSide->m_Items[i] = item;
			break;
		}
	}
	if (i == 10)
	{
		m_pErrorMsg = "交易栏已满, 无法放入新物品!";
		p->SendMsg(0, 0x2a4, 0, 0, 0);
		return FALSE;
	}
	pOtherSide->player->Send_Exchg_OtherAddItem(pSide->player, item);
	return TRUE;
}

BOOL CExchangeObj::TakeItem(CHumanPlayer* p, DWORD dwMakeIndex, ITEM& item)
{
	return TRUE;
}

BOOL CExchangeObj::PutMoney(CHumanPlayer* p, money_type type, DWORD dwCount)
{
	exchange_side* pSide = GetSide(p);
	exchange_side* pOtherSide = GetOtherSide(p);
	if (type == MT_GOLD)
		pSide->dwGold = dwCount;
	else
		pSide->dwYuanbao = dwCount;
	pOtherSide->player->Send_Exchg_OtherAddMoney(p, type, dwCount);
	return TRUE;
}

BOOL CExchangeObj::TakeMoney(CHumanPlayer* p, money_type type, DWORD dwCount)
{
	return TRUE;
}

BOOL CExchangeObj::DoExchange(exchange_side* pActionSide, exchange_side* pOtherSide)
{
	//	check item space
	int itemcount1 = 0;
	int itemcount2 = 0;
	for (int i = 0; i < 10; i++)
	{
		if (pActionSide->m_Items[i].dwMakeIndex != 0)
			itemcount1++;
		if (pOtherSide->m_Items[i].dwMakeIndex != 0)
			itemcount2++;
	}
	if (itemcount1 > pOtherSide->player->GetBag().GetFree())
	{
		pActionSide->player->SaySystem("对方的背包无法容纳这么多物品!");
		return FALSE;
	}
	if (itemcount2 > pActionSide->player->GetBag().GetFree())
	{
		pOtherSide->player->SaySystem("对方的背包无法容纳这么多物品!");
		return FALSE;
	}
	//	check money space
	if (pActionSide->dwGold > 0)
	{
		if (!pOtherSide->player->TestAddGold(pActionSide->dwGold))
		{
			pActionSide->player->SaySystem("钱币太多, 对方拿不下!");
			return FALSE;
		}
	}
	if (pActionSide->dwYuanbao > 0)
	{
		if (!pOtherSide->player->TestAddMoney(MT_YUANBAO, pActionSide->dwYuanbao))
		{
			pActionSide->player->SaySystem("元宝太多, 对方拿不下!");
			return FALSE;
		}
	}
	if (pOtherSide->dwGold > 0)
	{
		if (!pActionSide->player->TestAddGold(pOtherSide->dwGold))
		{
			pOtherSide->player->SaySystem("钱币太多, 对方拿不下!");
			return FALSE;
		}
	}
	if (pOtherSide->dwYuanbao > 0)
	{
		if (!pActionSide->player->TestAddMoney(MT_YUANBAO, pOtherSide->dwYuanbao))
		{
			pOtherSide->player->SaySystem("元宝太多, 对方拿不下!");
			return FALSE;
		}
	}
	//	then do exchange~
	for (int i = 0; i < 10; i++)
	{
		if (pActionSide->m_Items[i].dwMakeIndex != 0)
			pOtherSide->player->AddBagItem(pActionSide->m_Items[i], FALSE, TRUE, FALSE);
		if (pOtherSide->m_Items[i].dwMakeIndex != 0)
			pActionSide->player->AddBagItem(pOtherSide->m_Items[i], FALSE, TRUE, FALSE);
	}
	pActionSide->player->SendWeightChanged();
	pOtherSide->player->SendWeightChanged();
	pActionSide->player->AddGold(pOtherSide->dwGold);
	pOtherSide->player->AddGold(pActionSide->dwGold);
	pActionSide->player->AddMoney(MT_YUANBAO, pOtherSide->dwYuanbao);
	pOtherSide->player->AddMoney(MT_YUANBAO, pActionSide->dwYuanbao);

	pActionSide->player->SendMsg(0, SM_TRADEEND, 0, 0, 0);
	pActionSide->player->SaySystemAttrib(CC_EXCHANGE, "交易成功");
	pOtherSide->player->SendMsg(0, SM_TRADEEND, 0, 0, 0);
	pOtherSide->player->SaySystemAttrib(CC_EXCHANGE, "交易成功");

	//pActionSide->player->SetExchangeObject( nullptr );
	//pOtherSide->player->SetExchangeObject( nullptr );

	return TRUE;
}

BOOL CExchangeObj::DoCancel(exchange_side* pActionSide, exchange_side* pOtherSide)
{
	for (int i = 0; i < 10; i++)
	{
		if (pActionSide->m_Items[i].dwMakeIndex != 0)
			pActionSide->player->AddBagItem(pActionSide->m_Items[i], TRUE, FALSE, FALSE);
		if (pOtherSide->m_Items[i].dwMakeIndex != 0)
			pOtherSide->player->AddBagItem(pOtherSide->m_Items[i], TRUE, FALSE, FALSE);
	}
	pActionSide->player->SendWeightChanged();
	pOtherSide->player->SendWeightChanged();
	pActionSide->player->AddGold(pActionSide->dwGold, FALSE);
	pActionSide->player->AddMoney(MT_YUANBAO, pActionSide->dwYuanbao, FALSE);
	pOtherSide->player->AddGold(pOtherSide->dwGold, FALSE);
	pOtherSide->player->AddMoney(MT_YUANBAO, pOtherSide->dwYuanbao, FALSE);

	pActionSide->player->SendMsg(0, SM_TRADECANCELED, 0, 0, 0);
	pActionSide->player->SaySystemAttrib(CC_EXCHANGE, "交易取消");
	pOtherSide->player->SendMsg(0, SM_TRADECANCELED, 0, 0, 0);
	pOtherSide->player->SaySystemAttrib(CC_EXCHANGE, "交易取消");

	return TRUE;
}

BOOL CExchangeObj::End(CHumanPlayer* p, e_endtype type)
{
	exchange_side* pside = GetSide(p);
	exchange_side* potherside = GetOtherSide(p);
	if (pside == nullptr)
	{
		m_pErrorMsg = "您现在不在交易状态!";
		return FALSE;
	}
	BOOL bTradeEnd = FALSE;
	switch (type)
	{
	case ET_CANCEL:
	{
		//	返还物品
		potherside->player->SaySystem("对方取消交易");
		DoCancel(pside, potherside);
		bTradeEnd = TRUE;
	}
	break;
	case ET_CONFIRM:
	{
		if (pside->bReady)
		{
			pside->player->SaySystemAttrib(CC_EXCHANGE, "请让对方按下交易按钮");
			potherside->player->SaySystemAttrib(CC_EXCHANGE, "对方再次要求你确认交易, 按下[交易]键确认");
		}
		else
		{
			pside->bReady = TRUE;
			if (potherside->bReady)
			{
				//	进行交换   
				if (!DoExchange(pside, potherside))
					DoCancel(pside, potherside);
				bTradeEnd = TRUE;
			}
			else
			{
				m_State = EE_WAITFOROTHER;
				pside->player->SaySystemAttrib(CC_EXCHANGE, "请让对方按下交易按钮");
				potherside->player->SaySystemAttrib(CC_EXCHANGE, "对方再次要求你确认交易, 按下[交易]键确认");
			}
		}
	}
	break;
	default:
		return FALSE;
	}
	if (bTradeEnd)
	{
		pside->player->SetExchangeObject(nullptr);
		potherside->player->SetExchangeObject(nullptr);
		CExchangeObjectMgr::GetInstance()->EndExchange(this);
	}
	return TRUE;
}