#pragma once
class CGuildEx;
typedef struct tagGuildWar
{
	tagGuildWar()
	{
		FILLSELF(0);
	}
	CGuildEx* pReqGuild;
	CGuildEx* pAtkGuild;
	CServerTimer tmrWar;
}GuildWar;

#define	MAX_GUILD_WAR	1024
class CGuildWarManager :
	public xError,
	public xSingletonClass<CGuildWarManager>
{
public:
	CGuildWarManager(void);
	virtual ~CGuildWarManager(void);
	BOOL RequestWar(CGuildEx* pReqGuild, CGuildEx* pAtkGuild);
	VOID Update();
protected:
	xObjectPool<GuildWar> m_xWarPool;
	GuildWar* m_pGuildWars[MAX_GUILD_WAR];
	UINT m_nWarCount;
	UINT m_nUpdatePtr;
};
