#include "stdafx.h"
#include "stdafx.h"
#include "scriptvariable.h"
#include "monsterex.h"
#include "monstermanagerex.h"
#include "logicmap.h"

DEFINE_SCRIPT_VAR(MV_POSX) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue((UINT)pMonster->getX());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_POSY) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue((UINT)pMonster->getY());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_MAPID) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue((UINT)pMonster->GetMapId());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_MAPNAME) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	if (pMonster->GetMap() == nullptr)return FALSE;
	result.SetValue(pMonster->GetMap()->GetTitle());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_CURHP) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue((UINT)pMonster->GetPropValue(PI_CURHP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_CURMP) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue((UINT)pMonster->GetPropValue(PI_CURMP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_MAXHP) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue((UINT)pMonster->GetPropValue(PI_MAXHP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_MAXMP) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue((UINT)pMonster->GetPropValue(PI_MAXMP));
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_TARGETNAME) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	if (pMonster->GetTarget() == nullptr)return FALSE;
	result.SetValue(pMonster->GetTarget()->GetName());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_NAME) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue(pMonster->GetName());
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_CLASSNAME) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	if (pMonster->GetDesc() == nullptr)return FALSE;
	result.SetValue(pMonster->GetDesc()->base.szClassName);
}END_SCRIPT_VAR

DEFINE_SCRIPT_VAR(MV_LEVEL) {
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	result.SetValue((UINT)pMonster->GetPropValue(PI_LEVEL));
}END_SCRIPT_VAR
