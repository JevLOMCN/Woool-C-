#include "StdAfx.h"
#include ".\monstergenmanager.h"
#include ".\monstermanagerEx.h"
#include ".\logicmap.h"
#include ".\logicmapmgr.h"
#include ".\server.h"
#include ".\gameworld.h"
#include ".\monsterex.h"
extern CServerForm g_Form;

CMonsterGenManager::CMonsterGenManager(void)
{
	memset(m_MonsterGens, 0, sizeof(m_MonsterGens));
	m_iMonsterGenCount = 0;
	m_iRefreshMonGenIndex = 0;
}

CMonsterGenManager::~CMonsterGenManager(void)
{
}

BOOL CMonsterGenManager::LoadMonGen(const char* pszPath)
{
	return StartFind(pszPath, "*.csv", TRUE);
}

VOID CMonsterGenManager::OnFoundFile(const char* pszFilename, UINT nParam)
{
	CStringFile sf(pszFilename);
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		if (*sf[i] == '#')continue;
		AddMonsterGen(sf[i]);
	}
}

MONSTERGEN* CMonsterGenManager::AddMonsterGen(const char* pszGenDesc)
{
	if (m_iMonsterGenCount >= MONSTERGEN_MAX_COUNT)return NULL;
	char g_szTempString[128];
	o_strncpy(g_szTempString, pszGenDesc, 127);
	char* Params[8];
	//#名字/地图ID/X/Y/范围/数量/刷新间隔(分)/脚本节点
	int nParam = SearchParam(g_szTempString, Params, 8, ',');
	int MongenFactor = CGameWorld::GetInstance()->GetVar(EVI_MONGENFACTOR);
	MongenFactor = MIN(MongenFactor, 100);
	int fFactor = ROUND(MongenFactor / 100);
	if (nParam >= 7)
	{
		if (CMonsterManagerEx::GetInstance()->GetClassByName(Params[0]) == nullptr)
		{
			PRINT(ERROR_RED, "刷怪信息中出现未设置的怪物 %s \n", Params[0]);
			return NULL;
		}
		int mapid = StringToInteger(Params[1]);
		int x = StringToInteger(Params[2]);
		int y = StringToInteger(Params[3]);

		CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
		if (pMap == nullptr || x < 0 || y < 0 || x >= pMap->GetWidth() || y >= pMap->GetHeight())
		{
			PRINT(ERROR_RED, "刷怪点map(%d)(%d,%d)地图不存在, 或者该点在地图之外!\n", mapid, x, y);
			return NULL;
		}

		MONSTERGEN* p = m_xMonsterGenPool.newObject();
		if (p != nullptr)
		{
			p->bStartWhenAllDead = FALSE;
			o_strncpy(p->szName, Params[0], 31);
			p->mapid = mapid;
			p->x = x;
			p->y = y;
			p->range = StringToInteger(Params[4]);
			p->count = StringToInteger(Params[5]);
			p->count = fFactor * p->count;
			if (p->count <= 0)p->count = 1;
			p->curcount = 0;
			if (*Params[6] == '*')
			{
				p->bStartWhenAllDead = TRUE;
				Params[6]++;
			}
			p->dwRefreshDelay = StringToInteger(Params[6]) * 60 * 1000;
			p->dwLastRefreshTime = 0;
			if (nParam > 7)
				p->pszScriptPage = copystring(Params[7]);
			m_MonsterGens[m_iMonsterGenCount++] = p;
			return p;
		}
	}
	return NULL;
}

VOID CMonsterGenManager::UpdateGen()
{
	//if( m_bStopMonGen )return;
	if (m_iMonsterGenCount == 0) return;
	if (m_iRefreshMonGenIndex >= m_iMonsterGenCount)
		m_iRefreshMonGenIndex = 0;
	MONSTERGEN* p = this->m_MonsterGens[m_iRefreshMonGenIndex];
	int iCurRefIndex = m_iRefreshMonGenIndex;
	m_iRefreshMonGenIndex++;
	if (p == nullptr) return;
	if (p->bStartWhenAllDead) // 判断是否等全部死亡后才刷新的逻辑
	{
		if (p->curcount > 0)
		{
			p->tmrRefresh.Savetime();
			return;
		}
	}
	int iRefCount = p->bStartWhenAllDead ? p->count : (p->count - p->curcount);
	if (iRefCount <= 0) return;
	if (!p->tmrRefresh.IsTimeOut(p->dwRefreshDelay)) return;
	if (p->dwRefreshDelay == 0) // 如果刷新时间是0, 就删除刷怪配置
	{
		m_MonsterGens[iCurRefIndex] = nullptr;
		m_xMonsterGenPool.deleteObject(p);
		return;
	}
	if (p->range <= 1000)
	{
		static CRandomRangeSpawnStrategy strategy;
		if (!UpdateGenEx(p, &strategy, iRefCount))
		{
			m_MonsterGens[iCurRefIndex] = nullptr;
			return;  // 失败时不保存时间, 等待下次重试
		}
	}
	else
	{
		static CSpecialFormationSpawnStrategy strategy;
		if (!UpdateGenEx(p, &strategy, iRefCount))
		{
			m_MonsterGens[iCurRefIndex] = nullptr;
			return;  // 失败时不保存时间, 等待下次重试
		}
	}
	p->tmrRefresh.Savetime();
}

