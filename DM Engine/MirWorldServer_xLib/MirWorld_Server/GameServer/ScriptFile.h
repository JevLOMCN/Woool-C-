#pragma once
class CScriptFile
{
public:
	CScriptFile(void);
	virtual ~CScriptFile(void);
	BOOL Load(const char* pszFileName);
	VOID Close();
	char* FirstLine();
	char* NextLine();
	char* PrevLine();
	char* CurrentLine();
	char* CurrentLineRaw();
	UINT GetCurrentLineNumber();
	VOID SetLineIndex(UINT index)
	{
		m_nLineIndex = index;
	}
	const char* GetFileName() { return m_szFileName; }
protected:
	BOOL ProtectedLoad();
	char m_szFileName[1024];
	UINT m_nLineIndex;
	CStringFile m_fScript;
	BYTE* m_pData;
};