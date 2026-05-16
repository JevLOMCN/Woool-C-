#include "StdAfx.h"
#include ".\scriptfile.h"

CScriptFile::CScriptFile(void):m_szFileName{}
{
	m_nLineIndex = 0;
	m_pData = nullptr;
}

CScriptFile::~CScriptFile(void)
{
	if (m_pData)
		delete[]m_pData;
	m_fScript.Destroy();
}

BOOL CScriptFile::Load(const char* pszFileName)
{
	o_strncpy(m_szFileName, pszFileName, 1020);
	return ProtectedLoad();
}

VOID CScriptFile::Close()
{
	m_nLineIndex = 0;
	m_fScript.Destroy();
}

char* CScriptFile::FirstLine()
{
	this->m_nLineIndex = 0;
	return CurrentLine();
}

char* CScriptFile::NextLine()
{
	this->m_nLineIndex++;
	return CurrentLine();
}

char* CScriptFile::PrevLine()
{
	if (m_nLineIndex > 0)
		m_nLineIndex--;
	return CurrentLine();
}

char* CScriptFile::CurrentLine()
{
	char* p = m_fScript[m_nLineIndex];
	if (p)
	{
		char* pl = strchr(p, ';');
		if (pl)*pl = 0;
		return TrimEx(p);
	}
	return nullptr;
}

char* CScriptFile::CurrentLineRaw()
{
	char* p = m_fScript[m_nLineIndex];
	if (p)
	{
		char* pl = strchr(p, ';');
		if (pl)*pl = 0;
		return p;  // ²»È¥¿Õ¸ñ
	}
	return nullptr;
}

UINT CScriptFile::GetCurrentLineNumber()
{
	return m_nLineIndex;
}

BOOL CScriptFile::ProtectedLoad()
{
	int iDataSize = 0;
	m_pData = LoadFile(m_szFileName, iDataSize);
	if (m_pData == nullptr)return FALSE;
	BYTE* pData = m_pData;
	return m_fScript.SetData((char*)pData, iDataSize);
}