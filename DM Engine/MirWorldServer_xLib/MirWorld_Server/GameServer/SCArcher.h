#pragma once
#include "monsterex.h"

class CSCArcher : public CMonsterEx
{
public:
	CSCArcher(void);
	virtual ~CSCArcher(void);
	BOOL Init(const char* pszName, int mapid, int x, int y, WORD wHp);
	VOID OnDeath(DWORD dwKiller);
	VOID OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type);
};