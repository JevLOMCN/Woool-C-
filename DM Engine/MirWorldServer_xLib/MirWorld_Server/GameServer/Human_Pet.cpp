#include "stdafx.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "SystemScript.h"

BOOL CHumanPlayer::SetPetBagSize(UINT nSize)
{
	if (nSize == 0)
	{
		this->m_ItemPetBag.SetCountLimit(0);
		this->SendSetPetBag(0);
		this->SendPetBag(nullptr, 0);
		return TRUE;
	}
	if (nSize != 5 && nSize != 10)return FALSE;
	this->m_ItemPetBag.SetCountLimit(nSize);
	ITEMCLIENT items[20];
	int itemsize = m_ItemPetBag.GetClientItems(items, nSize);
	SendSetPetBag(nSize);
	SendPetBag(items, itemsize);
	return TRUE;
}

BOOL CHumanPlayer::GetItemFromPetBag(DWORD dwMakeIndex)
{
	ITEM item;
	ITEM* pItem = m_ItemPetBag.FindItem(dwMakeIndex);
	if (pItem != nullptr)
	{
		item = *pItem;
		if (m_ItemBox.AddItem(item))
		{
			SendMsg(dwMakeIndex, 0x9605, 0, 0, 1);
			m_ItemPetBag.RemoveItem(dwMakeIndex);
			SendMsg(GetId(), SM_ADDBAGITEM, 0, 0, 1, &item, sizeof(ITEMCLIENT));
			CItemManager::GetInstance()->UpdateItemPos(dwMakeIndex, IDF_BAG, 0);
			SendMsg(dwMakeIndex, 0x2c1, 0, 0, 1);
			SendWeightChanged();
			return TRUE;
		}
	}
	SendMsg(dwMakeIndex, 0x9605, 0, 0, 0);
	return FALSE;
}

BOOL CHumanPlayer::PutItemToPetBag(DWORD dwMakeIndex)
{
	ITEM item;
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	if (pItem != nullptr)
	{
		item = *pItem;
		if (m_ItemPetBag.AddItem(item))
		{
			m_ItemBox.RemoveItem(dwMakeIndex);
			SendMsg(dwMakeIndex, 0x9604, 0, 0, 1);
			CItemManager::GetInstance()->UpdateItemPos(item.dwMakeIndex, IDF_PETBANK, 0);
			SendMsg(GetId(), 0x9603, 0, 0, 1, &item, sizeof(ITEMCLIENT));
			SendWeightChanged();
			return TRUE;
		}
	}
	SendMsg(dwMakeIndex, 0x9604, 0, 0, 0);
	return FALSE;
}

VOID CHumanPlayer::SendPetName(ITEM* pItem)
{
	char szTempBuffer[32]{};
	xPacket packet(szTempBuffer, 31);
	packet.push((LPVOID)&pItem->dwMakeIndex, 4);
	packet.push(pItem->GetExName());
	packet.push(1);
	SendMsg(GetId(), 206, pItem->dwMakeIndex, 0, 1, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CHumanPlayer::SendOutPetInfo(ITEM* pItem, BYTE Type)
{
	WORD dwCurHp = 100;
	WORD dwMaxHp = 100;
	BYTE byHPPercent = 0; // 宠物血条百分比
	if (Type == 1 && m_pPet) // 豹子类
	{
		dwCurHp = m_pPet->GetPropValue(PI_CURHP);
		dwMaxHp = m_pPet->GetPropValue(PI_MAXHP);
	}
	else // 骑乘类 马
	{
		dwCurHp = m_pHorse->GetPropValue(PI_CURHP);
		dwMaxHp = m_pHorse->GetPropValue(PI_MAXHP);
	}
	char szTempBuffer[128]{};
	xPacket packet(szTempBuffer, 127);
	DWORD nValue = pItem->dwMakeIndex;
	packet.push((LPVOID)&nValue, 4);
	packet.push(20);
	nValue = pItem->GetPetTime(); // 最后喂养时间
	packet.push((LPVOID)&nValue, 4);
	nValue = 4;
	packet.push((LPVOID)&nValue, 1);
	packet.push(8);
	nValue = (dwCurHp / dwMaxHp) * 100; // 血条百分比
	byHPPercent = nValue;
	packet.push((LPVOID)&nValue, 1);
	packet.push(16);
	nValue = Type; // 0未释放、1释放出来跟随、2本体骑乘、3摆摊、4下马、5改变颜色、6本体骑战、100特殊状态
	packet.push((LPVOID)&nValue, 1);
	packet.push((LPVOID)&dwCurHp, 2); // 当前生命值
	packet.push((LPVOID)&dwMaxHp, 2); // 最大生命值
	SendMsg(0, 0x88a1, MAKEWORD(byHPPercent, Type), 1, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CHumanPlayer::PetEatItem(DWORD nPetItemIdx, DWORD nItemIdx)
{
	CItemBox& box = GetBag();
	ITEM* item2 = box.FindItem(nItemIdx); // 豹粮
	m_pUsingItem = item2; // 使用的物品
	ITEMCLASS* pClass = CItemManager::GetInstance()->GetItemClassByName(item2->baseitem.szName);
	if (pClass && pClass->wPageId != 0)
	{
		const char* pszPage = CItemManager::GetInstance()->GetStringFromPool(pClass->wPageId);
		if (pszPage[0] != 0)
		{
			DWORD saveindex = item2->dwMakeIndex;
			item2->dwParam[3] = UR_NORESULT;
			CSystemScript::GetInstance()->Execute(this->GetScriptTarget(), pszPage, FALSE);
			if (item2->dwParam[3] == UR_DELETED)
			{
				DeleteBagItem(saveindex);
				CItemManager::GetInstance()->DeleteItem(saveindex);
				SendEatOk();
			}
			else
			{
				SendEatFail();
				if (item2->dwParam[3] == UR_UPDATED)
					SendUpdateItem(*item2);
			}
		}
	}
}