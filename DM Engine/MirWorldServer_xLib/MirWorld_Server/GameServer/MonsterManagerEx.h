#pragma once
#include "findfile.h"

class CMonsterEx;
class CMonsterManagerEx :
	public CFindFile,
	public xSingletonClass<CMonsterManagerEx>
{
public:
	CMonsterManagerEx(void);
	virtual ~CMonsterManagerEx(void);
	VOID ClearMonsterData();
	BOOL LoadMonsters(const char* pszPath);
	VOID LoadMonsterScript(const char* pszFileName);
	VOID OnFoundFile(const char* pszFilename, UINT nParam1 = 0);
	CMonsterEx* CreateMonster(const char* pszName, int mapid, int x, int y, MONSTERGEN* pGen = nullptr);
	CMonsterEx* CreateMonster(MonsterClass* className, int mapid, int x, int y, MONSTERGEN* pGen = nullptr);
	BOOL DeleteMonster(CMonsterEx* pMonster);
	BOOL DeleteMonsterImm(CMonsterEx* pMonster);

	BOOL AddExtraMonster(CMonsterEx* pMonster);

	VOID UpdateDeleteMonster();
	VOID UpdateFreeObjects();

	CMonsterEx* GetMonsterById(UINT id) { return m_xMonsterList.getObject(id & 0xffffff); }
	MonsterClass* GetClassByName(const char* pszName) { return (MonsterClass*)m_MonsterClassHash.HGet(pszName); }

	int	getCount() { return m_xMonsterList.getCount(); }
	CMonsterEx* GetCurrentActiveMonster() { return m_pActiveMonster; }
	VOID SetCurrentActiveMonster(CMonsterEx* pMonster) { m_pActiveMonster = pMonster; }
private:
	CMonsterEx* m_pActiveMonster;
	CMonsterEx* newObject();
	VOID deleteObject(CMonsterEx* pObject);

	CNameHash m_MonsterClassHash;
	xObjectPool<MonsterClass> m_xMonsterClassPool;

	xObjectPool<CMonsterEx>	m_xMonsterPool;
	xIndexPtrList<CMonsterEx> m_xMonsterList;

	xPtrQueue<CMonsterEx> m_xDeleteQueue;
	UINT m_nCurFreeIndex;
};
