#pragma once

constexpr int MAXBOSSTJ = 50;  // 最大BOSSTJ数量

typedef struct tagBossItem
{
	tagBossItem()
	{
		FILLSELF(0);
	}
	char szShowName[64];
	char szName[64];
	tagBossItem* pNext;
} BOSSITEM;

typedef struct tagBoss_TJ
{
	tagBoss_TJ()
	{
		FILLSELF(0);
	}
	char szName[64];
	int nCate;
	int nEnable;
	char szTime[256];
	BYTE nTime;
	int nTimeType;
	int nLvRecomm;
	int nEnterLv;
	char szMap[64];
	int nFreshTime;
	int nPic;
	int nMoveBtn;
	char szOpenDesc[256];
	char szDesc[512];
	BOSSITEM* pItems;
} BOSS_TJ;

class CBossTJ : public xSingletonClass<CBossTJ>
{
public:
	CBossTJ(void);
	virtual ~CBossTJ(void);
	VOID Load(const char* pszPath);
	VOID SendBossList(CHumanPlayer* pPlayer) const;
	VOID SendBoss(CHumanPlayer* pPlayer, const char* pszName);
	VOID Update();
protected:
	VOID Clear();
private:
	CNameHash m_BossTJHash;
	BOSS_TJ* m_pBossTJList[MAXBOSSTJ];
	int m_nBossTJCount;
	CServerTimer m_tmrUpdate; // 更新计时器
};