#pragma once
struct gm_node
{
	char szAccount[32];
	int	level;
};

class CSe_Page;

typedef struct tagGameCommand
{
	tagGameCommand()
	{
		FILLSELF(0);
	}
	//	CSe_Page * pPage;
	StringCacheNode* pPage;
	fnCommandProc proc;
	BOOL fIsCallPage;
	BOOL fIsGmCmd;
	UINT nLimitLevel;
}GameCommand;

class CGmManager : public xSingletonClass<CGmManager>
{
public:
	CGmManager(void);
	virtual ~CGmManager(void);
	BOOL Load(const char* pszFile);
	VOID Save(const char* pszFile);
	int	GetGmLevel(const char* pszAccount);
	BOOL LoadCommandDef(const char* pszFile);
	BOOL ExecGameCmd(const char* pszCommand, CHumanPlayer* pPlayer);
	/*VOID OnGmCommand( */
	BOOL MapCommand(int iLevel, const char* pszCommand, const char* pszBuildInCommand);
private:
	VOID ClearCmdList();
	xStringList<512> m_xCmdList;
	xObjectPool<gm_node> m_xGmNodePool;
	CNameHash m_GmHash;
};