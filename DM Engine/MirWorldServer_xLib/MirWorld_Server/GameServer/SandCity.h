#pragma once
#define	MAX_ATTACKREQUEST 100

class CSCPalaceDoor;
class CSCDoor;
class CGuildEx;
class CPalaceWall;
class CSCArcher;
class CHumanPlayer;
class CLogicMap;
class CScriptNpc;

class CSandCity :
	public xSingletonClass<CSandCity>,
	public xError,
	public CTimeEventObject
{
public:
	CSandCity(void);
	virtual ~CSandCity(void);
	BOOL Init();
	CGuildEx* GetOwnerGuild();

	const char* GetName() { return m_szName; }
	//	攻城战控制
	BOOL StartWar();
	BOOL EndWar();
	//	攻城战是否开始
	BOOL IsWarStarted()const { return m_bWarStarted; }

	VOID UpdateWar();

	VOID OnEnterPalace(CHumanPlayer* pPlayer);
	VOID OnLeavePalace(CHumanPlayer* pPlayer);

	VOID ChangeOwner(CGuildEx* pOwner);

	VOID OpenGate();
	VOID CloseGate();
	VOID RepairGate();

	VOID RepairWall(int index);

	VOID Save();

	VOID Home(CHumanPlayer* pPlayer);

	BOOL AddAttackRequest(CGuildEx* pGuild, BOOL bToday = FALSE);

	VOID SaveAttackRequest();
	VOID LoadAttackRequest();

	VOID PrepareAttackGuild(CSystemTime& curTime);

	int PrePareAttackRequestPage(UINT nPage, char* pBuffer);

	BOOL SetSabukMaster(CHumanPlayer* m_pPlayer);

	VOID UpdateSabukMasterFigure();
	TopCharInfo* GetMasterInfo() { return &m_SabukMaster; }
	CSCDoor* GetMainGate() { return m_pMainGate; }

	BOOL AddIncoming(DWORD dwIncoming);
	BOOL AddTotalGold(DWORD dwAddGold);
	BOOL DecTotalGold(DWORD dwDecGold);

	DWORD GetTotalGold()const { return m_dwTotalGold; }
	DWORD GetTodayIncoming()const { return m_dwTodayIncome; }
	VOID SetTexRate(DWORD dwRate) { m_dwTexRate = dwRate; this->m_fTexRate = (FLOAT)m_dwTexRate / 100; }
	DWORD GetTexRate()const { return m_dwTexRate; }

	VOID SetRebate(DWORD dwRebate) { m_dwRebate = dwRebate; }
	DWORD GetRebate()const { return m_dwRebate; }
	const char* GetWarTime() { return m_WarTime.ToString(); }
protected:
	VOID OnHourChange(CSystemTime& curTime);
	VOID StartIdentify();
	VOID StopIdentify();

	VOID ProcIdentify();

	BOOL IdentifyEnd();
	BOOL m_bIdentifyStart;
	CServerTimer m_tmrIdentify;
	CServerTimer m_tmrWar;

	CSCPalaceDoor* m_pPalaceDoor;
	CSCDoor* m_pMainGate;
	CGuildEx* m_pOwnerGuild;

	CPalaceWall* m_pLeftWall;
	CPalaceWall* m_pCenterWall;
	CPalaceWall* m_pRightWall;

	CLogicMap* m_pPalaceMap;
	CSCArcher* m_pArchers[12];

	char m_szName[32];
	DWORD m_dwTotalGold;
	DWORD m_dwTodayIncome;
	CSystemTime m_ChangeTime;
	CSystemTime m_IncomeTime;
	CSystemTime m_WarTime;
	BOOL m_bWarStarted;
	DWORD m_dwHomeX;
	DWORD m_dwHomeY;
	DWORD m_dwHomeMapId;
	DWORD m_dwCastleMapId;
	DWORD m_dwWarRangeX;
	DWORD m_dwWarRangeY;

	DWORD m_dwPalaceMapId;
	DWORD m_dwPalaceDoorX;
	DWORD m_dwPalaceDoorY;
	DWORD m_dwCastlePalaceDoorX;
	DWORD m_dwCastlePalaceDoorY;
	FLOAT m_fTexRate;
	DWORD m_dwTexRate;
	DWORD m_dwRebate;
	CScriptNpc* m_pSabukMaster;
	TopCharInfo m_SabukMaster;

	AttackSabukRequest m_AttackRequest[MAX_ATTACKREQUEST];
	int m_iAttackRequestCount;
	CGuildEx* m_pWarGuild[MAX_ATTACKREQUEST];
	int m_iWarGuildCount;
};