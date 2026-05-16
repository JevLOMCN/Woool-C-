#include "StdAfx.h"
#include ".\gameworld.h"
#include "physicsmapmgr.h"
#include "logicmapmgr.h"
#include "ScriptVariable.h"
#include "humanplayermgr.h"
#include "itemmanager.h"
#include "server.h"
#include "magicmanager.h"
#include "scriptobjectmgr.h"
#include "npcmanager.h"
#include "gmmanager.h"
#include "StringListManager.h"
#include "fmttextfile.h"
#include "titlemanager.h"
#include "FengHaoGrowManager.h"
#include "downitemmgr.h"
#include "BundleManager.h"
#include "eventmanager.h"
#include "BossTJ.h"
#include "TimeAchieve.h"
#include "GameStage.h"
#include "guildex.h"
#include "guildmanagerex.h"
#include "monsterex.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>

///////////// monsters ////////////////////
#include "monstermanagerex.h"
#include "monstergenmanager.h"
#include "monitemsmgr.h"
#include "sandcity.h"
#include "scriptnpc.h"
#include "timesystem.h"
#include "systemscript.h"
#include "topmanager.h"
#include ".\specialequipmentmanager.h"
#include "scriptobject.h"
#include "minelist.h"
#include "marketmanager.h"
#include "autoscriptmanager.h"
#include "mapscriptmanager.h"
#include "guildwarmanager.h"
#include "TaskManager.h"

extern DWORD g_dwActionDelay[AT_MAX];

CGameWorld::CGameWorld(void) :
	m_pNotice(nullptr),
	m_pStartPoints(nullptr),
	m_dwUpdateKey(0),
	m_iStartPointCount(0),
	m_iNoticeLines(0),
	m_iNoticePtr(0),
	m_bWorkerRunning(false),
	m_bAsyncUpdateReady(false),
	m_xGlobeProcessQueue(64),
	m_fExpFactor(1.0f),
	m_iPlayerQueue(256),
	m_iMonsterQueue(32),
	m_iNpcQueue(32),
	m_DBUpdateTimer()
{
	memset(m_BornPoints, 0, sizeof(m_BornPoints));
	memset(m_iBornPointCount, 0, sizeof(m_iBornPointCount));
	memset(m_pObjectList, 0, sizeof(m_pObjectList));
	memset(m_pNameList, 0, sizeof(m_pNameList));
	memset(m_VarList, 0, sizeof(m_VarList));
	memset(m_dwChannelTime, 0, sizeof(m_dwChannelTime));
	memset(m_HumanData, 0, sizeof(m_HumanData));
	memset(&m_FirstLoginInfo, 0, sizeof(m_FirstLoginInfo));
	memset(m_NoticeLines, 0, sizeof(m_NoticeLines));

	// 初始化临界区
	InitializeCriticalSection(&m_csMonsterUpdateLock);
	// 初始化分段锁
	for (int i = 0; i < MONSTER_LOCK_SEGMENTS; i++) {
		InitializeCriticalSection(&m_csMonsterSegmentLocks[i]);
	}

	m_pObjectList[OBJ_MONSTER] = new MAPOBJECT_LIST;
	m_pObjectList[OBJ_PLAYER] = new MAPOBJECT_LIST;
	m_pObjectList[OBJ_NPC] = new MAPOBJECT_LIST;
	m_DBUpdateTimer.Savetime();
}

CGameWorld::~CGameWorld(void)
{
	ShutdownThreadPool(); // 关闭工作线程池
	if (m_pObjectList[OBJ_NPC] != nullptr) {
		delete m_pObjectList[OBJ_NPC];  // 释放内存
		m_pObjectList[OBJ_NPC] = nullptr;  // 防止悬挂指针
	}
	if (m_pObjectList[OBJ_PLAYER] != nullptr) {
		delete m_pObjectList[OBJ_PLAYER];  // 释放内存
		m_pObjectList[OBJ_PLAYER] = nullptr;  // 防止悬挂指针
	}
	if (m_pObjectList[OBJ_MONSTER] != nullptr) {
		delete m_pObjectList[OBJ_MONSTER];  // 释放内存
		m_pObjectList[OBJ_MONSTER] = nullptr;  // 防止悬挂指针
	}
	// 删除临界区
	DeleteCriticalSection(&m_csMonsterUpdateLock);
	// 删除分段锁
	for (int i = 0; i < MONSTER_LOCK_SEGMENTS; i++) {
		DeleteCriticalSection(&m_csMonsterSegmentLocks[i]);
	}
}

VOID CGameWorld::ShowNpc(UINT nMapId)//NPC显示
{
	xListHelper<CMapObject>	objlist(m_pObjectList[OBJ_NPC]);
	CScriptNpc* pNpc = nullptr;
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
	if (pMap == nullptr)return;
	for (pNpc = (CScriptNpc*)objlist.first(); pNpc != nullptr; pNpc = (CScriptNpc*)objlist.next())
	{
		if (pNpc->GetMapId() == nMapId && pNpc->GetMap() == nullptr)
			pMap->AddObject(pNpc);
	}
}

VOID CGameWorld::HideNpc(UINT nMapId)//NPC消失
{
	xListHelper<CMapObject>	objlist(m_pObjectList[OBJ_NPC]);
	CScriptNpc* pNpc = nullptr;
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
	if (pMap == nullptr)return;
	for (pNpc = (CScriptNpc*)objlist.first(); pNpc != nullptr; pNpc = (CScriptNpc*)objlist.next())
	{
		if (pNpc->GetMapId() == nMapId && pNpc->GetMap() == pMap)
			pNpc->GetMap()->RemoveObject(pNpc);
	}
}

VOID CGameWorld::ShowSandCityNpc()
{
	xListHelper<CMapObject>	objlist(m_pObjectList[OBJ_NPC]);
	CScriptNpc* pNpc = nullptr;
	CLogicMap* pMap = nullptr;
	for (pNpc = (CScriptNpc*)objlist.first(); pNpc != nullptr; pNpc = (CScriptNpc*)objlist.next())
	{
		if (pNpc->IsSandCityMerchant())
		{
			pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(pNpc->GetMapId());
			if (pMap)pMap->AddObject(pNpc);
		}
	}
}

VOID CGameWorld::HideSandCityNpc()
{
	xListHelper<CMapObject>	objlist(m_pObjectList[OBJ_NPC]);
	CScriptNpc* pNpc = nullptr;
	CLogicMap* pMap = nullptr;
	for (pNpc = (CScriptNpc*)objlist.first(); pNpc != nullptr; pNpc = (CScriptNpc*)objlist.next())
	{
		if (pNpc->IsSandCityMerchant() /*&& pNpc->GetMap() == pMap */)
		{
			pMap = pNpc->GetMap();
			if (pMap)pMap->RemoveObject(pNpc);
		}
	}
}

