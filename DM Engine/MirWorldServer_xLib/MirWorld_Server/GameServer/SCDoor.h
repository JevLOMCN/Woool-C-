#pragma once
#include "monsterex.h"
#include "GameWorld.h"
#include "monstermanagerex.h"

class CSCDoor : public CMonsterEx
{
public:
	CSCDoor(void);
	virtual ~CSCDoor(void);
	BOOL Init(const char* pszName, int mapid, int x, int y, WORD wHp, BOOL bOpened = FALSE);
	VOID OnDeath(DWORD dwKiller);
	VOID OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type);

	BOOL IsOpened()const { return m_bOpened; }
	VOID ChangeOpenState()
	{
		if (m_bOpened)
		{
			m_bOpened = FALSE;
			Open();
		}
		else
		{
			m_bOpened = TRUE;
			Close();
		}
	}
	BOOL Open();
	BOOL Close();

	VOID Repair();
	VOID LockAround();
	VOID UnLockAround();
protected:
	BOOL m_bOpened;
};