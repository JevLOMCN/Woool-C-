#include "StdAfx.h"
#include ".\guildmanager.h"
#include "guild.h"
#include "humanplayer.h"

CGuildManager::CGuildManager(void)
{
m_pGuildList = nullptr;
}

CGuildManager::~CGuildManager(void)
{
}

VOID CGuildManager::OnFoundFile( const char * pszFileName, UINT nParam )
{ 
	CGuild * pGuild = m_xGuildPool.newObject();
	if( pGuild == nullptr )return;
	if( !pGuild->LoadFromFile(pszFileName) )
	{
		pGuild->Clean();
		m_xGuildPool.deleteObject( pGuild );
		return;
	}
	if( !m_GuildHash.HAdd( pGuild->GetName(), pGuild ) )
	{
		pGuild->Clean();
		m_xGuildPool.deleteObject(pGuild);
	}
	else
	{
		pGuild->SetNext( m_pGuildList );
		m_pGuildList = pGuild;
	}
}

CGuild * CGuildManager::FindGuild( const char * pszName )
{ 
	return (CGuild*)m_GuildHash.HGet( pszName );
}

BOOL	CGuildManager::BuildGuild( CHumanPlayer * pCreator, const char * pszName )
{ 
	CGuild * pGuild = FindGuild( pszName );
	if( pGuild != nullptr )
	{
		pCreator->SaySystem( "该名字的行会已经存在！" );
		return FALSE;
	}
	pGuild = m_xGuildPool.newObject();
	if( pGuild == nullptr )return FALSE;
	if( !pGuild->Create( pCreator, pszName ) )
	{
		pGuild->Clean();
		m_xGuildPool.deleteObject( pGuild );
		return FALSE;
	}
	if( !m_GuildHash.HAdd( pszName, pGuild ) )
	{
		//pCreator->SetGuild( nullptr );
		pGuild->Clean();
		m_xGuildPool.deleteObject( pGuild );
		return FALSE;
	}

	if(!pGuild->AddMember(pCreator))
	{
		m_GuildHash.HDel( pszName );
		pGuild->Clean();
		m_xGuildPool.deleteObject( pGuild );
		return FALSE;
	}
	pGuild->SaveToFile( ".\\data\\guildbase\\guilds" );
	return TRUE;
}
#include "humanplayermgr.h"

VOID	CGuildManager::RestoreAllGuildName()
{
	//xStringList<1000>	xRestoreList;
	//xStringList<1000>	xRestoreMutexList;
	//CDBClientObj * pObj = CServer::GetInstance()->GetDBConnection( DI_CHARINFO );
	//if( pObj == nullptr )return;

	//CGuild * pGuild = m_pGuildList;
	//while( pGuild )
	//{
	//	pGuild->RestoreAllGuildName( xRestoreList, xRestoreMutexList );
	//	pGuild = pGuild->GetNext();
	//}
	//UINT i = 0;
	//for( i = 0;i < xRestoreList.GetCount();i ++ )
	//{
	//	char * p = xRestoreList[i]->pszString;
	//	if( xRestoreMutexList.IndexOf( p ) != -1 )
	//	{
	//		xRestoreMutexList.Add( p, xRestoreList[i]->lpObject );
	//		continue;
	//	}
	//	CHumanPlayer * player = CHumanPlayerMgr::GetInstance()->FindbyName( p );
	//	CGuildEx * pGuild = (CGuildEx *)xRestoreList[i]->lpObject;
	//	if( pGuild == nullptr )continue;
	//	if( player )
	//	{
	//		if( player->GetGuild() == nullptr )
	//		{
	//			player->SetGuild( pGuild );	
	//			player->SaySystemAttrib( CC_GREEN, "您的行会信息已经被恢复！" );
	//		}
	//	}
	//	else
	//	{/**/
	//		try{
	//			pObj->SendRestoreGuild( p, pGuild->GetName() );
	//		}
	//		catch(...)
	//		{
	//			PRINT( 0xff, "错误信息: 0x%08x 0x%08x\n", p, pGuild );
	//		}
	//	/**/}
	//}
	//FILE * fp = fopen( ".\\行会重复人员.txt", "w" );
	//if( fp != nullptr )
	//{
	//	for( i = 0;i < xRestoreMutexList.GetCount();i ++ )
	//	{
	//		CGuild * pGuild = (CGuild*)xRestoreMutexList[i]->lpObject;
	//		char * p = xRestoreMutexList[i]->pszString;
	//		if( p == nullptr || pGuild == nullptr )return;
	//		fprintf( fp, "%s : %s\n", p, pGuild->GetName() );
	//	}
	//	fclose( fp );
	//}
}