#define	LOADVAR( var, defvalue ) m_VarList[EVI_##var] = (DWORD)m_sfServer.GetInteger( "Var", #var, (defvalue) );
#define	LOADNAME( var, defvalue ) m_pNameList[ENI_##var] = (char*)m_sfServer.GetString( "Name", #var, (defvalue) );
BOOL CGameWorld::LoadServerConfig()
{
	m_sfServer.Close();
	if (!m_sfServer.Open(".\\Data\\Server.txt"))
	{
		PRINT(ERROR_RED, "无法打开服务器配置文件 .\\Data\\Server.txt");
		return FALSE;
	}

	g_dwActionDelay[AT_WALK] = (DWORD)m_sfServer.GetInteger("Speed", "Walk", 600);
	g_dwActionDelay[AT_RUN] = (DWORD)m_sfServer.GetInteger("Speed", "Run", 600);
	g_dwActionDelay[AT_ATTACK] = (DWORD)m_sfServer.GetInteger("Speed", "Attack", 850);
	g_dwActionDelay[AT_BEATTACK] = (DWORD)m_sfServer.GetInteger("Speed", "Beattack", 300);

	m_pNameList[ENI_GOLD] = (char*)m_sfServer.GetString("Name", "GoldName", "金币");
	m_pNameList[ENI_MALE] = (char*)m_sfServer.GetString("Name", "MaleName", "男");
	m_pNameList[ENI_FEMALE] = (char*)m_sfServer.GetString("Name", "FemaleName", "女");
	m_pNameList[ENI_GUILDNOTICE] = (char*)m_sfServer.GetString("Name", "GuildNotice", "公告");
	m_pNameList[ENI_KILLGUILDS] = (char*)m_sfServer.GetString("Name", "KillGuilds", "敌对行会");
	m_pNameList[ENI_ALLYGUILDS] = (char*)m_sfServer.GetString("Name", "AllyGuilds", "联盟行会");
	m_pNameList[ENI_MEMBERS] = (char*)m_sfServer.GetString("Name", "Members", "行会成员");
	m_pNameList[ENI_GUILDDECLARATION] = (char*)m_sfServer.GetString("Name", "GuildDeclaration", "行会宣言");
	m_pNameList[ENI_GUILDAPPLYLIST] = (char*)m_sfServer.GetString("Name", "GuildApplyList", "申请列表");
	m_pNameList[ENI_WARR] = (char*)m_sfServer.GetString("Name", "Warr", "战士");
	m_pNameList[ENI_MAGICAN] = (char*)m_sfServer.GetString("Name", "Magican", "法师");
	m_pNameList[ENI_TAOSHI] = (char*)m_sfServer.GetString("Name", "Taoshi", "道士");
	m_pNameList[ENI_TOPOFWORLD] = (char*)m_sfServer.GetString("Name", "TopOfWorld", "天下第一");
	m_pNameList[ENI_UPGRADEMINESTONE] = (char*)m_sfServer.GetString("Name", "UpgradeMineStone", "黑铁矿石");
	LOADNAME(PHYSICSMAPPATH, ".\\Data\\Maps\\Physics");
	LOADNAME(PHYSICSCACHEPATH, ".\\Data\\Maps\\Cache");

	m_fExpFactor = m_sfServer.GetInteger("Setting", "ExpFactor", 100) / 100.0f;//爆率
	m_iPlayerQueue = m_sfServer.GetInteger("Setting", "PlayerQueue", 256);//玩家处理队列数量
	m_iMonsterQueue = m_sfServer.GetInteger("Setting", "MonsterQueue", 32);//怪物处理队列数量
	m_iNpcQueue = m_sfServer.GetInteger("Setting", "NpcQueue", 32);//NPC处理队列数量

	m_VarList[EVI_MAXGOLD] = (DWORD)m_sfServer.GetInteger("Var", "MaxGold", 5000000);
	m_VarList[EVI_MAXBIGGOLD] = (DWORD)m_sfServer.GetInteger("Var", "MaxBigGold", 10000000);
	m_VarList[EVI_MAXYUANBAO] = (DWORD)m_sfServer.GetInteger("Var", "MaxYuanBao", 2000);
	m_VarList[EVI_ITEMUPDATETIME] = (DWORD)m_sfServer.GetInteger("Var", "ItemUpdateTime", 300) * 1000;
	m_VarList[EVI_MAXGROUPMEMBER] = (DWORD)m_sfServer.GetInteger("Var", "MaxGroupMember", 10);
	m_VarList[EVI_REDPKPOINT] = (DWORD)m_sfServer.GetInteger("Var", "RedPkPoint", 12);
	m_VarList[EVI_YELLOWPKPOINT] = (DWORD)m_sfServer.GetInteger("Var", "YellowPkPoint", 6);
	m_VarList[EVI_ONCEPKPOINT] = (DWORD)m_sfServer.GetInteger("Var", "OncePkPoint", 3);
	m_VarList[EVI_ONEPKPOINTTIME] = (DWORD)m_sfServer.GetInteger("Var", "OnePkPointTime", 120);
	m_VarList[EVI_BODYTIME] = (DWORD)m_sfServer.GetInteger("Var", "BodyTime", 60);
	m_VarList[EVI_STOREAGESIZE] = (DWORD)m_sfServer.GetInteger("Var", "StoreageSize", 39);
	m_VarList[EVI_SANDCITYTAKETIME] = (DWORD)m_sfServer.GetInteger("Var", "SandCityTakeTime", 300);
	m_VarList[EVI_WARENEMYCOLOR] = (DWORD)m_sfServer.GetInteger("Var", "WarEnemyColor", 243);
	m_VarList[EVI_WARALLYCOLOR] = (DWORD)m_sfServer.GetInteger("Var", "WarAllyColor", 4);
	m_VarList[EVI_WARNORMALCOLOR] = (DWORD)m_sfServer.GetInteger("Var", "WarNormalColor", 219);
	m_VarList[EVI_WARTIMELONG] = (DWORD)m_sfServer.GetInteger("Var", "WarTimeLong", 240);
	m_VarList[EVI_WARSTARTTIME] = ((DWORD)m_sfServer.GetInteger("Var", "WarStartTime", 20)) % 24;
	m_VarList[EVI_ENABLESAFEAREANOTICE] = (DWORD)m_sfServer.GetInteger("Var", "EnableSafeAreaNotice", 0);
	m_VarList[EVI_ADDFRIENDLEVEL] = (DWORD)m_sfServer.GetInteger("Var", "AddFriendLevel", 7);
	m_VarList[EVI_GUILDLEVELNUM] = (DWORD)m_sfServer.GetInteger("Var", "GuildLevelNum", 9);
	LOADVAR(PRIVATESHOPLEVEL, 20);
	LOADVAR(INITDRESSCOLOR, -1);
	LOADVAR(REPAIRDAMAGEDURA, 1000);
	LOADVAR(GRAYNAMETIME, 60);
	LOADVAR(DROPTARGETDISTANCE, 12);
	LOADVAR(WEAPONDAMAGERATE, 15);
	LOADVAR(DRESSDAMAGERATE, 15);
	LOADVAR(DEFENCEDAMAGERATE, 15);
	LOADVAR(JEWELRYDAMAGERATE, 15);
	LOADVAR(RANDOMUPGRADEITEMRATE, 16);
	LOADVAR(CHARINFOBACKUPTIME, 30);
	LOADVAR(DBUPDATEDELAY, 2000);
	LOADVAR(MAXUPGRADETIMES, 10);
	LOADVAR(MONGENFACTOR, 100);
	LOADVAR(PKCURSERATE, 50);
	LOADVAR(HPRECOVERPOINT, 9);
	LOADVAR(HPRECOVERTIME, 500);
	LOADVAR(MPRECOVERPOINT, 9);
	LOADVAR(MPRECOVERTIME, 500);
	LOADVAR(GUILDWARTIME, 3600 * 3);
	LOADVAR(STARTGUILDMEMBERCOUNT, 64);
	LOADVAR(DROPTARGETTIME, 3);

	m_dwChannelTime[CCH_NORMAL] = (DWORD)m_sfServer.GetInteger("ChatWait", "Normal", 1);
	m_dwChannelTime[CCH_CRY] = (DWORD)m_sfServer.GetInteger("ChatWait", "Cry", 10);
	m_dwChannelTime[CCH_WISPER] = (DWORD)m_sfServer.GetInteger("ChatWait", "Whisper", 2);
	m_dwChannelTime[CCH_GROUP] = (DWORD)m_sfServer.GetInteger("ChatWait", "Group", 2);
	m_dwChannelTime[CCH_GUILD] = (DWORD)m_sfServer.GetInteger("ChatWait", "Guild", 3);
	m_dwChannelTime[CCH_COUPLE] = (DWORD)m_sfServer.GetInteger("ChatWait", "Couple", 1);
	m_dwChannelTime[CCH_GM] = (DWORD)m_sfServer.GetInteger("ChatWait", "Gm", 0);
	m_dwChannelTime[CCH_FRIEND] = (DWORD)m_sfServer.GetInteger("ChatWait", "Friend", 2);

	CStringListManager::GetInstance()->ClearAll();
	CStringListManager::GetInstance()->Load(".\\Data\\StringList");

	//	读取角色信息文件
	char* pszHumanDescFile = (char*)m_sfServer.GetString("HumanData", "Warrior", nullptr);
	if (pszHumanDescFile != nullptr)
	{
		if (!LoadHumanDataDesc(PRO_WARRIOR, pszHumanDescFile))
			PRINT(ERROR_RED, "无法打开角色信息文件: %s\n", pszHumanDescFile);
		else
			PRINT(TEXT_WHITE, "角色信息文件 %s 已读取.\n", pszHumanDescFile);
	}
	else
		PRINT(ERROR_RED, "无法得到 %s 的角色信息文件名.\n", "Warrior");
	pszHumanDescFile = (char*)m_sfServer.GetString("HumanData", "Magician", nullptr);
	if (pszHumanDescFile != nullptr)
	{
		if (!LoadHumanDataDesc(PRO_MAGICIAN, pszHumanDescFile))
			PRINT(ERROR_RED, "无法打开角色信息文件: %s\n", pszHumanDescFile);
		else
			PRINT(TEXT_WHITE, "角色信息文件 %s 已读取.\n", pszHumanDescFile);
	}
	else
		PRINT(ERROR_RED, "无法得到 %s 的角色信息文件名.\n", "Magician");

	pszHumanDescFile = (char*)m_sfServer.GetString("HumanData", "Taoshi", nullptr);
	if (pszHumanDescFile != nullptr)
	{
		if (!LoadHumanDataDesc(PRO_TAOSHI, pszHumanDescFile))
			PRINT(ERROR_RED, "无法打开角色信息文件: %s\n", pszHumanDescFile);
		else
			PRINT(TEXT_WHITE, "角色信息文件 %s 已读取.\n", pszHumanDescFile);
	}
	else
		PRINT(ERROR_RED, "无法得到 %s 的角色信息文件名.\n", "Taoshi");
	char* pszTemp = "";
	m_FirstLoginInfo.nGameTime = (DWORD)m_sfServer.GetInteger("FirstLogin", "StartGameTime", -1);
	m_FirstLoginInfo.dwGold = (DWORD)m_sfServer.GetInteger("FirstLogin", "StartGold", 0);
	m_FirstLoginInfo.dwYuanBao = (DWORD)m_sfServer.GetInteger("FirstLogin", "StartYuanBao", 0);
	m_FirstLoginInfo.level = (DWORD)m_sfServer.GetInteger("FirstLogin", "StartLevel", 1);
	pszTemp = (char*)m_sfServer.GetString("FirstLogin", "StartItem", nullptr);
	while (m_FirstLoginInfo.pItems != nullptr)
	{
		FIRSTLOGIN_ITEM* p = m_FirstLoginInfo.pItems;
		m_FirstLoginInfo.pItems = p->pNext;
		delete p;
	}
	if (pszTemp != nullptr)
	{
		char* Params[BIGBAG_SLOT], * Params2[5];
		int nParam = SearchParam(pszTemp, Params, BIGBAG_SLOT, '/');
		int nParam2 = 0;
		for (int i = 0; i < nParam; i++)
		{
			nParam2 = SearchParam(Params[i], Params2, 5, '*');
			FIRSTLOGIN_ITEM* pItem = new FIRSTLOGIN_ITEM;
			if (nParam2 == 1)
			{
				pItem->nCount = 1;
				pItem->btJob = 99;//不限职业
				pItem->btSex = 99;//不限性别
				pItem->boBind = 1;//绑定
			}
			else if (nParam2 <= 2)
			{
				pItem->nCount = StringToInteger(Params2[1]);
				pItem->btJob = 99;
				pItem->btSex = 99;
				pItem->boBind = 1;
			}
			else if(nParam2 <= 3)
			{
				pItem->nCount = StringToInteger(Params2[1]);
				pItem->btJob = StringToInteger(Params2[2]);
				pItem->btSex = 99;
				pItem->boBind = 1;
			}
			else if (nParam2 <= 4)
			{
				pItem->nCount = StringToInteger(Params2[1]);
				pItem->btJob = StringToInteger(Params2[2]);
				pItem->btSex = StringToInteger(Params2[3]);
				pItem->boBind = 1;
			}
			else
			{
				pItem->nCount = StringToInteger(Params2[1]);
				pItem->btJob = StringToInteger(Params2[2]);
				pItem->btSex = StringToInteger(Params2[3]);
				pItem->boBind = StringToInteger(Params2[4]);
			}
			o_strncpy(pItem->szItem, Params2[0], 31);
			pItem->pNext = m_FirstLoginInfo.pItems;
			m_FirstLoginInfo.pItems = pItem;
		}
	}
	return TRUE;
}

