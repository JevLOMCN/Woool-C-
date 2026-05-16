#include "StdAfx.h"
#include "scriptfunction.h"
#include "cmdproc.h"
#include "scriptview.h"
#include "scripttarget.h"
#include "scriptshell.h"
#include "Humanplayer.h"
#include "ScriptElement.h"
#include "ScriptObject.h"
#include "monsterex.h"
#include "monstermanagerex.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		√Ë ˆ£∫«Âø’ƒø±Í
//		◊¢ Õ£∫
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MO_CLEARTARGET)
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	pMonster->SetTarget(0);
	return TRUE;
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		√Ë ˆ£∫±‰…Ì
//		◊¢ Õ£∫
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MO_CHANGEINTO)
	if (nParam < 1)return FALSE;
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	return pMonster->ChangeInto(Params[0].pszParam);
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		√Ë ˆ£∫ «∑Ò”–ƒø±Í
//		◊¢ Õ£∫
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MO_HASTARGET)
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	return (pMonster->GetTarget() != nullptr);
END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		√Ë ˆ£∫ «∑ÒÀ¿Õˆ
//		◊¢ Õ£∫
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MO_ISDEATH)
	CMonsterEx* pMonster = CMonsterManagerEx::GetInstance()->GetCurrentActiveMonster();
	if (pMonster == nullptr)return FALSE;
	return pMonster->IsDeath() ? TRUE : FALSE;
END_SCRIPT_FUNCTION
