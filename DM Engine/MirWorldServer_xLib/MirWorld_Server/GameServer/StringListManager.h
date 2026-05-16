#pragma once
#include "findfile.h"
typedef xStringList<128> xStringFileList;

class CStringListManager :
	public CFindFile,
	public xSingletonClass<CStringListManager>
{
public:
	CStringListManager(void);
	virtual ~CStringListManager(void);
	void OnFoundFile(const char* pszFilename, UINT nParam = 0);
	BOOL InStringList(const char* pszStringList, const char* pszString);
	BOOL AddToStringList(const char* pszStringList, const char* pszString);
	BOOL DelFromStringList(const char* pszStringList, const char* pszString);
	VOID ClearStringList(const char* pszStringList);
	VOID ClearAll();
	void LoadStringList(const char* pszFilename, BOOL bFindFile = FALSE);
	VOID Load(const char* pszPath);
	VOID SaveStringList(const char* pszName, xStringFileList* pList);
private:
	xObjectPool<xStringFileList> m_StringListPool;
	xStringList<128> m_StringListList;
};