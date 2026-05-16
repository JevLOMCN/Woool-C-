#pragma once
#include "findfile.h"

class CGuild;
class CHumanPlayer;

class CGuildManager : 
	public CFindFile,
	public xSingletonClass<CGuildManager>
{
public:
	CGuildManager(void);
	virtual ~CGuildManager(void);
	CGuild * FindGuild( const char * pszName );
	BOOL	BuildGuild( CHumanPlayer * pCreator, const char * pszName );
	VOID	RestoreAllGuildName();
protected:
	VOID OnFoundFile( const char * pszFileName, UINT nParam = 0 );
	xObjectPool<CGuild>	m_xGuildPool;
	CNameHash	m_GuildHash;
	CGuild * m_pGuildList;
};
