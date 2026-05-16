#include "stdafx.h"
#include "scriptvariable.h"
#include "topmanager.h"
#include "humanplayer.h"
#include "gameworld.h"
#include "sandcity.h"

DEFINE_SCRIPT_VAR(TOP_0_0_NAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(0);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szName);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_0_SEX) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(0);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btSex == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MALE));
	else if (pInfo->btSex == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_FEMALE));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_0_CLASS) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(0);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btClass == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_WARR));
	else if (pInfo->btClass == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MAGICAN));
	else if (pInfo->btClass == 2)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_TAOSHI));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_0_LEVEL) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(0);
	if (pInfo == nullptr)return FALSE;
	result.SetValue((UINT)pInfo->wLevel);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_0_EXP) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(0);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pPlayer->GetPropValue(PI_EXP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_0_RANKDATE) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(0);
	if (pInfo == nullptr)return FALSE;
	char* pStringCache = CScriptVariableManager::GetInstance()->GetStringCache();
	if (pStringCache == nullptr)return FALSE;
	result.SetValue(pStringCache);
	sprintf(pStringCache, "%04u-%02u-%02u", pInfo->stRanking.wYear, pInfo->stRanking.wMonth, pInfo->stRanking.wDay);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_0_GUILDNAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(0);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szGuild);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_1_NAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(1);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szName);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_1_SEX) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(1);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btSex == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MALE));
	else if (pInfo->btSex == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_FEMALE));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_1_CLASS) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(1);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btClass == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_WARR));
	else if (pInfo->btClass == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MAGICAN));
	else if (pInfo->btClass == 2)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_TAOSHI));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_1_LEVEL) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(1);
	if (pInfo == nullptr)return FALSE;
	result.SetValue((UINT)pInfo->wLevel);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_1_EXP) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(1);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pPlayer->GetPropValue(PI_EXP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_1_RANKDATE) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(1);
	if (pInfo == nullptr)return FALSE;
	char* pStringCache = CScriptVariableManager::GetInstance()->GetStringCache();
	if (pStringCache == nullptr)return FALSE;
	result.SetValue(pStringCache);
	sprintf(pStringCache, "%04u-%02u-%02u", pInfo->stRanking.wYear, pInfo->stRanking.wMonth, pInfo->stRanking.wDay);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_0_1_GUILDNAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(1);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szGuild);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_0_NAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(2);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szName);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_0_SEX) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(2);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btSex == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MALE));
	else if (pInfo->btSex == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_FEMALE));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_0_CLASS) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(2);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btClass == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_WARR));
	else if (pInfo->btClass == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MAGICAN));
	else if (pInfo->btClass == 2)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_TAOSHI));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_0_LEVEL) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(2);
	if (pInfo == nullptr)return FALSE;
	result.SetValue((UINT)pInfo->wLevel);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_0_EXP) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(2);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pPlayer->GetPropValue(PI_EXP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_0_RANKDATE) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(2);
	if (pInfo == nullptr)return FALSE;
	char* pStringCache = CScriptVariableManager::GetInstance()->GetStringCache();
	if (pStringCache == nullptr)return FALSE;
	result.SetValue(pStringCache);
	sprintf(pStringCache, "%04u-%02u-%02u", pInfo->stRanking.wYear, pInfo->stRanking.wMonth, pInfo->stRanking.wDay);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_0_GUILDNAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(2);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szGuild);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_1_NAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(3);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szName);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_1_SEX) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(3);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btSex == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MALE));
	else if (pInfo->btSex == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_FEMALE));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_1_CLASS) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(3);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btClass == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_WARR));
	else if (pInfo->btClass == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MAGICAN));
	else if (pInfo->btClass == 2)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_TAOSHI));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_1_LEVEL) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(3);
	if (pInfo == nullptr)return FALSE;
	result.SetValue((UINT)pInfo->wLevel);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_1_EXP) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(3);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pPlayer->GetPropValue(PI_EXP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_1_RANKDATE) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(3);
	if (pInfo == nullptr)return FALSE;
	char* pStringCache = CScriptVariableManager::GetInstance()->GetStringCache();
	if (pStringCache == nullptr)return FALSE;
	result.SetValue(pStringCache);
	sprintf(pStringCache, "%04u-%02u-%02u", pInfo->stRanking.wYear, pInfo->stRanking.wMonth, pInfo->stRanking.wDay);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_1_1_GUILDNAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(3);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szGuild);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_0_NAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(4);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szName);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_0_SEX) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(4);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btSex == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MALE));
	else if (pInfo->btSex == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_FEMALE));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_0_CLASS) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(4);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btClass == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_WARR));
	else if (pInfo->btClass == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MAGICAN));
	else if (pInfo->btClass == 2)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_TAOSHI));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_0_LEVEL) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(4);
	if (pInfo == nullptr)return FALSE;
	result.SetValue((UINT)pInfo->wLevel);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_0_EXP) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(4);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pPlayer->GetPropValue(PI_EXP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_0_RANKDATE) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(4);
	if (pInfo == nullptr)return FALSE;
	char* pStringCache = CScriptVariableManager::GetInstance()->GetStringCache();
	if (pStringCache == nullptr)return FALSE;
	result.SetValue(pStringCache);
	sprintf(pStringCache, "%04u-%02u-%02u", pInfo->stRanking.wYear, pInfo->stRanking.wMonth, pInfo->stRanking.wDay);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_0_GUILDNAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(4);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szGuild);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_1_NAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(5);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szName);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_1_SEX) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(5);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btSex == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MALE));
	else if (pInfo->btSex == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_FEMALE));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_1_CLASS) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(5);
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btClass == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_WARR));
	else if (pInfo->btClass == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MAGICAN));
	else if (pInfo->btClass == 2)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_TAOSHI));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_1_LEVEL) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(5);
	if (pInfo == nullptr)return FALSE;
	result.SetValue((UINT)pInfo->wLevel);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_1_EXP) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(5);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pPlayer->GetPropValue(PI_EXP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_1_RANKDATE) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(5);
	if (pInfo == nullptr)return FALSE;
	char* pStringCache = CScriptVariableManager::GetInstance()->GetStringCache();
	if (pStringCache == nullptr)return FALSE;
	result.SetValue(pStringCache);
	sprintf(pStringCache, "%04u-%02u-%02u", pInfo->stRanking.wYear, pInfo->stRanking.wMonth, pInfo->stRanking.wDay);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(TOP_2_1_GUILDNAME) {
	TopCharInfo* pInfo = CTopManager::GetInstance()->GetTopCharInfo(5);
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szGuild);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SABUKMASTER_NAME) {
	TopCharInfo* pInfo = CSandCity::GetInstance()->GetMasterInfo();
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pInfo->szName);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SABUKMASTER_SEX) {
	TopCharInfo* pInfo = CSandCity::GetInstance()->GetMasterInfo();
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btSex == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MALE));
	else if (pInfo->btSex == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_FEMALE));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SABUKMASTER_CLASS) {
	TopCharInfo* pInfo = CSandCity::GetInstance()->GetMasterInfo();
	if (pInfo == nullptr)return FALSE;
	if (pInfo->btClass == 0)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_WARR));
	else if (pInfo->btClass == 1)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_MAGICAN));
	else if (pInfo->btClass == 2)result.SetValue(CGameWorld::GetInstance()->GetName(ENI_TAOSHI));
	else result.SetValue("");
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SABUKMASTER_LEVEL) {
	TopCharInfo* pInfo = CSandCity::GetInstance()->GetMasterInfo();
	if (pInfo == nullptr)return FALSE;
	result.SetValue((UINT)pInfo->wLevel);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SABUKMASTER_EXP) {
	TopCharInfo* pInfo = CSandCity::GetInstance()->GetMasterInfo();
	if (pInfo == nullptr)return FALSE;
	result.SetValue(pPlayer->GetPropValue(PI_EXP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(SABUKMASTER_RANKDATE) {
	TopCharInfo* pInfo = CSandCity::GetInstance()->GetMasterInfo();
	if (pInfo == nullptr)return FALSE;
	char* pStringCache = CScriptVariableManager::GetInstance()->GetStringCache();
	if (pStringCache == nullptr)return FALSE;
	result.SetValue(pStringCache);
	sprintf(pStringCache, "%04u-%02u-%02u", pInfo->stRanking.wYear, pInfo->stRanking.wMonth, pInfo->stRanking.wDay);
}END_SCRIPT_VAR