VOID CMonsterGenManager::InitAllGen()
{
	if (m_iMonsterGenCount == 0) return;
	int iInitGenCount = 0;       // 初始化怪物数量
	int iProcessedCount = 0;     // 已处理的刷怪点数量
	for (int batch = 0; batch < m_iMonsterGenCount; batch += BATCH_SIZE)
	{
		int iBatchGenCount = 0;    // 当前批次生成的怪物数
		int iBatchProcessed = 0;   // 当前批次处理的刷怪点数
		// OpenMP 需要简单的终止条件, 先计算批次的结束索引
		int batchEnd = batch + BATCH_SIZE;
		if (batchEnd > m_iMonsterGenCount) batchEnd = m_iMonsterGenCount;
		#pragma omp parallel for reduction(+:iBatchGenCount,iBatchProcessed) schedule(dynamic, 8)
		for (int i = batch; i < batchEnd; i++)
		{
			MONSTERGEN* pGen = m_MonsterGens[i];
			if (pGen)
			{
				if (pGen->range <= 1000)
				{
					static CRandomRangeSpawnStrategy strategy;
					if (!UpdateGenEx(pGen, &strategy, pGen->count, TRUE, FALSE, 0, 0, &iBatchGenCount))
					{
						m_MonsterGens[i] = nullptr;
						m_xMonsterGenPool.deleteObject(pGen);
					}
					else
						pGen->tmrRefresh.Savetime();
				}
				else
				{
					static CSpecialFormationSpawnStrategy strategy;
					if (!UpdateGenEx(pGen, &strategy, pGen->count, TRUE, FALSE, 0, 0, &iBatchGenCount))
					{
						m_MonsterGens[i] = nullptr;
						m_xMonsterGenPool.deleteObject(pGen);
					}
					else
						pGen->tmrRefresh.Savetime();
				}
				iBatchProcessed++;
			}
		}
		// 累加总数
		iInitGenCount += iBatchGenCount;
		iProcessedCount += iBatchProcessed;
		// 每批处理完后输出进度
		if (batch + BATCH_SIZE < m_iMonsterGenCount)
		{
			PRINT(CYAN, "初始化怪物进度: %d/%d (%.2f%%), 当前生成 %d 个怪物, 总共 %d 个怪物\n",
				iProcessedCount, m_iMonsterGenCount,
				(float)iProcessedCount * 100.0f / m_iMonsterGenCount,
				iBatchGenCount, iInitGenCount);
		}
	}
	PRINT(CYAN, "初始化怪物进度: %d/%d (100.00%%), 共 %d 个刷怪点, %d 个怪物!\n",
		iProcessedCount, m_iMonsterGenCount, iProcessedCount, iInitGenCount);
}

// 统一的刷怪核心方法
BOOL CMonsterGenManager::UpdateGenEx(MONSTERGEN* p, IMonsterSpawnStrategy* pStrategy, int maxcount, BOOL bSetGenPtr, BOOL bGotoTarget, WORD wTargetX, WORD wTargetY, int* initGenCount)
{
	if (p == nullptr) return FALSE;
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(p->mapid);
	if (pMap == nullptr)
	{
		PRINT(ERROR_RED, "在地图 %d 的 (%d,%d) 刷 %s 怪, 由于地图不存在而被禁用!\n",
			p->mapid, p->x, p->y, p->szName);
		return FALSE;
	}

	if (bSetGenPtr)
	{
		DWORD dwCurTime = timeGetTime();
		if (dwCurTime - p->dwLastRefreshTime < p->dwRefreshDelay && p->dwLastRefreshTime != 0)
			return TRUE;
		p->dwLastRefreshTime = dwCurTime;

		if (p->xMonsterList.getMax() == 0)
			p->xMonsterList.create(p->count);
	}

	if (p->curcount >= p->count) return TRUE;
	
	int iSuccess = 0;
	bool bResult = pStrategy->SpawnMonster(this, p, iSuccess, maxcount, bSetGenPtr, bGotoTarget, wTargetX, wTargetY, initGenCount);

	if (!bResult) return FALSE;
	
	// 错误计数处理
	if (iSuccess == 0)
	{
		p->errortime++;
		if (p->errortime > 10)
		{
			p->count = 0;
			PRINT(ERROR_RED, "在地图 %d 的 (%d,%d) 刷 %s 怪, 10次错误被禁用!\n",
				p->mapid, p->x, p->y, p->szName);
			p->xMonsterList.destroy();
			return FALSE;
		}
	}
	else
	{
		p->errortime = 0;
	}
	return TRUE;
}

