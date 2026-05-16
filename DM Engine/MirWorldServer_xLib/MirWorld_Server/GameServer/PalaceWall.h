#pragma once
#include "monsterex.h"

class CPalaceWall : public CMonsterEx
{
public:
	CPalaceWall(void);
	virtual ~CPalaceWall(void);
	BOOL Init(const char* pszName, int mapid, int x, int y, WORD wHp);
	VOID OnDeath(DWORD dwKiller);
	VOID OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type);
	VOID Repair();
	VOID OnEnterMap(CLogicMap* pMap);
	VOID CleanAroundBlock();
	VOID SetAroundBlock();
};