VOID CGameWorld::LoadSafeArea()
{
	CFmtTextFile ftfSafeArea("w4", ".\\Data\\Config\\SafeArea.csv", TRUE);
	WORD wSafeArea[4] = { 0 };
	for (int i = 0; i < ftfSafeArea.GetCount(); i++)
	{
		if (ftfSafeArea.GetStruct(i, (LPVOID)wSafeArea))
		{
			CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(wSafeArea[0]);
			if (pMap != nullptr)
			{
				//	设置安全区
				PRINT(COOL_BLUE, "设置 %s 地图的 (%u,%u) 为中心半径 %u 为安全区\n", pMap->GetTitle(), wSafeArea[1], wSafeArea[2], wSafeArea[3]);
				pMap->SetSafeArea(wSafeArea[1], wSafeArea[2], wSafeArea[3]);
			}
		}
	}
}

VOID CGameWorld::LoadStartPoint()
{
	m_iStartPointCount = 0;
	if (m_pStartPoints != nullptr)
		delete[]m_pStartPoints;
	m_pStartPoints = nullptr;
	CFmtTextFile ftfStartPoint("s32w4b3", ".\\Data\\Config\\StartPoint.csv", TRUE);
	if (ftfStartPoint.GetCount() > 0)
	{
		m_pStartPoints = new start_point[ftfStartPoint.GetCount()];
		for (int i = 0; i < ftfStartPoint.GetCount(); i++)
		{
			if (ftfStartPoint.GetStruct(i, &m_pStartPoints[m_iStartPointCount]))
			{
				PRINT(STRING_GREEN, "读取出生点 %s ( %u, %u )[%u] %u\n",
					m_pStartPoints[m_iStartPointCount].szName,
					m_pStartPoints[m_iStartPointCount].x,
					m_pStartPoints[m_iStartPointCount].y,
					m_pStartPoints[m_iStartPointCount].mapid,
					m_pStartPoints[m_iStartPointCount].range);
				CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(m_pStartPoints[m_iStartPointCount].mapid);
				if (pMap == nullptr)
				{
					PRINT(ERROR_RED, "没有该地图号所指的地图, 该出生点作废!\n");
					continue;
				}
				m_pStartPoints[m_iStartPointCount].index = m_iStartPointCount;
				if (!m_StartPointHash.HAdd(m_pStartPoints[m_iStartPointCount].szName, &m_pStartPoints[m_iStartPointCount]))
				{
					PRINT(ERROR_RED, "由于名字重复, 该出生点作废!\n");
					continue;
				}
				pMap->AddStartPoint(&m_pStartPoints[m_iStartPointCount]);
				if (m_pStartPoints[m_iStartPointCount].bFighterBorn)
					m_iBornPointCount[0]++;
				if (m_pStartPoints[m_iStartPointCount].bMagicianBorn)
					m_iBornPointCount[1]++;
				if (m_pStartPoints[m_iStartPointCount].bTaoshiBorn)
					m_iBornPointCount[2]++;

				m_iStartPointCount++;
			}
		}

		if (m_BornPoints[0])
			delete[]m_BornPoints[0];
		if (m_BornPoints[1])
			delete[]m_BornPoints[1];
		if (m_BornPoints[2])
			delete[]m_BornPoints[2];

		m_BornPoints[0] = new START_POINT * [m_iBornPointCount[0]];
		m_BornPoints[1] = new START_POINT * [m_iBornPointCount[1]];
		m_BornPoints[2] = new START_POINT * [m_iBornPointCount[2]];
		m_iBornPointCount[0] = 0;
		m_iBornPointCount[1] = 0;
		m_iBornPointCount[2] = 0;

		for (int i = 0; i < m_iStartPointCount; i++)
		{
			if (m_pStartPoints[i].bFighterBorn)
			{
				m_BornPoints[0][m_iBornPointCount[0]++] = m_pStartPoints + i;
				PRINT(STRING_GREEN, "战士出生点 %s ( %u, %u )[%u] %u\n",
					m_pStartPoints[i].szName,
					m_pStartPoints[i].x,
					m_pStartPoints[i].y,
					m_pStartPoints[i].mapid,
					m_pStartPoints[i].range
				);
			}
			if (m_pStartPoints[i].bMagicianBorn)
			{
				m_BornPoints[1][m_iBornPointCount[1]++] = m_pStartPoints + i;
				PRINT(STRING_GREEN, "法师出生点 %s ( %u, %u )[%u] %u\n",
					m_pStartPoints[i].szName,
					m_pStartPoints[i].x,
					m_pStartPoints[i].y,
					m_pStartPoints[i].mapid,
					m_pStartPoints[i].range
				);
			}
			if (m_pStartPoints[i].bTaoshiBorn)
			{
				m_BornPoints[2][m_iBornPointCount[2]++] = m_pStartPoints + i;
				PRINT(STRING_GREEN, "道士出生点 %s ( %u, %u )[%u] %u\n",
					m_pStartPoints[i].szName,
					m_pStartPoints[i].x,
					m_pStartPoints[i].y,
					m_pStartPoints[i].mapid,
					m_pStartPoints[i].range
				);
			}
		}

	}
}

