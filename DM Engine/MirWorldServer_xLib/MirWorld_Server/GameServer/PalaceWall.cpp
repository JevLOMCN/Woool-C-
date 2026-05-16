#include "StdAfx.h"
#include ".\palacewall.h"
#include "GameWorld.h"
#include "monstermanagerex.h"
#include "logicmap.h"

CPalaceWall::CPalaceWall(void)
{
	Clean();
}

CPalaceWall::~CPalaceWall(void)
{
}

BOOL CPalaceWall::Init(const char* pszName, int mapid, int x, int y, WORD wHp)
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
	return TRUE;
}

VOID CPalaceWall::OnDeath(DWORD dwKiller)
{
	SetDirection(ED_DN);
	CleanAroundBlock();
}

VOID CPalaceWall::OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type)
{
	WORD wHp = static_cast<WORD>(GetPropValue(PI_CURHP));
	WORD wMaxHp = static_cast<WORD>(GetPropValue(PI_MAXHP));
	int changedir = -1;
	int curdir = (int)GetDirection();

	if (wHp < wMaxHp / 4)
		changedir = 4;
	else if (wHp < wMaxHp / 2)
		changedir = 3;
	else if (wHp < wMaxHp * 3 / 4)
		changedir = 1;

	if (changedir != -1 && curdir != changedir)
	{
		SetDirection((e_direction)changedir);
		DWORD dwFeature = GetFeather();
		SendAroundMsg(GetId(), 0xc, getX(), getY(), (int)GetDirection(), &dwFeature, 4);
	}
}

VOID CPalaceWall::Repair()//ÖØÐÂÐÞ¸´
{
	if (m_bDead)
	{
		char szMsg[1024];
		int length = 0;
		if (this->GetOutViewmsg(szMsg, length))
		{
			SendAroundMsg(szMsg, length);
		}
		m_bDead = FALSE;
		SetDirection(ED_UP);
		if (this->GetViewmsg(szMsg, length))
			SendAroundMsg(szMsg, length);
	}
	SetAroundBlock();
	CMonsterEx::m_wCurHp = GetPropValue(PI_MAXHP);
	SendHpMpChanged();
}

VOID CPalaceWall::OnEnterMap(CLogicMap* pMap)
{
	SetAroundBlock();
	CMonsterEx::OnEnterMap(pMap);
}

VOID CPalaceWall::CleanAroundBlock()
{
	//int x = getX();
	//int y = getY();
	//int nx, ny;
	//for( int dir = 0;dir < 8;dir ++ )
	//{
	//	nx = x, ny = y;
	//	GETNEXTPOS( nx, ny, dir );
	//	if( m_pMap != nullptr )
	//		m_pMap->UnLockPos( nx, ny );
	//}
}

VOID CPalaceWall::SetAroundBlock()
{
	//int x = getX();
	//int y = getY();
	//int nx, ny;
	//for( int dir = 0;dir < 8;dir ++ )
	//{
	//	nx = x, ny = y;
	//	GETNEXTPOS( nx, ny, dir );
	//	if( m_pMap != nullptr )
	//		m_pMap->LockPos( nx, ny );
	//}
}