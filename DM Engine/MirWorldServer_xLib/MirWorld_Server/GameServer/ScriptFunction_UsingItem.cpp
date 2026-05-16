#include "StdAfx.h"
#include ".\scriptfunction.h"
#include ".\cmdproc.h"
#include "time.h"
#include ".\scriptview.h"
#include ".\scripttarget.h"
#include ".\scriptshell.h"
#include ".\Humanplayer.h"
#include ".\ScriptElement.h"
#include ".\ScriptObject.h"
#include ".\ItemManager.h"
#include "logicmap.h"
#include "damageevent.h"
#include "eventmanager.h"
#include "gameworld.h"
#include "logicmapmgr.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否变身
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISTRANSFORMED)
{
	return pPlayer->IsSystemFlagSeted(SF_TRANSFORMED);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：删除
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_DELETE)
{
	ITEM* pItem = pPlayer->GetUsingItem();
	if (pItem)
	{
		pItem->dwParam[3] = UR_DELETED;
		return 1;
	}
	return 0;
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：减少持久
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_DAMAGEDURA)
{
	if (nParam == 0)return 0;
	ITEM* pItem = pPlayer->GetUsingItem();
	if (pItem && pItem->wCurDura > 0)
	{
		if (pItem->wCurDura >= Params[0].nParam)
		{
			pItem->wCurDura -= Params[0].nParam;
			if (pItem->wCurDura == 0)
				pItem->dwParam[3] = UR_DELETED;
			else
				pItem->dwParam[3] = UR_UPDATED;
			CItemManager::GetInstance()->AddItemModifyFlag(*pItem, ITEMMODIFY_DURACHANGED);
			return 1;
		}
	}
	return 0;
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查和更新时间
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_CHECKANDUPTODATE)
{
	ITEM* pItem = pPlayer->GetUsingItem();
	if (pItem)
	{
		DWORD& dwItemTime = *(DWORD*)&pItem->baseitem.btMinDef;
		if (dwItemTime == 0)	//	没有开始使用
		{
			time_t t;
			time(&t);
			dwItemTime = (DWORD)t;
			CItemManager::GetInstance()->AddItemModifyFlag(*pItem, ITEMMODIFY_DURACHANGED);
			pItem->dwParam[3] = UR_UPDATED;
			return 1;			//	开使使用~~
		}
		else
		{
			time_t t;
			time(&t);
			DWORD dwT2 = (DWORD)t;
			DWORD dwT3 = pItem->wCurDura * 86400;
			if (dwT2 > dwItemTime)
			{
				if (dwT2 - dwItemTime >= dwT3)
				{
					pItem->dwParam[3] = UR_DELETED;
					return 3;
					//DWORD saveindex = pItem->dwMakeIndex;
					//if( pPlayer->DeleteBagItem( pItem->dwMakeIndex ) )
					//{
					//	CItemManager::GetInstance()->DeleteItem(saveindex);
					//	return 3;	//	超时, 已经删除
					//}
				}
			}
			//pPlayer->SendEatFail();
			return 2;	//	正常, 没有超时, 也不是第一次
		}
	}
	//pPlayer->SendEatFail();
	return 0;
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：升级豹魔石物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_UPGRADE)
{
	if (nParam < 1)
		return 0;
	ITEM* pItem = nullptr;
	if ((pItem = pPlayer->GetUsingItem()) == nullptr)
		return 0;

	//pItem->dwParam[3] == UR_DELETED;

	pPlayer->SetUsItem(UR_DELETED);
	ITEM* pUsItem = nullptr;

	if (!pPlayer->IsHasPet())//没有召唤豹子提示
	{
		pPlayer->SetUsItem(UR_UPDATED);
		pPlayer->SaySystem("你的宠物还没有放出,不能喂食.");
		return 0;
	}
	else
	{
		pUsItem = pPlayer->GetbaoziItem();
		pUsItem->SetPetTime();
		pPlayer->SaySystemAttrib(CC_REDPET, "喂食成功, 你的从林豹又充满精神!");
	}
	//更新
	BOOL te = CItemManager::GetInstance()->UpgradePetItem(*pUsItem, Params[1].pszParam);
	pPlayer->SendUpdateItem(*pUsItem);
	WORD id = pUsItem->baseitem.wDc;
	char name[20];
	switch (id)
	{
	case 0:
		strcpy(name, "丛林豹0");
		break;
	case 1:
		strcpy(name, "丛林豹1");
		break;
	case 2:
		strcpy(name, "丛林豹2");
		break;
	case 3:
		strcpy(name, "丛林豹3");
		break;
	case 4:
		strcpy(name, "丛林豹4");
		break;
	case 5:
		strcpy(name, "丛林豹5");
		break;
	case 6:
		strcpy(name, "丛林豹6");
		break;
	case 7:
		strcpy(name, "丛林豹7");
		break;
	default:
		break;
	}
	pPlayer->SetPetName(name);
	if (te)
	{
		pPlayer->SetPetMonster(name);
	}
	CItemManager::GetInstance()->AddItemModifyFlag(*pUsItem, ITEMMODIFY_DURACHANGED);//更新时间
	return 1;
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：创建回城石
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_MAKEHOMESTONE)
{
	if (nParam < 1)return 0;
	return pPlayer->RecordHomeXY(Params[0].pszParam);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：使用回城石
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_USEHOMESTONE)
{
	ITEM* pItem = nullptr;
	if ((pItem = pPlayer->GetUsingItem()) == nullptr)return 0;
	CLogicMap* pMap = pPlayer->GetMap();
	if (pMap == nullptr || pMap->IsFlagSeted(MF_NOHOME))
	{
		pPlayer->SaySystem(getstrings(SD_MAPLIMITED_NOHOME));
		return 0;
	}
	if (pItem->baseitem.wMapId == 0)
	{
		pPlayer->SaySystem("你的回城石还未记录城市, 无法使用");
		return 0;
	}
	return pPlayer->FlyTo(pItem->baseitem.wMapId, pItem->baseitem.wAc, pItem->baseitem.wMac);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查物品的分类
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_CHECKITEMSTDMODE)
{
	if (nParam < 1)return 0;
	ITEM* pItem = nullptr;
	if ((pItem = pPlayer->GetUsingItem()) == nullptr)return 0;
	if (pItem->baseitem.btStdMode != (BYTE)Params[0].nParam)return 0;
	return 1;
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查物品的形状
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_CHECKITEMSHAPE)
{
	if (nParam < 1)return 0;
	ITEM* pItem = nullptr;
	if ((pItem = pPlayer->GetUsingItem()) == nullptr)return 0;
	if (pItem->baseitem.btShape != (BYTE)Params[0].nParam)return 0;
	return 1;
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查物品的名字
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_CHECKITEMNAME)
{
	if (nParam < 1)return 0;
	ITEM* pItem = nullptr;
	if ((pItem = pPlayer->GetUsingItem()) == nullptr)return 0;
	char* szFullName = CItemManager::GetInstance()->GetItemClassPtr(*pItem)->szFullName;
	if (strcmp(szFullName, Params[0].pszParam) == 0)return 1;
	return 0;
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查物品是否绑定
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_CHECKITEMBIND)
{
	ITEM* pItem = nullptr;
	if ((pItem = pPlayer->GetUsingItem()) == nullptr)return 0;
	return pItem->IsBind();
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查PACK物品的名字
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_CHECKPACKITEMNAME)
{
	if (nParam < 1)return 0;
	ITEM* pItem = nullptr;
	if ((pItem = pPlayer->GetPackItem()) == nullptr)return 0;
	char* szFullName = CItemManager::GetInstance()->GetItemClassPtr(*pItem)->szFullName;
	if (strcmp(szFullName, Params[0].pszParam) == 0)return 1;
	return 0;
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：释放宠物
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_RELEASEPET)
{
	if (nParam == 0)return 0;
	ITEM* pItem = nullptr;
	if ((pItem = pPlayer->GetUsingItem()) == nullptr)return 0;
	char szPetName[64];
	WORD wLevel = pItem->baseitem.wDc;
	_snprintf(szPetName, 60, "%s%u", Params[0].pszParam, wLevel);
	return pPlayer->SummonPet(szPetName);
	//if( nParam == 0 )return 0;
	//pPlayer->SetPrivateShopSign( Params[0].nParam );
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查限制 - 待完善
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(US_CHECKLIMIT)
{
	if (nParam == 0)return 0;

}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：提交物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(PUTITEM)
{
	if (nParam < 2)return 0;
	pTarget->SetPutItemPage(Params[1].pszParam, pShell);
	pPlayer->AddProcess(EP_PUTITEM, 0, 0, 0, 0, 0, 0, Params[0].pszParam);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示烟花
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWFIREWORKS)
{
	if (nParam == 0)return 0;
	pPlayer->SendMsg(pPlayer->GetId(), 0x152, pPlayer->getX(), pPlayer->getY(), (WORD)pPlayer->GetDirection(), &Params[0].nParam, sizeof(WORD));
	pPlayer->SendAroundMsg(pPlayer->GetId(), 0x152, pPlayer->getX(), pPlayer->getY(), (WORD)pPlayer->GetDirection(), &Params[0].nParam, sizeof(WORD));
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示矿堆
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWPILE)
{
	if (nParam == 0)return 0;
	DWORD dwParams[4] = { Params[0].nParam, nParam > 1 ? Params[1].nParam * 1000 : -1, nParam > 2 ? Params[2].nParam : 0, nParam > 3 ? Params[3].nParam : 0 };
	int x, y;
	pPlayer->GetFrontPosition(x, y);
	CEventManager::GetInstance()->NewVisibleEvent(pPlayer->GetMap(), x, y, dwParams[0], 1000, dwParams[1], nullptr, dwParams[2], dwParams[3]);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示攻击矿堆
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWDAMAGEPILE)
{
	if (nParam == 0)return 0;
	//	( mapid, x, y, nView, nRange, nDamage, nDamageType, nLastTime, nRunTick )	9
	//	( nView, nRange, nDamage, nDamageType, nLastTime, nRunTick )				6
	int	x = 0;
	int y = 0;

	UINT nMapId = 0;
	UINT nView = 0;
	UINT Last = 0;
	UINT RunTick = 0;

	UINT nDamage = 0;
	UINT nRange = 0;

	damage_type dtype = DT_PHYSICS;	//DT_DIRECT
	UINT nParam1 = 0, nParam2 = 0;

	if (nParam == 9)
	{
		nMapId = Params[0].nParam;
		x = Params[1].nParam;
		y = Params[2].nParam;
		nView = Params[3].nParam;
		nRange = Params[4].nParam;
		nDamage = Params[5].nParam;
		dtype = (damage_type)(Params[6].nParam % 3);
		Last = Params[7].nParam * 1000;
		RunTick = Params[8].nParam * 1000;
	}
	else if (nParam == 6)
	{
		nView = Params[0].nParam;
		nRange = Params[1].nParam;
		nDamage = Params[2].nParam;
		dtype = (damage_type)(Params[3].nParam % 3);
		Last = Params[4].nParam * 1000;
		RunTick = Params[5].nParam * 1000;
		nMapId = pPlayer->GetMapId();
		pPlayer->GetFrontPosition(x, y);
	}
	else
		return 0;
	//CEventManager::GetInstance()->NewVisibleEvent( pPlayer->GetMap(), x, y, dwParams[0], 1000, dwParams[1],  nullptr, dwParams[2], dwParams[3] );
	return (CDamageEvent::Create(nMapId, x, y, nRange, nView, nParam1, nParam2, nDamage, dtype, pPlayer, Last, RunTick) != nullptr);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：更改字体
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEFONT)
{
	if (nParam == 0)return 0;
	DWORD dwTime = (DWORD)-1;
	if (nParam > 1)
		dwTime = Params[1].nParam * 1000;
	pPlayer->SetSystemFlag(SF_FONTCHANGED, Params[0].nParam != 0, Params[0].nParam, dwTime);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：更改天气
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEWEATHER)
{
	if (nParam == 0)return 0;
	CLogicMap* pMap = nullptr;
	UINT nWeather = 0;
	if (nParam == 1)
	{
		nWeather = Params[0].nParam;
		pMap = pPlayer->GetMap();
	}
	else
	{
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(Params[0].nParam);
		nWeather = Params[1].nParam;
	}
	//0：天晴
	//1：细雨缠绵	@天气 $0001 $0000 $0000000
	//2：烟雨朦胧	@天气 $f102 $0000 $ffffffff
	//3：沙漠风暴	@天气 $1403 $0032 $ffffff99
	//4：玫瑰花雨	@天气 $0004 $0000 $0000000
	//5：紫樱缤纷	@天气 $0005 $0000 $0000000
	//6：飞雪连天	@天气 $0006 $0000 $0000000
	WORD nSpeed = 0;
	DWORD dwFogColor = 0;
	if (nWeather == 3)
	{
		nSpeed = 0x32;
		nWeather |= 0x1400;
		dwFogColor = 0xffffff99;
	}
	else if (nWeather == 2)
	{
		nWeather |= 0xf100;
		dwFogColor = 0xffffffff;
	}
	pMap->GetWeather().wWeatherIndex = nWeather;
	pMap->GetWeather().wFlag = nSpeed;
	pMap->GetWeather().dwWeatherColor = dwFogColor;
	CGameWorld::GetInstance()->AddGlobeProcess(EP_WEATHERCHANGED, pMap->GetIndex());
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：更改技能颜色
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEMAGICCOLOR)
{
	DWORD dwColor = 0;
	if (nParam < 1)return 0;
	if (nParam > 1)
		dwColor = Params[1].nParam;
	USERMAGIC* pMagic = nullptr;
	if (Params[0].nParam == 0 && Params[0].pszParam[0] == 0)return 0;
	if (Params[0].nParam != 0)
		pMagic = pPlayer->GetMagic(Params[0].nParam);
	else
		pMagic = pPlayer->GetMagicByName(Params[0].pszParam);
	if (pMagic == nullptr)return 0;
	if (pMagic->dwColor == dwColor)
		pMagic->dwColor = 0;
	else
		pMagic->dwColor = dwColor;
	pPlayer->SendMsg(dwColor, 0xb4a4, pMagic->pClass->id, 0, 0);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置摆摊风格
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETPRIVATESHOPSTYLE)
{
	if (nParam == 0)return 0;
	pPlayer->SetPrivateShopStyle(Params[0].nParam);
}
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置摆摊标示
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETPRIVATESHOPFLAG)
{
	if (nParam == 0)return 0;
	pPlayer->SetPrivateShopSign(Params[0].nParam);
}
END_SCRIPT_FUNCTION