VOID CGameWorld::LoadNotice()
{
	if (m_pNotice != nullptr)
		delete[]m_pNotice;
	m_pNotice = (char*)LoadFile(".\\Data\\Config\\Notice.txt");
	if (m_pNotice == nullptr)
	{
		m_pNotice = " \n经典游戏  精彩无限";
	}
	CStringFile sfLN(".\\Data\\Config\\LineNotice.txt");
	m_iNoticeLines = 0;
	m_iNoticePtr = 0;
	for (int i = 0; i < (sfLN.GetLineCount() > 1024 ? 1024 : sfLN.GetLineCount()); i++)
	{
		if (*sfLN[i] == '#')continue;
		xStringsExpander<10> ss(sfLN[i], '|');
		if (ss.getCount() == 1)
		{
			o_strncpy(m_NoticeLines[i].szWords, ss[0], 255);
			m_NoticeLines[m_iNoticeLines].dwDelay = 300000;
			m_NoticeLines[m_iNoticeLines].timer.Savetime();
			m_iNoticeLines++;
		}
		else if (ss.getCount() >= 2)
		{
			o_strncpy(m_NoticeLines[m_iNoticeLines].szWords, ss[1], 255);
			m_NoticeLines[m_iNoticeLines].dwDelay = 1000 * StringToInteger(ss[0]);
			m_NoticeLines[m_iNoticeLines].timer.Savetime();
			m_iNoticeLines++;
		}
	}
	m_LineNoticeTimer.Savetime();
}

VOID CGameWorld::LoadClientKeyConfig()
{
	memset(m_ClientKeyConfig, 0, sizeof(m_ClientKeyConfig));
	FILE* fp = fopen(".\\Data\\Config\\ClientKeyConfig.json", "rb");
	if (fp == nullptr)
	{
		PRINT(ERROR_RED, "无法打开 ClientKeyConfig.json 文件\n");
		return;
	}
	char readBuffer[6144]{};
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document doc;
	doc.ParseStream(is);
	fclose(fp);
	if (doc.HasParseError())
	{
		PRINT(ERROR_RED, "ClientKeyConfig.json 解析错误: %d\n", doc.GetParseError());
		return;
	}
	if (!doc.IsArray())
	{
		PRINT(ERROR_RED, "ClientKeyConfig.json 格式错误: 根节点应为数组\n");
		return;
	}
	int nCount = doc.Size();
	if (nCount > 100) nCount = 100;
	for (int i = 0; i < nCount; i++)
	{
		const rapidjson::Value& item = doc[i];
		if (!item.IsObject()) continue;
		m_ClientKeyConfig[i].Key1 = item.HasMember("Key1") && item["Key1"].IsString() ? (BYTE)std::stoi(item["Key1"].GetString(), nullptr, 16) : 0;
		m_ClientKeyConfig[i].Key2 = item.HasMember("Key2") && item["Key2"].IsString() ? (BYTE)std::stoi(item["Key2"].GetString(), nullptr, 16) : 0;
		m_ClientKeyConfig[i].unknow = item.HasMember("unknow") && item["unknow"].IsString() ? (WORD)std::stoi(item["unknow"].GetString(), nullptr, 16) : 0;
	}
}

