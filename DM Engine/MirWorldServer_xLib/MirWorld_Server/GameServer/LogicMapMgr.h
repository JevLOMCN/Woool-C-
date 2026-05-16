#pragma once
#include "findfile.h"
#include "logicmap.h"

constexpr UINT MAX_LOGIC_MAP = 10240;

class CLogicMapMgr : public xSingletonClass<CLogicMapMgr>,
	public CFindFile
{
public:
	CLogicMapMgr(void);
	virtual ~CLogicMapMgr(void);

	VOID Load(const char* pszPath);
	CLogicMap* GetLogicMapByTitle(const char* pszTitleName);
	CLogicMap* GetLogicMapById(UINT id)const { if (id == 0 || id > MAX_LOGIC_MAP) return nullptr; return m_pMaps[id - 1]; }
	int	getCount() { return m_LogicMaps.GetCount(); }
protected:
	VOID OnFoundFile(const char* pszFilename, UINT nParam = 0);
private:
	CLogicMap* m_pMaps[MAX_LOGIC_MAP];
	CNameHash m_MapNameHash;
	CLogicMap* m_pLoadingMap;
	CIndexList<CLogicMap, MAX_LOGIC_MAP> m_LogicMaps;
};
