#pragma once
#include "findfile.h"

constexpr int MONSTERGEN_MAX_COUNT = 200000; // 20万个刷怪节点, 根据 MonGens 文件夹下的文件配置, 每行算1个.
constexpr int BATCH_SIZE = 100;  // 每批处理的刷怪点数量, 可根据需要调整

// 刷怪位置策略接口
class IMonsterSpawnStrategy
{
public:
	virtual ~IMonsterSpawnStrategy() = default;
	virtual bool SpawnMonster(class CMonsterGenManager* pMgr, MONSTERGEN* p, int& iSuccess, int maxcount, BOOL bSetGenPtr, BOOL bGotoTarget, WORD wTargetX, WORD wTargetY, int* initGenCount) = 0;
};
// 随机范围刷怪策略
class CRandomRangeSpawnStrategy : public IMonsterSpawnStrategy
{
public:
	virtual bool SpawnMonster(class CMonsterGenManager* pMgr, MONSTERGEN* p, int& iSuccess, int maxcount, BOOL bSetGenPtr, BOOL bGotoTarget, WORD wTargetX, WORD wTargetY, int* initGenCount) override;
};
// 特殊阵法刷怪策略
class CSpecialFormationSpawnStrategy : public IMonsterSpawnStrategy
{
public:
	virtual bool SpawnMonster(class CMonsterGenManager* pMgr, MONSTERGEN* p, int& iSuccess, int maxcount, BOOL bSetGenPtr, BOOL bGotoTarget, WORD wTargetX, WORD wTargetY, int* initGenCount) override;
};

class CMonsterGenManager : public CFindFile, public xSingletonClass<CMonsterGenManager>
{
public:
	CMonsterGenManager(void);
	virtual ~CMonsterGenManager(void);
	BOOL LoadMonGen(const char* pszPath);
	VOID UpdateGen();
	MONSTERGEN* AddMonsterGen(const char* pszGenDesc);
	VOID InitAllGen();
	// 统一的刷怪核心方法
	BOOL UpdateGenEx(MONSTERGEN* p, IMonsterSpawnStrategy* pStrategy, int maxcount = 10, BOOL bSetGenPtr = TRUE, BOOL bGotoTarget = FALSE, WORD wTargetX = 0, WORD wTargetY = 0, int* initGenCount = nullptr);
protected:
	MONSTERGEN* m_MonsterGens[MONSTERGEN_MAX_COUNT];
	int	m_iMonsterGenCount;
	int	m_iRefreshMonGenIndex;
	VOID OnFoundFile(const char* pszFilename, UINT nParam = 0);
	xObjectPool<MONSTERGEN> m_xMonsterGenPool;
};