#include "StdAfx.h"
#include ".\stringlistmanager.h"

CStringListManager::CStringListManager(void) : m_StringListList(TRUE)
{
}

CStringListManager::~CStringListManager(void)
{
}

VOID CStringListManager::ClearStringList(const char* pszStringList)
{
	xStringFileList* pList = (xStringFileList*)this->m_StringListList.ObjectOf(pszStringList);
	if (pList == nullptr)return;
	pList->Clear();
}

VOID CStringListManager::ClearAll()
{
	for (UINT i = 0; i < m_StringListList.GetCount(); i++)
	{
		xStringFileList* pList = (xStringFileList*)m_StringListList[i]->lpObject;
		if (pList != nullptr) pList->Clear();
	}
	m_StringListList.Clear();
}

BOOL CStringListManager::InStringList(const char* pszStringList, const char* pszString)
{
	char szStringList[64];
	char szString[64];
	o_strncpy(szStringList, pszStringList, 63);
	q_strupper(szStringList);

	o_strncpy(szString, pszString, 63);
	q_strupper(szString);

	xStringFileList* pList = (xStringFileList*)this->m_StringListList.ObjectOf(szStringList);
	if (pList == nullptr)return FALSE;
	return (pList->IndexOf(szString) != -1);
}

VOID CStringListManager::OnFoundFile(const char* pszFilename, UINT nParam)
{
	LoadStringList(pszFilename, TRUE);
}

void CStringListManager::LoadStringList(const char* pszFilename, BOOL bFindFile)
{
	if (bFindFile == FALSE && !FileExist(pszFilename))return;
	char szStringListName[64];
	BOOL bAdd = FALSE;
	_splitpath(pszFilename, nullptr, nullptr, szStringListName, nullptr);

	q_strupper(szStringListName);

	xStringFileList* p = (xStringFileList*)this->m_StringListList.ObjectOf(szStringListName);
	if (p == nullptr)
	{
		p = m_StringListPool.newObject();
		p->SetIgnCase(TRUE);
		bAdd = TRUE;
	}
	p->LoadFromFile(pszFilename);
	if (bAdd)
	{
		m_StringListList.Add(szStringListName, (LPVOID)p);
	}
}

BOOL CStringListManager::AddToStringList(const char* pszStringList, const char* pszString)
{
	char szStringList[128];
	char szString[128];
	//char szDirectory[64] = { 0 };
	BOOL bAdd = FALSE;
	o_strncpy(szStringList, pszStringList, 120);
	q_strupper(szStringList);

	o_strncpy(szString, pszString, 120);
	q_strupper(szString);

	xStringsExtracter<2> path(szStringList, "\\");

	char* pszStringListName = path[path.getCount() - 1];

	xStringFileList* pList = (xStringFileList*)this->m_StringListList.ObjectOf(pszStringListName);
	if (pList == nullptr)
	{
		pList = m_StringListPool.newObject();
		if (pList == nullptr)return FALSE;

		//	´´½¨Ä¿Â¼
		if (path.getCount() > 1)
		{
			char szPath[256];
			_snprintf(szPath, 256, ".\\Data\\StringList\\%s", path[0]);
			if (!PathIsFolder(szPath))
			{
				if (!CreateDirectory(szPath, nullptr))return FALSE;
			}

			char	szFileName[256];
			_makepath(szFileName, nullptr, szPath, pszStringListName, ".txt");
			pList->SetFileName(szFileName);
		}

		pList->SetIgnCase(TRUE);
		bAdd = TRUE;
	}
	if (pList->IndexOf(szString) == -1)
	{
		pList->Add(szString);
		SaveStringList(szStringList, pList);
	}
	if (bAdd)
	{
		m_StringListList.Add(pszStringListName, (LPVOID)pList);
	}
	return TRUE;
}

BOOL CStringListManager::DelFromStringList(const char* pszStringList, const char* pszString)
{
	char szStringList[64];
	char szString[64];

	o_strncpy(szStringList, pszStringList, 63);
	q_strupper(szStringList);

	o_strncpy(szString, pszString, 63);
	q_strupper(szString);

	xStringFileList* pList = (xStringFileList*)this->m_StringListList.ObjectOf(szStringList);
	if (pList == nullptr)return FALSE;
	if (pList->Delete(szString))
	{
		SaveStringList(szStringList, pList);
		return TRUE;
	}
	return FALSE;
}

VOID CStringListManager::Load(const char* pszPath)
{
	CFindFile::StartFind(pszPath, "*.txt", TRUE);
}

VOID CStringListManager::SaveStringList(const char* pszName, xStringFileList* pList)
{
	if (pList->GetFileName() == nullptr)
	{
		char szFile[1024];
		sprintf_s(szFile, sizeof(szFile), ".\\Data\\StringList\\%s.txt", pszName);
		pList->SaveToFile(szFile);
	}
	else
		pList->SaveToFile(pList->GetFileName());
}