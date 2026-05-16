#pragma once
//#include "scriptpage.h"
#include "localdefine.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <functional>

class CPhysicsMapMgr;
class CLogicMapMgr;
class CHumanPlayerMgr;
class CHumanPlayer;
class CMapObject;
class CItemManager;
class CMagicManager;
class CAliveObject;
class CNpcManager;
class CScriptObjectMgr;

struct tagOBJECTPROCESS;
typedef struct tagSTRING_BUFFER
{
	char szString[1024];
}STRING_BUFFER;

typedef xListHost<CMapObject> MAPOBJECT_LIST;

typedef struct firstlogin_item
{
	firstlogin_item()
	{
		FILLSELF(0);
	}
	char szItem[32];
	int	nCount;
	BYTE btJob;
	BYTE btSex;
	BOOL boBind;
	firstlogin_item* pNext;
}FIRSTLOGIN_ITEM;

typedef struct firstlogin_info
{
	firstlogin_info()
	{
		FILLSELF(0);
	}
	int	level;
	int nGameTime;
	DWORD dwGold;
	DWORD dwYuanBao;
	FIRSTLOGIN_ITEM* pItems;
}FIRSTLOGIN_INFO;

typedef struct linenotice
{
	CServerTimer timer;
	DWORD dwDelay = 0;
	char szWords[256] = { 0 };
	linenotice() : dwDelay(0)
	{
		memset(szWords, 0, sizeof(szWords));
	}
}LINENOTICE;

class CMonsterEx;

class CGameWorld : public xSingletonClass<CGameWorld>
{
	xListHost<CMonsterEx> m_xUpdateMonsterList; // 有目标的怪
	xListHost<CMonsterEx> m_xUpdateAutoMonsterList; // 闲散无目标的怪
	CRITICAL_SECTION m_csMonsterUpdateLock; // 保护怪物列表切换的锁
	static const int MONSTER_LOCK_SEGMENTS = 64; // 分段锁数量
	CRITICAL_SECTION m_csMonsterSegmentLocks[MONSTER_LOCK_SEGMENTS]; // 分段锁
private:
	enum MonsterUpdateType { MUT_AUTO, MUT_ACTIVE };
	VOID UpdateMonster(xListHost<CMonsterEx>& monsterList, MonsterUpdateType updateType, int nStart = 0, int nEnd = 0);
	VOID UpdatePlayers();
public:
	UINT GetUpdateMonsterListCount() 
	{ 
		EnterCriticalSection(&m_csMonsterUpdateLock);
		UINT count = m_xUpdateMonsterList.getCount() + m_xUpdateAutoMonsterList.getCount();
		LeaveCriticalSection(&m_csMonsterUpdateLock);
		return count;
	}
	CGameWorld(void);
	virtual ~CGameWorld(void);

	VOID AddUpdateMonster(CMonsterEx* pMonster);
	// 获取怪物的分段锁索引
	int GetMonsterLockIndex(CMonsterEx* pMonster);

	VOID ShowSandCityNpc();
	VOID HideSandCityNpc();

	VOID ShowNpc(UINT nMapId);
	VOID HideNpc(UINT nMapId);

	BOOL LoadServerConfig();
	VOID LoadSafeArea();
	VOID LoadStartPoint();
	VOID LoadNotice();
	VOID LoadClientKeyConfig();
	BOOL Init();

	VOID Update();

	//工作线程池
	VOID InitThreadPool();
	VOID ShutdownThreadPool();
	VOID AsyncUpdateWorker();
	VOID ProcessAsyncUpdateResults();
	VOID SubmitAsyncTask(std::function<void()> task);

	BOOL AddMapObject(CMapObject* pObject);
	BOOL RemoveMapObject(CMapObject* pObject);

	VOID CleanAllMonsters();
#ifdef USE_FREE_MEMORY
	OBJECTPROCESS* AllocProcess(const char* pszString = nullptr) {
		OBJECTPROCESS* p = new OBJECTPROCESS;
		if (p == nullptr)return p;
		memset(p, 0, sizeof(*p));
		if (pszString != nullptr)
		{
			p->pszParam = new char[1024];
			if (p->pszParam == nullptr)
			{
				MessageBox(nullptr, "分配内存失败, 服务器将自动关闭, 请删除Market_Save下的所有文件, 重新启动服务器!", "错误", 0);
				throw 1;
			}
			o_strncpy(p->pszParam, pszString, 1023);
		}
		return p;
	}
	VOID	FreeProcess(OBJECTPROCESS* p) {
		if (p == nullptr)
		{
			MessageBox(nullptr, "无法确定的内存问题, 请记录当前的服务器物理内存和虚拟内存占用, 报告到论坛, 谢谢!", "错误", 0);
			throw 1;
		}
		if (p->pszParam != nullptr)
		{
			delete[]p->pszParam;
		}
		delete p;
	}
#else
	OBJECTPROCESS* AllocProcess(const char* pszString = nullptr) {
		OBJECTPROCESS* p = m_ObjProcesses.newObject();
		if (p == nullptr)return p;
		memset(p, 0, sizeof(*p));
		if (pszString != nullptr)
		{
			STRING_BUFFER* psb = m_StringBufferPool.newObject();
			if (psb == nullptr)
			{
				m_ObjProcesses.deleteObject(p);
				return nullptr;
			}
			o_strncpy(psb->szString, pszString, 1023);
			p->pszParam = psb->szString;
		}
		return p;
	}

	VOID FreeProcess(OBJECTPROCESS* p) {
		if (p->pszParam != nullptr)
		{
			STRING_BUFFER* psb = (STRING_BUFFER*)p->pszParam;
			m_StringBufferPool.deleteObject(psb);
			p->pszParam = nullptr;
		}
		m_ObjProcesses.deleteObject(p);
	}
#endif
	const char* GetNotice() { return m_pNotice; }

