#pragma once
class CScriptView;
class CSe_Page;
class CScriptShell;
class CScriptObject;

class CScriptTarget : public xVariableProvider
{
public:
	CScriptTarget(void);
	virtual ~CScriptTarget(void);
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	//		描述：	发送页面, 发送关闭页面
	//		注释：
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	virtual VOID SendPage(CScriptShell* pShell, CScriptView* pView) {}
	virtual VOID SendClosePage(CScriptShell* pShell) { setCurScriptObject(nullptr); setCurShell(nullptr); }
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	//		描述：	取得变量的值
	//		注释：
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	virtual char* GetVariableValue(const char* pszVariable);
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	//		描述：	SHELL和PAGE连接操作
	//		注释：
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	CScriptObject* getCurScriptObject() { return m_pCurScriptObject; }
	VOID setCurScriptObject(CScriptObject* pObject) { m_pCurScriptObject = pObject; }
	CScriptShell* getCurShell() { return m_pCurShell; }
	VOID setCurShell(CScriptShell* pShell) { m_pCurShell = pShell; }
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	//		描述：	变量操作
	//		注释：
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	VOID SetInputPage(const char* pszPage, CScriptShell* pInputShell, UINT nLength);
	VOID OnInputConfirm(char* pszInputText);
	VOID SetPutItemPage(const char* pszPage, CScriptShell* pPutItemShell);
	VOID OnPutItem(UINT nShellId, UINT nMakeIndex);
	virtual VOID SetVariableValue(const char* pszVariable, const char* pszValue);
	virtual VOID ClrVariable(const char* pszVariable);
	virtual BOOL AddVariable(const char* pszVariable, const char* pszValue);
protected:
	char m_szToPage[64];
	CScriptShell* m_pInputShell;
	UINT m_nInputLength;
	CScriptObject* m_pCurScriptObject;
	CScriptShell* m_pCurShell;
	xVarList<32> m_xVarList;
};

class CHumanPlayer;
class CScriptTargetForPlayer : public CScriptTarget
{
public:
	CScriptTargetForPlayer(CHumanPlayer* pPlayer);
	VOID SendPage(CScriptShell* pShell, CScriptView* pView);
	VOID SendClosePage(CScriptShell* pShell);
	char* GetVariableValue(const char* pszVariable);
	VOID SetVariableValue(const char* pszVariable, const char* pszValue);
	VOID ClrVariable(const char* pszVariable);
	BOOL AddVariable(const char* pszVariable, const char* pszValue);
	VOID Clean();
	CHumanPlayer* GetOwner() { return m_pOwner; }
	VOID SaveVars(const char* pszFilename);
	VOID LoadVars(const char* pszFilename);
	BOOL CheckExceedDistance(BOOL boCLose = TRUE);
protected:
	CHumanPlayer* m_pOwner;
};