BOOL CGameWorld::Init()
{
	// 脚本系统首先读取
	CTimeSystem::GetInstance();
	CScriptObjectMgr::GetInstance()->Load(".\\Data\\Script");
	CScriptVariableManager::GetInstance()->StartFind(".\\Data\\Variables", "*.txt", TRUE);
	CScriptObject* pScriptObject = CScriptObjectMgr::GetInstance()->GetScriptObject("system");
	if (pScriptObject)
		CSystemScript::GetInstance()->Init(pScriptObject);
	else
	{
		pScriptObject = new CScriptObject();
		CSystemScript::GetInstance()->Init(pScriptObject);
	}
	if (!LoadServerConfig())return FALSE;
	CPhysicsMapMgr::GetInstance()->Init(GetName(ENI_PHYSICSMAPPATH), GetName(ENI_PHYSICSCACHEPATH));
	CLogicMapMgr::GetInstance()->Load(".\\Data\\Maps\\Logic");
	// 地图密切相关的东西
	LoadSafeArea();
	LoadStartPoint();
	LoadNotice();
	LoadClientKeyConfig();

	CItemManager::GetInstance()->Load(".\\Data\\Config\\BaseItem.csv");
	CItemManager::GetInstance()->LoadLimit(".\\Data\\Config\\ItemLimit.txt");
	CItemManager::GetInstance()->LoadScriptLink(".\\Data\\ItemScript.txt");
	CItemManager::GetInstance()->LoadPetINI(".\\Data\\Config\\PetLevel.ini");
	CMagicManager::GetInstance()->LoadMaigc(".\\Data\\Config\\BaseMagic.csv");
	CMagicManager::GetInstance()->LoadMagicExt(".\\Data\\Config\\MagicExt.csv", TRUE);
	CMagicManager::GetInstance()->LoadMaigcskill(".\\Data\\Config\\MagicSkill.xml");
	CTitleManager::GetInstance()->LoadData(".\\Data\\Config\\Titles.csv", TRUE);
	CFengHaoGrowManager::GetInstance()->LoadData(".\\Data\\Config\\FengHaoGrow.csv", TRUE);

	CBundleManager::GetInstance()->LoadBundle(".\\Data\\Config\\BundleItem.csv", TRUE);

	CNpcManager::GetInstance()->Load(".\\Data\\NpcGens.csv");
	CGmManager::GetInstance()->Load(".\\Data\\GameMaster\\GmList.txt");
	CGmManager::GetInstance()->LoadCommandDef(".\\Data\\GameMaster\\CmdList.txt");

	CGuildManagerEx::GetInstance()->LoadGuildUpExp(".\\Data\\GuildBase\\GuildInfo.ini", GetVar(EVI_GUILDLEVELNUM));
	CGuildManagerEx::GetInstance()->StartFind(".\\Data\\GuildBase\\Guilds", "*.ini", FALSE);

	CMonItemsMgr::GetInstance()->LoadMonItems(".\\Data\\MonItems");
	CMonsterManagerEx::GetInstance()->LoadMonsters(".\\Data\\Monsters");
	CMonsterManagerEx::GetInstance()->LoadMonsterScript(".\\Data\\MonsterScript.txt");
	CMonsterGenManager::GetInstance()->LoadMonGen(".\\Data\\MonGens");
	CMonsterGenManager::GetInstance()->InitAllGen();

	CSandCity::GetInstance()->Init();

	CTopManager::GetInstance()->LoadFigure(".\\Data\\Figure\\TopNpc.txt");
	CTopManager::GetInstance()->Load(".\\Data\\Figure\\TopList.txt");

	CSpecialEquipmentManager::GetInstance()->LoadSpecialEquipmentFunction(".\\Data\\Config\\SpecialItem.txt");
	LoadMineList(".\\Data\\Config\\MineList.txt");

	CMarketManager::GetInstance()->LoadScrollText(".\\Data\\Market\\ScrollText.txt");
	CMarketManager::GetInstance()->LoadMarkets(".\\Data\\Market\\MainDir.txt");
	CAutoScriptManager::GetInstance()->Load(".\\Data\\AutoScript.txt");
	CMapScriptManager::GetInstance()->Load(".\\Data\\MapScript.txt");
	CTaskManager::GetInstance()->Load(".\\Data\\Task");
	CBossTJ::GetInstance()->Load(".\\Data\\Config\\BossTJ.xml");
	CTimeAchieve::GetInstance()->Load(".\\Data\\Config\\TimeAchieve.xml");
	CGameStage::GetInstance()->Load(".\\Data\\Config\\GameStage.xml");

	InitThreadPool(); // 初始化工作线程池
	return TRUE;
}

VOID CGameWorld::InitThreadPool()
{
	if (m_bWorkerRunning) return;
	m_bWorkerRunning = true;
	m_bAsyncUpdateReady = false;

	// 获取系统CPU亲和性掩码
	DWORD_PTR systemMask = 0;
	DWORD_PTR processMask = 0;
	GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask);

	// 创建工作线程, 根据CPU核心数决定线程数量
	int threadCount = std::thread::hardware_concurrency();
	if (threadCount < 2) threadCount = 2; // 至少2个线程
	if (threadCount >= 2) threadCount = 4; // 最多4个工作线程
	for (int i = 0; i < threadCount; ++i)
	{
		m_WorkerThreads.emplace_back([this, i, threadCount, systemMask]() {
			// 循环分配核心：线程i绑定到核心 (i % 可用核心数)
			int coreNum = 0;
			int assignedCore = -1;
			// 计算 systemMask 中置位的位数
			int activeCoreCount = 0;
			DWORD_PTR tempMask = systemMask;
			while (tempMask)
			{
				activeCoreCount += tempMask & 1;
				tempMask >>= 1;
			}
			for (int j = 0; j < 64; j++)
			{
				if (systemMask & (DWORD_PTR(1) << j))
				{
					if (coreNum == (i % activeCoreCount))
					{
						assignedCore = j;
						break;
					}
					coreNum++;
				}
			}
			if (assignedCore >= 0)
			{
				DWORD_PTR affinityMask = DWORD_PTR(1) << assignedCore;
				SetThreadAffinityMask(GetCurrentThread(), affinityMask);
			}
			AsyncUpdateWorker();
		});
	}
	PRINT(ORANGE, "工作线程池已初始化, 线程数: %d\n", threadCount);
}