	BOOL GetStartPoint(int pro, int& mapid, int& x, int& y);

	HUMANDATADESC* GetHumanDataDesc(int pro, int level)
	{
		if (pro < 0 || pro >= PRO_MAX)return nullptr;
		if (level <= 0 || level > MAX_LEVEL)return nullptr;
		return &m_HumanData[pro][level - 1];
	}
	FIRSTLOGIN_INFO* GetFirstLoginInfo() { return &m_FirstLoginInfo; }

	CAliveObject* GetAliveObjectById(UINT id);
	const char* GetName(e_nameindex index) { if (index < 0 || index >= ENI_MAX)return "<nullptr>"; return m_pNameList[index]; }
	DWORD GetVar(e_varindex index)const { if (index < 0 || index >= EVI_MAX)return 0; return m_VarList[index]; }
	BOOL AddGlobeProcess(e_process ident, DWORD dwParam1 = 0, DWORD dwParam2 = 0, DWORD dwParam3 = 0, DWORD dwParam4 = 0,
		DWORD dwDelay = 0, int repeattimes = 0, const char* pszString = nullptr);
public:
	VOID PostSystemMessage(const char* pszWords, DWORD dwDelay = 0);
	VOID PostSystemMessage(DWORD dwColor, const char* pszWords, DWORD dwDelay = 0);
	VOID PostSystem10Message(const char* pszWords, DWORD dwDelay = 0);

	USERMAGIC* AllocUserMagic();
	VOID FreeUserMagic(USERMAGIC* p);

	BOOL GetBornPoint(int pro, int& mapid, int& x, int& y, char* pszName);
	BOOL GetStartPoint(const char* pszName, int& mapid, int& x, int& y);

	START_POINT* GetBornPoint(int pro)const;
	START_POINT* GetStartPoint(DWORD dwIndex)
	{
		if (m_pStartPoints == nullptr)return nullptr;
		if (dwIndex >= (DWORD)m_iStartPointCount)
			return nullptr;
		return &m_pStartPoints[dwIndex];
	}

	START_POINT* GetStartPoint(const char* pszName)
	{
		return (START_POINT*)m_StartPointHash.HGet(pszName);
	}
	FLOAT GetExpFactor()const { return m_fExpFactor; }
	VOID SetExpFactor(FLOAT fFactor) { m_fExpFactor = fFactor; }
	DWORD GetChannelWaitTime(e_chatchannel channel)const
	{
		if (channel < 0 || channel >= CCH_MAX)channel = (e_chatchannel)0;
		return m_dwChannelTime[channel] * 1000;
	}
	DWORD GetUpdateKey()const { return m_dwUpdateKey; }
	VOID SetUpdateKey(DWORD dwKey) { m_dwUpdateKey = dwKey; }
	BOOL CanSaveToDB() { if (m_DBUpdateTimer.IsTimeOut(GetVar(EVI_DBUPDATEDELAY)))return TRUE; return FALSE; }
	VOID UpdateDBUpdateTimer() { m_DBUpdateTimer.Savetime(); }
	int GetMonsterQueue()const { return m_iMonsterQueue; }
	int GetNpcQueue()const { return m_iNpcQueue; }
	int GetPlayerQueue()const { return m_iPlayerQueue; }
	ClientKeyState* GetClientKeyConfig() { return m_ClientKeyConfig; }
private:
	FLOAT m_fExpFactor;
	BOOL GetValidPointFromStartPoint(START_POINT* pPoint, int& map, int& x, int& y);
	char* m_pNameList[ENI_MAX];
	DWORD m_VarList[EVI_MAX];

	DWORD m_dwChannelTime[CCH_MAX];
	DWORD m_dwUpdateKey;

	BOOL LoadHumanDataDesc(int pro, const char* pszFile);
	char* m_pNotice;
	MAPOBJECT_LIST* m_pObjectList[OBJ_MAX];
	xPtrQueue<tagOBJECTPROCESS>	m_xGlobeProcessQueue;
	xObjectPool	<tagOBJECTPROCESS> m_ObjProcesses;
	xObjectPool <STRING_BUFFER> m_StringBufferPool;
	xObjectPool<USERMAGIC> m_UserMagicPool;
	START_POINT** m_BornPoints[3];
	int	m_iBornPointCount[3];
	CSettingFile m_sfServer;
	HUMANDATADESC m_HumanData[PRO_MAX][MAX_LEVEL];
	FIRSTLOGIN_INFO m_FirstLoginInfo;
	CServerTimer m_LineNoticeTimer;
	START_POINT* m_pStartPoints;
	int	m_iStartPointCount;
	CNameHash m_StartPointHash;
	LINENOTICE m_NoticeLines[1024];
	int	m_iNoticeLines;
	int	m_iNoticePtr;
	CServerTimer m_DBUpdateTimer;
	int m_iMonsterQueue;
	int m_iNpcQueue;
	int m_iPlayerQueue;

	ClientKeyState m_ClientKeyConfig[100]; // 自定义快捷键

	// 工作线程池相关
	std::vector<std::thread> m_WorkerThreads;
	std::queue<std::function<void()>> m_WorkerTaskQueue;
	std::mutex m_WorkerQueueMutex;
	std::condition_variable m_WorkerQueueCV;
	std::atomic<bool> m_bWorkerRunning;
	std::atomic<bool> m_bAsyncUpdateReady;

	// 异步通告结果缓冲
	struct AsyncNoticeResult {
		char szText[1024];
		int len;
		DWORD dwDelay;
	};
	std::queue<AsyncNoticeResult> m_AsyncNoticeQueue;
	std::mutex m_AsyncNoticeMutex;
};
