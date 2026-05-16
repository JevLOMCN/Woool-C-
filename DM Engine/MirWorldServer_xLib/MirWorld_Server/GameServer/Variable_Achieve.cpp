#include "stdafx.h"
#include "scriptvariable.h"
#include "humanplayer.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	获取玩家当前成就点数值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_VAR(ACHIEVEPOINT) {
	DWORD pExp = pPlayer->GetAchieveExp();
	result.SetValue(pExp);
}END_SCRIPT_VAR

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	获取指定成就相关信息-成就名称
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_VAR(ACHIEVE_NAME) {
	if (nParam == 1)
	{
		if (Params[0].nParam < CTimeAchieve::GetInstance()->GetAchieveCount())
		{
			const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->FindAchieveById(Params[0].nParam);
			result.SetValue(pAchieveItem->szName);
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}END_SCRIPT_VAR

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	获取玩家指定成就相关信息-成就进度
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_VAR(ACHIEVE_EXP) {
	if (nParam == 1)
	{
		if (Params[0].nParam < CTimeAchieve::GetInstance()->GetAchieveCount())
		{
			DWORD pExp = pPlayer->GetAchieveExpById(Params[0].nParam);
			result.SetValue(pExp);
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}END_SCRIPT_VAR

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	获取指定成就相关信息-成就最大进度
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_VAR(ACHIEVE_MAXEXP) {
	if (nParam == 1)
	{
		if (Params[0].nParam < CTimeAchieve::GetInstance()->GetAchieveCount())
		{
			const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->FindAchieveById(Params[0].nParam);
			result.SetValue(pAchieveItem->nMaxExp);
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}END_SCRIPT_VAR

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	获取玩家成指定就相关信息-成就状态
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_VAR(ACHIEVE_STATE) {
	if (nParam == 1)
	{
		if (Params[0].nParam < CTimeAchieve::GetInstance()->GetAchieveCount())
		{
			BYTE pState = pPlayer->GetAchieveStateById(Params[0].nParam);
			result.SetValue(pState);
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}END_SCRIPT_VAR

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	获取玩家指定成就相关信息-成就完成时间
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_VAR(ACHIEVE_TIME) {
	if (nParam == 1)
	{
		if (Params[0].nParam < CTimeAchieve::GetInstance()->GetAchieveCount())
		{
			DWORD pCompleteTime = pPlayer->GetAchieveCompleteTimeById(Params[0].nParam);
			result.SetValue(pCompleteTime);
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}END_SCRIPT_VAR

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	获取指定成就相关信息-成就名称
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_VAR(ACHIEVE_POINT) {
	if (nParam == 1)
	{
		if (Params[0].nParam < CTimeAchieve::GetInstance()->GetAchieveCount())
		{
			const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->FindAchieveById(Params[0].nParam);
			result.SetValue(pAchieveItem->nPoint);
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}END_SCRIPT_VAR

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	获取指定成就相关信息-成就组ID
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_VAR(ACHIEVE_GROUP) {
	if (nParam == 1)
	{
		if (Params[0].nParam < CTimeAchieve::GetInstance()->GetAchieveCount())
		{
			const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->FindAchieveById(Params[0].nParam);
			result.SetValue(pAchieveItem->nGroup);
		}
		else
			return FALSE;
	}
	else
		return FALSE;
}END_SCRIPT_VAR