VOID CGameWorld::ShutdownThreadPool()
{
	if (!m_bWorkerRunning) return;
	m_bWorkerRunning = false;
	// 唤醒所有工作线程
	m_WorkerQueueCV.notify_all();
	// 等待所有工作线程结束
	for (auto& thread : m_WorkerThreads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	m_WorkerThreads.clear();
	// 清空任务队列
	{
		std::lock_guard<std::mutex> lock(m_WorkerQueueMutex);
		while (!m_WorkerTaskQueue.empty()) {
			m_WorkerTaskQueue.pop();
		}
	}
	// 清空通告结果队列
	{
		std::lock_guard<std::mutex> lock(m_AsyncNoticeMutex);
		while (!m_AsyncNoticeQueue.empty()) {
			m_AsyncNoticeQueue.pop();
		}
	}
}

VOID CGameWorld::PostSystemMessage(const char* pszWords, DWORD dwDelay)
{
	char sztext[1024];
	int len = EncodeMsg(sztext, 0, SM_SYSCHAT, 0x38ff, 0, 0, (LPVOID)pszWords);
	AddGlobeProcess(EP_SENDCODEDTEXT, 0, 0, 0, 0, dwDelay, 1, sztext);
}

VOID CGameWorld::PostSystemMessage(DWORD dwColor, const char* pszWords, DWORD dwDelay)
{
	char sztext[1024];
	int len = EncodeMsg(sztext, 0, SM_SYSCHAT, dwColor & 0xffff, dwColor >> 16, 0, (LPVOID)pszWords);
	AddGlobeProcess(EP_SENDCODEDTEXT, 0, 0, 0, 0, dwDelay, 1, sztext);
}

VOID CGameWorld::PostSystem10Message(const char* pszWords, DWORD dwDelay)
{
	char sztext[1024];
	int len = EncodeMsg(sztext, 0, SM_SYSCHAT, 0, 0, 10, (LPVOID)pszWords);
	AddGlobeProcess(EP_SENDCODEDTEXT, 0, 0, 0, 0, dwDelay, 1, sztext);
}

VOID CGameWorld::Update()
{
	//分帧更新
	DWORD dwkey = (m_dwUpdateKey % 10);
	switch (dwkey)
	{
	case 0: case 5:  // 因为 VOID CAliveObject::Update() 更新分了奇数偶数分帧更新
	{
		// 根据怪物数量动态调整分批数量
		int nCount = m_xUpdateAutoMonsterList.getCount();
		int nBatchCount = 1;  // 默认不分批
		if (nCount > 10000)
			nBatchCount = 8;      // 10000+ 怪物分8批
		else if (nCount > 5000)
			nBatchCount = 6;      // 5000+ 怪物分6批
		else if (nCount > 2000)
			nBatchCount = 4;      // 2000+ 怪物分4批
		else if (nCount > 800)
			nBatchCount = 2;      // 800+ 怪物分2批

		if (nBatchCount > 1)
		{
			int nPer = nCount / nBatchCount;
			int nRemainder = nCount % nBatchCount;
			int nCurrentStart = 0;
			for (int i = 0; i < nBatchCount; i++)
			{
				int nBatchSize = nPer + (i < nRemainder ? 1 : 0);
				int nEnd = nCurrentStart + nBatchSize;
				if (nBatchSize > 0)
				{
					SubmitAsyncTask([this, nCurrentStart, nEnd]() {
						UpdateMonster(m_xUpdateAutoMonsterList, MUT_AUTO, nCurrentStart, nEnd);
						});
				}
				nCurrentStart = nEnd;
			}
		}
		else
			UpdateMonster(m_xUpdateAutoMonsterList, MUT_AUTO);// 闲散怪物线程
	}
	break;
	case 1:
	{
		CDownItemMgr::GetInstance()->UpdateDownItem();//更新掉落物品
		SubmitAsyncTask([]() {
				CBossTJ::GetInstance()->Update(); // Boss图鉴刷新时间更新
			});
	}
	break;
	case 2:
	{
		CEventManager::GetInstance()->UpdateEvents();
		SubmitAsyncTask([]() {
			CDownItemMgr::GetInstance()->UpdateDeletedObject();
			});
		SubmitAsyncTask([]() {
			CMonsterManagerEx::GetInstance()->UpdateFreeObjects(); // 释放
			CMonsterManagerEx::GetInstance()->UpdateDeleteMonster(); // 删除
			});
	}
	break;
	case 3:
	{
		CEventManager::GetInstance()->UpdateDeleteObject();
		//沙城行会战争
		CSandCity* pSandCity = CSandCity::GetInstance();
		if (pSandCity && pSandCity->IsWarStarted())
			CSandCity::GetInstance()->UpdateWar();
		CGuildWarManager::GetInstance()->Update();
		SubmitAsyncTask([]() {
			CMonsterGenManager::GetInstance()->UpdateGen(); // 刷怪
			});
	}
	case 6:
	{
		CTimeSystem::GetInstance()->Update(); // 时间定时器
		CNpcManager::GetInstance()->Update(); // NPC线程
		// 处理异步更新结果（如果有）
		if (m_bAsyncUpdateReady) ProcessAsyncUpdateResults();
		if (m_iNoticeLines > 0)
		{
			if (m_LineNoticeTimer.IsTimeOut(m_NoticeLines[m_iNoticePtr].dwDelay))
			{
				SubmitAsyncTask([this]() { 	// 将通告处理提交到工作线程
					char sztext[1024];
					int len = 0;
					DWORD dwDelay = m_NoticeLines[m_iNoticePtr].dwDelay;

					if (m_NoticeLines[m_iNoticePtr].szWords[0] != 0)
					{
						len = EncodeMsg(sztext, 0, 0X64, 0x38ff, 0, 0, (LPVOID)m_NoticeLines[m_iNoticePtr].szWords);
						// 将结果放入队列供主线程处理
						std::lock_guard<std::mutex> lock(m_AsyncNoticeMutex);
						AsyncNoticeResult result{};
						memcpy(result.szText, sztext, 1024);
						result.len = len;
						result.dwDelay = 50;
						m_AsyncNoticeQueue.push(result);
						m_bAsyncUpdateReady = true;
					}
					m_iNoticePtr++;
					if (m_iNoticePtr >= m_iNoticeLines)
						m_iNoticePtr = 0;
					});

				m_LineNoticeTimer.Savetime();
			}
		}
	}
	break;
	case 4: case 7: // 因为 VOID CAliveObject::Update() 更新分了奇数偶数分帧更新
	{
		UpdateMonster(m_xUpdateMonsterList, MUT_ACTIVE);// 有目标的怪线程
	}
	break;
	}
	UpdatePlayers(); // 玩家线程
}

VOID CGameWorld::AsyncUpdateWorker()
{
	while (m_bWorkerRunning)
	{
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(m_WorkerQueueMutex);
			m_WorkerQueueCV.wait(lock, [this] { return !m_WorkerTaskQueue.empty() || !m_bWorkerRunning; });
			if (!m_bWorkerRunning) break;
			if (!m_WorkerTaskQueue.empty())
			{
				task = std::move(m_WorkerTaskQueue.front());
				m_WorkerTaskQueue.pop();
			}
		}
		if (task)task();
	}
}

VOID CGameWorld::ProcessAsyncUpdateResults()
{
	std::lock_guard<std::mutex> lock(m_AsyncNoticeMutex);
	int processCount = 0; // 限制每帧最多处理 10 个
	while (!m_AsyncNoticeQueue.empty() && processCount < 10) {
		AsyncNoticeResult result = m_AsyncNoticeQueue.front();
		m_AsyncNoticeQueue.pop();
		// 在主线程中发送消息（避免跨线程直接调用）
		AddGlobeProcess(EP_SENDCODEDTEXT, 0, 0, 0, 0, result.dwDelay, 1, result.szText);
		processCount++;
	}
	m_bAsyncUpdateReady = !m_AsyncNoticeQueue.empty(); // 如果还有未处理的, 标记为需要继续处理
}

VOID CGameWorld::SubmitAsyncTask(std::function<void()> task)
{
	if (!m_bWorkerRunning) return;
	{
		std::lock_guard<std::mutex> lock(m_WorkerQueueMutex);
		m_WorkerTaskQueue.push(std::move(task));
	}
	m_WorkerQueueCV.notify_all();
}

BOOL CGameWorld::AddGlobeProcess(e_process ident, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3, DWORD dwParam4, DWORD dwDelay, int repeattimes, const char* pszString)
{
	OBJECTPROCESS* p = AllocProcess(pszString);
	if (p == nullptr)return FALSE;
	if (!m_xGlobeProcessQueue.push(p))
	{
		FreeProcess(p);
		return FALSE;
	}
	p->dwDelayTime = dwDelay;
	p->dwDeliverTime = timeGetTime();
	p->dwParam[0] = dwParam1;
	p->dwParam[1] = dwParam2;
	p->dwParam[2] = dwParam3;
	p->dwParam[3] = dwParam4;
	p->repeattimes = repeattimes;
	p->ident = ident;
	return TRUE;
}

