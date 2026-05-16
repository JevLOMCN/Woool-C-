#include "stdafx.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "downitemmgr.h"
#include "logicmap.h"

BOOL CHumanPlayer::InSafeArea()
{
	if (CMapObject::InSafeArea())
	{
		if (this->IsStatusSet(SI_ITEMTRACED))
			return FALSE;
		return TRUE;
	}
	return FALSE;
}

VOID CHumanPlayer::OnPickupItem(ITEM& item, UINT x, UINT y)
{
	if (CItemManager::GetInstance()->ItemLimited(item, IL_TRACEDITEM))
		SetStatus(SI_ITEMTRACED, item.dwMakeIndex, -1);
}

VOID CHumanPlayer::OnDropItem(ITEM& item, UINT x, UINT y)
{
	if (CItemManager::GetInstance()->ItemLimited(item, IL_TRACEDITEM))
		ClrStatus(SI_ITEMTRACED);
}

VOID CHumanPlayer::PetsFlyto(CLogicMap* pToMap, UINT nToX, UINT nToY, BOOL IsBlock)
{
	int x = nToX;
	int y = nToY;
	int nPetCount = 0;
	if (!IsBlock) //如果IsBlock为FALSE 时, 就是宝宝飞到玩家身后1格.
	{
		int dir = GETBACKDIR(GetDirection());
		GETNEXTNEXTPOS(x, y, dir);
	}
	if (m_pHorse && !m_pHorse->IsDeath()) // 马飞
	{
		m_pHorse->FlyTo(pToMap, x, y);
	}
	if (m_pPet && !m_pPet->IsDeath()) // 豹子飞
	{
		m_pPet->SetTarget(nullptr);
		m_pPet->SetCurFocusItem();
		m_pPet->FlyTo(pToMap, x, y);
		nPetCount++;
	}
	if (IsPetsActive() && m_iPetCount > 0) // 宝宝飞
	{
		for (int i = 0; i < m_iPetCount; i++)
		{
			CMonsterEx* monster = m_pPets[i];
			if (monster == nullptr || monster->IsDeath()) continue;
			monster->SetTarget(nullptr);
			monster->FlyTo(pToMap, x, y);
			nPetCount++;
		}
	}
	if (nPetCount > 2 && IsBlock && pToMap->IsBlocked(nToX, nToY))//判断移动点是否锁定,如果锁定玩家后退1步
	{
		POINT pt;
		if (pToMap->GetValidPoint(nToX, nToY, &pt, 1))
		{
			int dir = GetFlyDirection(pt.x, pt.y, nToX, nToY);
			SetDirection((e_direction)dir);
			ResetPos(pt.x, pt.y);
		}
		else
			return;
	}
}

VOID CHumanPlayer::OnChangeMap(CLogicMap* pFromMap, UINT nFromX, UINT nFromY, CLogicMap* pToMap, UINT nToX, UINT nToY)
{
	if (pFromMap != pToMap)
	{
		if (IsStatusSet(SI_ITEMTRACED))
		{
			DWORD dwMakeIndex = GetStatusParam(SI_ITEMTRACED);
			ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
			ITEM item = *pItem;
			if (pItem)
			{
				this->DeleteBagItem(dwMakeIndex);
				SendTakeBagItem(&item);
				CDownItemMgr::GetInstance()->DropItem(pFromMap, item, nFromX, nFromY, FALSE);
			}
			ClrStatus(SI_ITEMTRACED);
		}
	}
	PetsFlyto(pToMap, nToX, nToY);  // 宠物飞
}