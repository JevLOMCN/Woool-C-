#include "StdAfx.h"
#include ".\scarcher.h"
#include "monsterex.h"
#include "monstermanagerex.h"
#include "gameworld.h"

CSCArcher::CSCArcher(void)
{
	Clean();
}

CSCArcher::~CSCArcher(void)
{
}

BOOL CSCArcher::Init(const char* pszName, int mapid, int x, int y, WORD wHp)
{
	MonsterClass* pDesc = CMonsterManagerEx::GetInstance()->GetClassByName(pszName);
	if (!CMonsterEx::Init(pDesc, mapid, x, y, nullptr))
		return FALSE;
	if (!CMonsterManagerEx::GetInstance()->AddExtraMonster(this))
		return FALSE;
	if (!CGameWorld::GetInstance()->AddMapObject(this))
		return FALSE;
	SetSType(0x62);
	SetDirection(ED_RD);
	return TRUE;
}

VOID CSCArcher::OnDeath(DWORD dwKiller)
{
}

VOID CSCArcher::OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type)
{
}