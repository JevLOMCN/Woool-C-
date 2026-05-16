#include "stdafx.h"
#include "scriptvariable.h"
#include "humanplayer.h"
#include "LogicMap.h"
#include "sandcity.h"
#include "guildex.h"

DEFINE_SCRIPT_VAR(AC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MINAC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXAC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXAC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MINMAC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXMAC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXMAC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(DC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MINDC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXDC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXDC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MINSC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXSC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXSC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MINMC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXMC) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXMC);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(WW) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_CURBODYWEIGHT);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXWW) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXBODYWEIGHT);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(HW) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_CURHANDWEIGHT);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXHW) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXHANDWEIGHT);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(BW) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_CURBAGWEIGHT);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXBW) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXBAGWEIGHT);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(CREDIT) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetCredit();
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(LEVEL) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_LEVEL);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(HP) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_CURHP);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXHP) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXHP);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MP) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_CURMP);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXMP) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPropValue(PI_MAXMP);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(PKPOINT) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetPkValue();
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(LOGINTIME) {
	if (pPlayer == nullptr)return FALSE;
	char* pStringCache = CScriptVariableManager::GetInstance()->GetStringCache();
	if (pStringCache == nullptr)return FALSE;
	CSystemTime* pLoginTime = pPlayer->GetLoginTime();
	result.SetValue(pStringCache);
	sprintf(pStringCache, "%d-%d-%d %d:%02d:%02d",
		pLoginTime->GetYear(), pLoginTime->GetMonth(), pLoginTime->GetDay(),
		pLoginTime->GetHour(), pLoginTime->GetMinute(), pLoginTime->GetSecond());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(LOGINLONG) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetLoginLong();
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(NOWTIMESTAMP) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetTimeStamp());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(GOLD) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetMoney(MT_GOLD);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(YUANBAO) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetMoney(MT_YUANBAO);
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(POSX) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->getX();
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(POSY) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->getY();
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAPID) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = pPlayer->GetMapId();
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(DIRECTION) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = (UINT)pPlayer->GetDirection();
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAPNAME) {
	if (pPlayer == nullptr)return FALSE;
	CLogicMap* pMap = pPlayer->GetMap();
	if (pMap)
		result.SetValue(pMap->GetTitle());
	else
		result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(EXPFACTOR) {
	if (pPlayer == nullptr)return FALSE;
	UINT nVal = static_cast<UINT>(pPlayer->GetExpFactor() * 100.0f);  // 四舍五入
	result.SetValue(nVal);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MASTER) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetMaster());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MARRIAGE) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetMarriage());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(STUDENT1) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetStudent(0));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(STUDENT2) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetStudent(1));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(STUDENT3) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetStudent(2));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(USERNAME) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetName());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(RANKNAME) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetGuildTitle());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(GUILDNAME) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetGuildName());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(EXP) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetPropValue(PI_EXP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MAXEXP) {
	if (pPlayer == nullptr)return FALSE;
	result.SetValue(pPlayer->GetLevelupExp());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(STONE) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_STONE);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SHOES) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_SHOE);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(BELT) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_BELT);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MISC) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_POISON);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(ARMRING_L) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_ARMRINGL);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(ARMRING_R) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_ARMRINGR);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(RING_L) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_RINGL);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(RING_R) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_RINGR);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(NECKLACE) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_NECKLACE);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(HELMET) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_HELMET);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(RIGHTHAND) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_CHARM);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(WEAPON) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_WEAPON);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(DRESS) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		ITEM* pItem = pPlayer->GetEquipment(_U_DRESS);
		if (pItem)
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pItem->baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("未装备");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UPGRADEWEAPON) {
	if (pPlayer == nullptr)
		return FALSE;
	else
	{
		if (pPlayer->HasUpgradeWeapon())
		{
			char* p = CScriptVariableManager::GetInstance()->GetStringCache();
			o_strncpy(p, pPlayer->GetUpgradeWeapon().baseitem.szName, 14);
			result.SetValue(p);
		}
		else
			result.SetValue("没有升级武器");
	}
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(GUILDMAXMEMBERCOUNT) {
	if (pPlayer == nullptr || pPlayer->GetGuild() == nullptr)
		return FALSE;
	else
		result.SetValue(pPlayer->GetGuild()->GetMaxMemberCount());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(BAGSPACE) {
	result.SetValue((UINT)pPlayer->GetBag().GetFree());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(FORGERATE) {
	result.SetValue((UINT)pPlayer->GetForgePoint());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(GAMETIME) {
	result.SetValue((UINT)pPlayer->GetGameTIme());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SPARAM) {
	if (nParam == 1)
	{
		if (Params[0].nParam < 10)
			result.SetValue(pPlayer->getSParam(Params[0].nParam));
	}
	else
		return FALSE;
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(VPARAM) {
	if (nParam == 1)
	{
		if (Params[0].nParam < 10)
			result.SetValue(pPlayer->getVParam(Params[0].nParam));
	}
	else
		return FALSE;
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TASKVALUES) {
	if (nParam == 2)
	{
		if (Params[1].nParam < 10)
			result.SetValue(pPlayer->GetTaskString(Params[0].nParam, Params[1].nParam));
	}
	else
		return FALSE;
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TASKVALUEI) {
	if (nParam == 2)
	{
		if (Params[1].nParam < 10)
			result.SetValue(pPlayer->GetTaskValue(Params[0].nParam, Params[1].nParam));
	}
	else
		return FALSE;
}END_SCRIPT_VAR