bool CRandomRangeSpawnStrategy::SpawnMonster(CMonsterGenManager* pMgr, MONSTERGEN* p, int& iSuccess, int maxcount, BOOL bSetGenPtr, BOOL bGotoTarget, WORD wTargetX, WORD wTargetY, int* initGenCount)
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(p->mapid);
	CMonsterManagerEx* pMonsterMgr = CMonsterManagerEx::GetInstance();
	CGameWorld* pGameWorld = CGameWorld::GetInstance();

	const int mapw = pMap->GetWidth();
	const int maph = pMap->GetHeight();
	const int start = p->curcount;
	const int end = p->count;
	const int startx = p->x - p->range;
	const int endx = p->x + p->range;
	const int starty = p->y - p->range;
	const int endy = p->y + p->range;

	for (int i = start; i < end; i++)
	{
		if (iSuccess >= maxcount) continue;

		int tx = GetRangeRand(startx, endx);
		int ty = GetRangeRand(starty, endy);
		tx = MAX(0, MIN(mapw - 1, tx));
		ty = MAX(0, MIN(maph - 1, ty));

		if (pMap->IsPhysicsBlocked(tx, ty))
		{
			POINT pt;
			if (pMap->GetValidPoint(tx, ty, &pt, 1) == 0) continue;
			tx = pt.x; ty = pt.y;
		}
		CMonsterEx* pMonster = pMonsterMgr->CreateMonster(p->szName, p->mapid, tx, ty, p);
		if (pMonster)
		{
			if (!pGameWorld->AddMapObject(pMonster))
				pMonsterMgr->DeleteMonsterImm(pMonster);
			else
			{
				pMonster->SetGotoTarget(bGotoTarget, wTargetX, wTargetY);
				iSuccess++;
				if (initGenCount) (*initGenCount)++;
				if(bSetGenPtr) p->xMonsterList.addObject(pMonster); // 把刷的怪对象针放到列表
			}
			if (!bSetGenPtr) pMonster->SetGen(nullptr);
		}
	}
	return true;
}

bool CSpecialFormationSpawnStrategy::SpawnMonster(CMonsterGenManager* pMgr, MONSTERGEN* p, int& iSuccess, int maxcount, BOOL bSetGenPtr, BOOL bGotoTarget, WORD wTargetX, WORD wTargetY, int* initGenCount)
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(p->mapid);
	CMonsterManagerEx* pMonsterMgr = CMonsterManagerEx::GetInstance();
	CGameWorld* pGameWorld = CGameWorld::GetInstance();

	SquareArea pSquareArea = CalculateSquareArea(p->x, p->y);
	auto SpawnAt = [&](int tx, int ty) {
		if (pMap->IsPhysicsBlocked(tx, ty))
		{
			POINT pt;
			if (pMap->GetValidPoint(tx, ty, &pt, 1) == 0) return;
			tx = pt.x; ty = pt.y;
		}
		CMonsterEx* pMonster = pMonsterMgr->CreateMonster(p->szName, p->mapid, tx, ty, p);
		if (pMonster)
		{
			if (!pGameWorld->AddMapObject(pMonster))
				pMonsterMgr->DeleteMonsterImm(pMonster);
			else
			{
				pMonster->SetGotoTarget(bGotoTarget, wTargetX, wTargetY);
				iSuccess++;
				if (initGenCount) (*initGenCount)++;
				if (bSetGenPtr) p->xMonsterList.addObject(pMonster); // 把刷的怪对象针放到列表
			}
			if (!bSetGenPtr) pMonster->SetGen(nullptr);
		}
	};

	for (const auto& pSquare : pSquareArea.innerSquare) SpawnAt(pSquare.first, pSquare.second);
	for (const auto& pSquare : pSquareArea.outerSquare) SpawnAt(pSquare.first, pSquare.second);
	return true;
}