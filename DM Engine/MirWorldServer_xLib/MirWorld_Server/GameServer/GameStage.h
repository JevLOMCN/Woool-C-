#pragma once

constexpr int MAXGAMESTAGE = 50;  // 最大游戏阶段数量

typedef struct tagStageItem
{
	tagStageItem()
	{
		FILLSELF(0);
	}
	int nLooks;
	char szDesc[128];
	tagStageItem* pNext;
} STAGEITEM;

typedef struct tagStageContent
{
	tagStageContent()
	{
		FILLSELF(0);
	}
	char szName[64];
	int nLooks;
	STAGEITEM* pItems;
	tagStageContent* pNext;
} STAGECONTENT;

typedef struct tagStageInfo
{
	tagStageInfo()
	{
		FILLSELF(0);
	}
	int nId;
	char szName[64];
	int nDay;
	int nMaxLevel;
	STAGECONTENT* pContents;
} STAGEINFO;

typedef struct tagStageVar
{
	tagStageVar()
	{
		FILLSELF(0);
	}
	char szCurStage[128];
	char szCurDay[128];
	char szSelfLv[128];
} STAGEVAR;

class CGameStage : public xSingletonClass<CGameStage>
{
public:
	CGameStage(void);
	virtual ~CGameStage(void);
	VOID Load(const char* pszPath);
	VOID SendPlayerMapJumpHome(CHumanPlayer* pPlayer);
	VOID SendPlayerMapJumpPage(CHumanPlayer* pPlayer, const char* pszName);
protected:
	VOID Clear();
private:
	CNameHash m_StageHash;
	STAGEINFO* m_pStageList[MAXGAMESTAGE];
	int m_nStageCount;
	STAGEVAR* m_pStageVar;
};