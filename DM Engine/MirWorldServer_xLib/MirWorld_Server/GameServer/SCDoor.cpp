#include "StdAfx.h"
#include ".\scdoor.h"
#include "LogicMap.h"

CSCDoor::CSCDoor(void)
{
	Clean();
	m_bOpened = FALSE;
}

CSCDoor::~CSCDoor(void)
{
}

BOOL CSCDoor::Init(const char* pszName, int mapid, int x, int y, WORD wHp, BOOL bOpened)
{
	MonsterClass* pDesc = CMonsterManagerEx::GetInstance()->GetClassByName(pszName);
	if (!CMonsterEx::Init(pDesc, mapid, x, y, nullptr))
		return FALSE;
	if (!CMonsterManagerEx::GetInstance()->AddExtraMonster(this))
		return FALSE;
	if (!CGameWorld::GetInstance()->AddMapObject(this))
		return FALSE;
	SetSType(0x62);
	SetDirection(ED_UP);
	if (bOpened)
	{
		m_bOpened = FALSE;
		Open();
	}
	else
	{
		m_bOpened = TRUE;
		Close();
	}
	return TRUE;
}
VOID CSCDoor::OnDeath(DWORD dwKiller)
{
	SetDirection(ED_RT);
	UnLockAround();
}

VOID CSCDoor::OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type)
{
	WORD wHp = static_cast<WORD>(GetPropValue(PI_CURHP));
	WORD wMaxHp = static_cast<WORD>(GetPropValue(PI_MAXHP));
	int changedir = -1;
	int curdir = (int)GetDirection();

	if (wHp < wMaxHp / 3)
		changedir = 2;
	else if (wHp < wMaxHp * 2 / 3)
		changedir = 1;

	if (changedir != -1 && curdir != changedir)
	{
		SetDirection((e_direction)changedir);
		DWORD dwFeature = GetFeather();
		SendAroundMsg(GetId(), 0xc, getX(), getY(), (int)GetDirection(), &dwFeature, 4);
	}
}

BOOL CSCDoor::Open()
{
	if (m_bDead) return FALSE;
	if (m_bOpened) return FALSE;
	m_bOpened = TRUE;
	SetDirection(ED_LU);
	DWORD dwFeature = GetFeather();
	SendAroundMsg(GetId(), 0xc, getX(), getY(), (int)GetDirection(), &dwFeature, 4);
	UnLockAround();
	return TRUE;
}

BOOL CSCDoor::Close()
{
	if (m_bDead) return FALSE;
	if (!m_bOpened) return FALSE;
	m_bOpened = FALSE;
	WORD wHp = static_cast<WORD>(GetPropValue(PI_CURHP));
	WORD wMaxHp = static_cast<WORD>(GetPropValue(PI_MAXHP));
	int changedir = 0;
	if (wHp < wMaxHp / 3)
		changedir = 2;
	else if (wHp < wMaxHp * 2 / 3)
		changedir = 1;

	SetDirection((e_direction)changedir);
	DWORD dwFeature = GetFeather();
	SendAroundMsg(GetId(), 0xc, getX(), getY(), (int)GetDirection(), &dwFeature, 4);
	LockAround();
	return TRUE;
}

VOID CSCDoor::Repair()
{
	if (m_bDead)
	{
		char szMsg[1024];
		int length = 0;
		if (this->GetOutViewmsg(szMsg, length))
			SendAroundMsg(szMsg, length);
		m_bDead = FALSE;
		if (this->GetViewmsg(szMsg, length))
			SendAroundMsg(szMsg, length);
	}
	CMonsterEx::m_wCurHp = GetPropValue(PI_MAXHP);
	ChangeOpenState();
	SendHpMpChanged();
}

VOID CSCDoor::LockAround()
{
	if (m_pMap == nullptr)return;
	WORD x = getX(), y = getY();
	m_pMap->LockPos(x, y);
	m_pMap->LockPos(x - 1, y);
	m_pMap->LockPos(x, y - 1);
	m_pMap->LockPos(x + 1, y - 1);
}

VOID CSCDoor::UnLockAround()
{
	if (m_pMap == nullptr)return;
	WORD x = getX(), y = getY();
	m_pMap->UnLockPos(x, y);
	m_pMap->UnLockPos(x - 1, y);
	m_pMap->UnLockPos(x, y - 1);
	m_pMap->UnLockPos(x + 1, y - 1);
}