BOOL CGameWorld::AddMapObject(CMapObject* pObject)
{
	MAPOBJECT_LIST* pList = m_pObjectList[pObject->GetType()];
	if (pList)
		if (!pList->addNode(pObject->GetLinkNode(LNI_WORLD))) return FALSE;
	UINT id = pObject->GetMapId();
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(id);
	if (pMap == nullptr || !pMap->AddObject(pObject))
	{
		if (pList)
			pList->removeNode(pObject->GetLinkNode(LNI_WORLD));
		return FALSE;
	}
	return TRUE;
}

BOOL CGameWorld::RemoveMapObject(CMapObject* pObject)
{
	CLogicMap* pMap = pObject->GetMap();
	if (pMap != nullptr)
		pMap->RemoveObject(pObject);
	if (pObject->GetType() >= OBJ_MAX)
	{
		MAPOBJECT_LIST* pList = m_pObjectList[pObject->GetType()];
		if (pList)
			pList->removeNode(pObject->GetLinkNode(LNI_WORLD));
	}
	if (pObject->GetType() == OBJ_MONSTER)
	{
		char szBuffer[1024];
		int length = 0;
		if (((CMonsterEx*)pObject)->GetOutViewmsg(szBuffer, length)) ((CMonsterEx*)pObject)->SendAroundMsg(szBuffer, length);
	}
	return TRUE;
}

BOOL CGameWorld::GetStartPoint(int pro, int& mapid, int& x, int& y)
{
	if (pro < 0 || pro >= 3) return FALSE;
	return TRUE;
}

BOOL CGameWorld::LoadHumanDataDesc(int pro, const char* pszFile)
{
	if (pro >= PRO_MAX) return FALSE;
	CStringFile	sf(pszFile);
	char* Params[100];
	int	nParam = 0;
	int	nLevel = 0;
	if (sf.GetLineCount() == 0) return FALSE;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		if (*sf[i] == '#')continue;
		nParam = SearchParam(sf[i], Params, 100, ',');
		if (nParam < 26)continue;
		nLevel = atoi(Params[0]);
		if (nLevel > MAX_LEVEL || nLevel < 1)continue;
		HUMANDATADESC* p = &m_HumanData[pro][nLevel - 1];
		p->minac = (BYTE)atoi(Params[1]) & 0xff;
		p->maxac = (BYTE)atoi(Params[2]) & 0xff;
		p->minmac = (BYTE)atoi(Params[3]) & 0xff;
		p->maxmac = (BYTE)atoi(Params[4]) & 0xff;
		p->mindc = (BYTE)atoi(Params[5]) & 0xff;
		p->maxdc = (BYTE)atoi(Params[6]) & 0xff;
		p->minmc = (BYTE)atoi(Params[7]) & 0xff;
		p->maxmc = (BYTE)atoi(Params[8]) & 0xff;
		p->minsc = (BYTE)atoi(Params[9]) & 0xff;
		p->maxsc = (BYTE)atoi(Params[10]) & 0xff;
		p->wHp = (WORD)atoi(Params[11]) & 0xffff;
		p->wMp = (WORD)atoi(Params[12]) & 0xffff;
		p->bagweight = (WORD)atoi(Params[13]) & 0xffff;
		p->bodyweight = (BYTE)atoi(Params[14]) & 0xff;
		p->handweight = (BYTE)atoi(Params[15]) & 0xff;
		p->hitrate = (BYTE)atoi(Params[16]) & 0xff;
		p->escape = (BYTE)atoi(Params[17]) & 0xff;
		p->magicrecover = (BYTE)atoi(Params[18]) & 0xff;
		p->hprecover = (BYTE)atoi(Params[19]) & 0xff;
		p->magescape = (BYTE)atoi(Params[20]) & 0xff;
		p->magicnicety = (BYTE)atoi(Params[21]) & 0xff;
		p->poisonescape = (BYTE)atoi(Params[22]) & 0xff;
		p->poisonnicety = (BYTE)atoi(Params[23]) & 0xff;
		p->huoli = (WORD)atoi(Params[24]) & 0xffff;
		p->dwLevelupExp = (DWORD)atoi(Params[25]);
	}
	return TRUE;
}

CAliveObject* CGameWorld::GetAliveObjectById(UINT id)
{
	BYTE btType = id >> 24;
	switch (btType)
	{
	case OBJ_MONSTER:
		return (CAliveObject*)CMonsterManagerEx::GetInstance()->GetMonsterById(id);
	case OBJ_NPC:
		return (CAliveObject*)CNpcManager::GetInstance()->GetScriptNpcById(id);
	case OBJ_PLAYER:
		return (CAliveObject*)CHumanPlayerMgr::GetInstance()->FindbyId(id);
	}
	return nullptr;
}

USERMAGIC* CGameWorld::AllocUserMagic()
{
#ifdef USE_FREE_MEMORY
	USERMAGIC* p = new USERMAGIC;
#else
	USERMAGIC* p = m_UserMagicPool.newObject();
#endif
	if (p)
		memset(p, 0, sizeof(*p));
	return p;
}

VOID CGameWorld::FreeUserMagic(USERMAGIC* p)
{
#ifdef USE_FREE_MEMORY
	delete p;
#else
	m_UserMagicPool.deleteObject(p);
#endif	
}

BOOL CGameWorld::GetBornPoint(int pro, int& mapid, int& x, int& y, char* pszName)
{
	if (pro < 0 || pro >= 3)return FALSE;

	int selectindex = Getrand(m_iBornPointCount[pro]);
	START_POINT* pPoint = m_BornPoints[pro][selectindex];
	o_strncpy(pszName, pPoint->szName, 15);
	return GetValidPointFromStartPoint(pPoint, mapid, x, y);
}

START_POINT* CGameWorld::GetBornPoint(int pro)const
{
	if (pro < 0 || pro >= 3)return nullptr;
	if (m_iBornPointCount[pro] <= 0)return nullptr;
	int selectindex = Getrand(m_iBornPointCount[pro]);
	return m_BornPoints[pro][selectindex];
}

BOOL CGameWorld::GetStartPoint(const char* pszName, int& mapid, int& x, int& y)
{
	START_POINT* pPoint = GetStartPoint(pszName);
	if (!pPoint)pPoint = m_pStartPoints;
	if (pPoint == nullptr)return FALSE;
	return GetValidPointFromStartPoint(pPoint, mapid, x, y);
}

VOID CGameWorld::CleanAllMonsters()
{
	xListHelper<CMapObject> monsters;
	monsters.setList(m_pObjectList[OBJ_MONSTER]);
	for (CMonsterEx* pMonster = (CMonsterEx*)monsters.first(); pMonster != nullptr; pMonster = (CMonsterEx*)monsters.next())
	{
		RemoveMapObject((CMapObject*)pMonster);
		CMonsterManagerEx::GetInstance()->DeleteMonster(pMonster);
	}
}

BOOL CGameWorld::GetValidPointFromStartPoint(START_POINT* pPoint, int& map, int& x, int& y)
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(pPoint->mapid);
	if (pMap == nullptr)return FALSE;
	map = pPoint->mapid;
	int rx = Getrand(pPoint->range * 2) - pPoint->range;
	int ry = Getrand(pPoint->range * 2) - pPoint->range;
	rx += pPoint->x;
	ry += pPoint->y;
	if (pMap->IsBlocked(rx, ry))
	{
		POINT	pt;
		if (pMap->GetValidPoint(rx, ry, &pt, 1) > 0)
		{
			x = pt.x;
			y = pt.y;
		}
		else
			return GetValidPointFromStartPoint(pPoint, map, x, y);
	}
	else
	{
		x = rx;
		y = ry;
	}
	return TRUE;
}

