#include "StdAfx.h"
#include "scriptvariable.h"

CVariableFile::CVariableFile()
{
	m_bModified = FALSE;
	m_pFilename = nullptr;
	m_szName[0] = 0;
	if (!PathIsFolder(".\\Data\\Variables"))
		CreateDirectory(".\\Data\\Variables", nullptr);
	if (!PathIsFolder(".\\Data\\CharVars"))
		CreateDirectory(".\\Data\\CharVars", nullptr);
}

CVariableFile::~CVariableFile()
{
	m_xVarList.ClearVars();
}

BOOL CVariableFile::Load(const char* pszFilename)
{
	CStringFile sf(pszFilename);
	char* pLine = nullptr;
	for (UINT i = 0; i < (UINT)sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == 0 || *pLine == '#')continue;
		xStringsExtracter<2> val(pLine, "=", " \t\"");
		if (val.getCount() == 0)continue;
		if (val.getCount() > 1)
			m_xVarList.AddVar(val[0], val[1]);
		else
			m_xVarList.AddVar(val[0], "");
	}
	if (this->m_pFilename)delete[]m_pFilename;
	m_pFilename = copystring(pszFilename);
	_splitpath(m_pFilename, nullptr, nullptr, m_szName, nullptr);
	return TRUE;
}

BOOL CVariableFile::Reload()
{
	if (m_pFilename == nullptr)return FALSE;
	m_xVarList.ClearVars();
	CStringFile sf(m_pFilename);
	char* pLine = nullptr;
	for (UINT i = 0; i < (UINT)sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == 0 || *pLine == '#')continue;
		xStringsExtracter<2> val(pLine, "=", " \t\"");
		if (val.getCount() == 0)continue;
		if (val.getCount() > 1)
			m_xVarList.AddVar(val[0], val[1]);
		else
			m_xVarList.AddVar(val[0], "");
	}
	return TRUE;
}

static thread_local char g_szFilename[1024];
BOOL CVariableFile::Save()
{
	if (m_pFilename == nullptr)
	{
		if (m_szName[0] == 0)return FALSE;
		sprintf(g_szFilename, ".\\Data\\Variables\\%s.txt", m_szName);
		m_pFilename = copystring(g_szFilename);
	}
	FILE* fp = fopen(m_pFilename, "w");
	if (fp == nullptr)return FALSE;
	xStringList<32>* p = m_xVarList.GetList();
	for (UINT n = 0; n < p->GetCount(); n++)
	{
		xVarList<32>::Variable* var = (xVarList<32>::Variable*)(*p)[n]->lpObject;
		char* pVal = var->pszValue;
		if (pVal == nullptr)
			pVal = "";
		fprintf(fp, "%s = \"%s\"\n", (*p)[n]->pszString, pVal);
	}
	fclose(fp);
	return TRUE;
}

const char* CVariableFile::GetVariableValue(const char* pszName)
{
	return this->m_xVarList.GetVarValue(pszName);
}

VOID CVariableFile::SetVariableValue(const char* pszName, const char* pszValue)
{
	this->m_xVarList.AddVar(pszName, (char*)pszValue);
}

VOID CVariableFile::DelVariable(const char* pszName)
{
	this->m_xVarList.DelVar(pszName);
}

VOID CVariableFile::Clear()
{
	this->m_xVarList.ClearVars();
}

CVariableRegisterAgent::CVariableRegisterAgent(const char* pszName, fnGetVariable fnGet)
{
	if (!CScriptVariableManager::GetInstance()->AddVariable(pszName, fnGet))
		PRINT(ERROR_RED, "×¢²á±äÁ¿º¯Êý %s Ê§°Ü!\n", pszName);
}

CScriptVariableManager::CScriptVariableManager() : m_xVarList(TRUE)
{
	m_nCachePtr = 0;
	memset(m_xStringCache, 0, sizeof(m_xStringCache));
}

BOOL CScriptVariableManager::AddVariable(const char* pszName, fnGetVariable fnGet)
{
	if (this->m_xVarList.Add(pszName, (LPVOID)fnGet) == -1)return FALSE;
	return TRUE;
}

BOOL CScriptVariableManager::GetVariable(const char* pszName, CHumanPlayer* pPlayer, GetVariableStruct& var)
{
	BOOL boOK = FALSE;
	char szTemp[256];
	strcpy_s(szTemp, sizeof(szTemp), pszName);
	char* Params[5];
	CallParamEx* callParams = new CallParamEx[4];
	int nParam = SearchParam(szTemp, Params, 5, '*');
	if (nParam > 0)
	{
		for (int i = 1; i < nParam; i++)
		{
			callParams[i-1].pszParam = Params[i];
			callParams[i-1].nParam = StringToInteger(Params[i]);
		}
		fnGetVariable fnGet = (fnGetVariable)m_xVarList.ObjectOf(Params[0]);
		if (fnGet)
		{
			fnGet(Params[0], callParams, nParam - 1, var, pPlayer);
			boOK = TRUE;
		}
	}
	if (callParams) delete[] callParams;
	if (boOK) return TRUE;
	return FALSE;
}

VOID CScriptVariableManager::OnFoundFile(const char* pszFilename, UINT nParam)
{
	CVariableFile* pFile = NewVariableFile();
	if (pFile == nullptr)return;
	if (!pFile->Load(pszFilename))
	{
		DeleteVariableFile(pFile);
		return;
	}
	if (this->m_xVarGroupList.Add(pFile->GetName(), (LPVOID)pFile) == -1)
	{
		DeleteVariableFile(pFile);
		return;
	}
}

CVariableFile* CScriptVariableManager::NewVariableFile()
{
	return this->m_xVarGroupPool.newObject();
}

VOID CScriptVariableManager::DeleteVariableFile(CVariableFile* pFile)
{
	pFile->Clear();
	this->m_xVarGroupPool.deleteObject(pFile);
}

BOOL CScriptVariableManager::SetVariable(const char* pszName, const char* pszVarName, const char* pszValue)
{
	CVariableFile* pFile = (CVariableFile*)m_xVarGroupList.ObjectOf(pszName);
	if (pFile == nullptr)
	{
		pFile = NewVariableFile();
		if (pFile == nullptr)return FALSE;
		pFile->SetName(pszName);
		if (this->m_xVarGroupList.Add(pFile->GetName(), (LPVOID)pFile) == -1)
		{
			DeleteVariableFile(pFile);
			return FALSE;
		}
	}
	pFile->SetVariableValue(pszVarName, pszValue);
	pFile->Save();
	return TRUE;
}

const char* CScriptVariableManager::GetVariable(const char* pszName, const char* pszVarName)
{
	CVariableFile* pFile = (CVariableFile*)m_xVarGroupList.ObjectOf(pszName);
	if (pFile == nullptr)return nullptr;
	return pFile->GetVariableValue(pszVarName);
}

VOID CScriptVariableManager::DelVariable(const char* pszName, const char* pszVarName)
{
	CVariableFile* pFile = (CVariableFile*)m_xVarGroupList.ObjectOf(pszName);
	if (pFile == nullptr)return;
	pFile->DelVariable(pszVarName);
}