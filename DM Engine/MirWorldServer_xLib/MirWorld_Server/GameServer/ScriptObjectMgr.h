#pragma once
#include "findfile.h"

enum File_Type
{
	FILE_SCRIPT,
	FILE_DEFINE
};
class CScriptObject;
class CScriptObjectMgr : public xSingletonClass<CScriptObjectMgr>,
	public CFindFile, public xVariableProvider
{
public:
	CScriptObjectMgr(void);
	virtual ~CScriptObjectMgr(void);
	VOID Load(const char* pszPath);
	VOID OnFoundFile(const char* pszFilename, UINT nParam = 0);
	char* GetVariableValue(const char* pszVariable);
	CScriptObject* GetScriptObject(const char* pszName)
	{
		return (CScriptObject*)m_xScriptList.ObjectOf(pszName);
	}
private:
	VOID LoadScript(const char* pszFileName);
	VOID LoadDefine(const char* pszFileName);
	xStringList<512> m_xScriptList;
	xVarList<256> m_xDefineVarList;
};