VOID CGameWorld::AddUpdateMonster(CMonsterEx* pMonster)
{
	if (pMonster == nullptr || pMonster->IsDeath()) return;
	xListHost<CMonsterEx>::xListNode* pNode = pMonster->getUpdateNode();
	if (pNode == nullptr) return;

	// 使用分段锁减少竞争
	int lockIndex = GetMonsterLockIndex(pMonster);
	EnterCriticalSection(&m_csMonsterSegmentLocks[lockIndex]);

	if (pMonster->GetRef() == 0)
	{
		if (pNode->BelongTo(&m_xUpdateAutoMonsterList))
			m_xUpdateAutoMonsterList.removeNode(pNode);
	}
	else if (!pNode->BelongTo(&m_xUpdateAutoMonsterList))
		m_xUpdateAutoMonsterList.addNode(pNode);

	LeaveCriticalSection(&m_csMonsterSegmentLocks[lockIndex]);
}

int CGameWorld::GetMonsterLockIndex(CMonsterEx* pMonster)
{
	if (pMonster == nullptr) return 0;
	// 使用怪物指针地址计算分段索引,确保同一怪物总是落在同一分段
	UINT_PTR ptr = reinterpret_cast<UINT_PTR>(pMonster);
	return static_cast<int>(ptr % MONSTER_LOCK_SEGMENTS);
}

VOID CGameWorld::UpdateMonster(xListHost<CMonsterEx>& monsterList, MonsterUpdateType updateType, int nStart, int nEnd)
{
	if (monsterList.getCount() <= 0) return;
	// 预分配内存
	int totalCount = monsterList.getCount();
	std::vector<CMonsterEx*> updateMonsters;
	updateMonsters.reserve(totalCount);
	std::vector<xListHost<CMonsterEx>::xListNode*> nodesToRemove;
	nodesToRemove.reserve(totalCount);
	// 获取目标列表引用
	xListHost<CMonsterEx>& targetList = (updateType == MUT_AUTO) ? m_xUpdateMonsterList : m_xUpdateAutoMonsterList;
	// 收集需要处理的怪物和要删除的节点（持有锁）
	{
		EnterCriticalSection(&m_csMonsterUpdateLock);
		xListHost<CMonsterEx>::xListNode* pNode = monsterList.getHead();
		int currentIndex = 0;
		BOOL boBatch = (nStart != 0 || nEnd != 0);
		while (pNode)
		{
			CMonsterEx* p = pNode->getObject();
			if (p)
			{
				if (p->IsDeath() || p->GetRef() == 0)
					nodesToRemove.push_back(pNode);
				else if (!boBatch || (currentIndex >= nStart && currentIndex < nEnd))
					updateMonsters.push_back(p);
			}
			else
				nodesToRemove.push_back(pNode);
			pNode = pNode->getNext();
			currentIndex++;
		}
		// 删除无效节点
		for (auto node : nodesToRemove)
		{
			monsterList.removeNode(node);
			if (node->BelongTo(&targetList))
				targetList.removeNode(node);
		}
		LeaveCriticalSection(&m_csMonsterUpdateLock);
	}
	// 用于批量处理需要切换列表的怪物（按锁索引分组）
	struct MonsterSwitchInfo {
		CMonsterEx* pMonster;
		xListHost<CMonsterEx>::xListNode* pNode;
	};
	std::vector<MonsterSwitchInfo> switchMonstersBySegment[MONSTER_LOCK_SEGMENTS];
	int estimatedSwitchCount = updateMonsters.size();
	for (int seg = 0; seg < MONSTER_LOCK_SEGMENTS; seg++)
	{
		switchMonstersBySegment[seg].reserve(estimatedSwitchCount / MONSTER_LOCK_SEGMENTS + 1);
	}
	// 处理更新（不持有主锁, 但使用分段锁保护列表修改）
	for (size_t idx = 0; idx < updateMonsters.size(); idx++)
	{
		CMonsterEx* p = updateMonsters[idx];
		if (!p || p->IsDeath() || p->GetRef() == 0) continue;
		// 更新怪
		p->SetUpdateKey(this->m_dwUpdateKey);
		p->Update();
		// MUT_AUTO: 有目标时切换；MUT_ACTIVE: 无目标时切换
		BOOL shouldSwitch = (updateType == MUT_AUTO) ? (p->GetTarget() != nullptr) : (p->GetTarget() == nullptr);
		if (shouldSwitch)
		{
			// 收集需要切换的怪物, 而不是立即切换
			int lockIndex = GetMonsterLockIndex(p);
			xListHost<CMonsterEx>::xListNode* pNode = p->getUpdateNode();
			if (pNode && !p->IsDeath() && p->GetRef() > 0 && !pNode->BelongTo(&targetList))
			{
				MonsterSwitchInfo info = { p, pNode };
				switchMonstersBySegment[lockIndex].push_back(info);
			}
		}
	}
	// 批量处理需要切换列表的怪物（每个分段锁只获取一次）
	for (int seg = 0; seg < MONSTER_LOCK_SEGMENTS; seg++)
	{
		if (switchMonstersBySegment[seg].empty())
			continue;
		EnterCriticalSection(&m_csMonsterSegmentLocks[seg]);
		for (const auto& info : switchMonstersBySegment[seg])
		{
			// 再次检查状态, 确保怪物仍然有效
			if (!info.pMonster->IsDeath() && info.pMonster->GetRef() > 0)
			{
				if (!info.pNode->BelongTo(&targetList))
					targetList.addNode(info.pNode);
			}
		}
		LeaveCriticalSection(&m_csMonsterSegmentLocks[seg]);
	}
}

VOID CGameWorld::UpdatePlayers()
{
	xListHost<CMapObject>::xListNode* pNode = nullptr;
	OBJECTPROCESS* pProcesses[UPDATE_GLOBE_PROCESS_COUNT] = { nullptr };
	int	globeprocesscount = 0;
	int i = 0;
	while (globeprocesscount < UPDATE_GLOBE_PROCESS_COUNT && (pProcesses[globeprocesscount] = m_xGlobeProcessQueue.pop()))
		globeprocesscount++;
	MAPOBJECT_LIST* pList = m_pObjectList[OBJ_PLAYER];
	for (pNode = pList->getHead(); pNode != nullptr; pNode = pNode->getNext())
	{
		CHumanPlayer* p = (CHumanPlayer*)pNode->getObject();
		// 只添加必要的全局进程, 避免对所有玩家都添加
		if (globeprocesscount > 0 && p->NeedGlobeProcess(UPDATE_GLOBE_PROCESS_COUNT))
		{
			for (i = 0; i < globeprocesscount; i++)
			{
				OBJECTPROCESS* pProceess = pProcesses[i];
				if (pProceess && pProceess->ident < EP_MAX)
					p->AddProcess(pProceess);
			}
		}
		p->SetUpdateKey(this->m_dwUpdateKey);
		p->Update();
	}

	for (i = 0; i < globeprocesscount; i++)
	{
		OBJECTPROCESS* pProceess = pProcesses[i];
		if (pProceess && pProceess->ident < EP_MAX)
			FreeProcess(pProceess);
	}
}