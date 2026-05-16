#include "stdafx.h"
#include "scriptvariable.h"
#include "humanplayer.h"

DEFINE_SCRIPT_VAR(UI_NAME) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	char* p = CScriptVariableManager::GetInstance()->GetStringCache();
	memcpy(p, pItem->baseitem.szName, 14);
	p[14] = 0;
	result.SetValue(p);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_AC1) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Ac1);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_AC2) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Ac2);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_MAC1) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Mac1);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_MAC2) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Mac2);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_DC1) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Dc1);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_DC2) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Dc2);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_MC1) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Mc1);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_MC2) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Mc2);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_SC1) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Sc1);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_SC2) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.Sc2);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_STDMODE) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.btStdMode);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_SHAPE) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.btShape);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_IMAGE) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.wImageIndex);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_WEIGHT) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.btWeight);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_CURDURA) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->wCurDura);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_MAXDURA) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->wMaxDura);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_DEFDURA) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.wMaxDura);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(UI_PRICE) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue((UINT)pItem->baseitem.nPrice);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(DI_POSX) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue(pPlayer->GetParam(0));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(DI_POSY) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue(pPlayer->GetParam(1));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(DI_MAPID) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue(pPlayer->GetParam(2));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(DI_EXECUTETIME) {
	ITEM* pItem = nullptr;
	if (pPlayer == nullptr ||
		(pItem = pPlayer->GetUsingItem()) == nullptr)return FALSE;
	result.SetValue(pPlayer->GetParam(3));
}END_SCRIPT_VAR
