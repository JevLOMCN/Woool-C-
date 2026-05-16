#include "StdAfx.h"
#include ".\chatchannel.h"
#include "humanplayer.h"
CChatChannel::CChatChannel(void)
{}

CChatChannel::~CChatChannel(void)
{}

void	CChatChannel::Clean()
{TRY_BEGIN
	m_nLimitCount = 0;
TRY_END}

BOOL	CChatChannel::Add( CHumanPlayer * pPlayer )
{TRY_BEGIN
	if( m_xList.getCount() >= m_nLimitCount )
		return FALSE;
	if( m_xList.addObject(pPlayer) )
	{
		OnAdd( pPlayer );
		return TRUE;
	}
	return FALSE;
TRY_END}

VOID	CChatChannel::Remove( CHumanPlayer * pPlayer )
{TRY_BEGIN
	if( m_xList.removeObject(pPlayer))
	{
		OnRemove( pPlayer );
		//return TRUE;
	}
//	return FALSE;
TRY_END}

void	CChatChannel::OnRemove( CHumanPlayer * pPlayer )
{
	//	tell the channel the one leave
}

void CChatChannel::OnAdd( CHumanPlayer * pPlayer )
{
	//	tell the channel the on enter
}

void	CChatChannel::OnWalkThrough( CHumanPlayer * pPlayer )
{
	switch( m_WTOperation.opr )
	{
	case	CCO_SENDMSG:
		{
			pPlayer->OnAroundMsg( nullptr, (char*)m_WTOperation.lpParam, m_WTOperation.iParam );
		}
		break;
	}
}

VOID CChatChannel::fnWalkThrough( CHumanPlayer * pPlayer, CChatChannel * pChannel )
{
	if( pPlayer != nullptr && pChannel != nullptr )
	{
		pChannel->OnWalkThrough( pPlayer );
	}
}
static char	g_szTempBuffer[4096];

VOID CChatChannel::SendChannelMsg( DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpData, int datasize )
{
	int length = encodeMsg( g_szTempBuffer, dwFlag, wCmd, w1, w2, w3, lpData, datasize );
	m_WTOperation.iParam = length;
	m_WTOperation.lpParam = (LPVOID)g_szTempBuffer;
	m_WTOperation.opr = CCO_SENDMSG;
	m_xList.walkThrough((xAutoList::cbWalkThrough)fnWalkThrough,this);
}

