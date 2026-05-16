#include "StdAfx.h"
#include ".\humanplayermgr.h"
#include ".\gameworld.h"

CHumanPlayerMgr::CHumanPlayerMgr(void)
{
	m_HumanPlayers.Create(1024);
	VMP_PROTECT_BEGIN("CHumanPlayerMgr-boTest");
	m_boTest = FALSE;
	VMP_PROTECT_END();
}

CHumanPlayerMgr::~CHumanPlayerMgr(void)
{
}

CHumanPlayer* CHumanPlayerMgr::FindbyName(const char* pszName)
{
	return (CHumanPlayer*)m_PlayerNameHash.HGet(pszName);
}

CHumanPlayer* CHumanPlayerMgr::FindbyId(UINT id)
{
	if (((id & 0xff000000) >> 24) == OBJ_PLAYER)
		id &= 0xffffff;
	return m_HumanPlayers.Get(id);
}

BOOL CHumanPlayerMgr::AddPlayerNameList(CHumanPlayer* pPlayer, const char* pszName)
{
	return m_PlayerNameHash.HAdd(pszName, (LPVOID)pPlayer);
}

CHumanPlayer* CHumanPlayerMgr::NewPlayer()
{
	VMP_PROTECT_BEGIN("NewPlayer-boTest");
	if (IsTestMode())
	{
		if (m_HumanPlayers.GetCount() >= 5)
			return nullptr;
	}
	VMP_PROTECT_END();
	CHumanPlayer* pPlayer = nullptr;
	UINT id = 0;
	id = m_HumanPlayers.New(&pPlayer);
	if (id == 0 || pPlayer == nullptr) return nullptr;
	id |= (OBJ_PLAYER << 24);
	pPlayer->SetId(id);
	return pPlayer;
}

BOOL CHumanPlayerMgr::DeletePlayer(CHumanPlayer* pPlayer)
{
	UINT id = (pPlayer->GetId() & 0xffffff);
	m_PlayerNameHash.HDel(pPlayer->GetName());
	pPlayer->Clean();
	return m_HumanPlayers.Del(id);
}