#pragma once

class CScriptTarget;
class CScriptObject;

enum Execute_Result
{
	ER_OK,
	ER_CLOSE,
	ER_RETURN,
	ER_BREAK,
};
class CSe_Page;
class CScriptShell :
	public xVariableProvider
{
public:
	CScriptShell(void);
	virtual ~CScriptShell(void);
	BOOL Execute(CScriptTarget* pTarget, const char* pszPage = nullptr, BOOL bUserQuery = TRUE, DWORD dwParam = 0);
	BOOL Execute(CScriptTarget* pTarget, CSe_Page* pPage, BOOL bUserQuery = TRUE, DWORD dwParam = 0);

	char* GetVariableValue(const char* pszVariable);

	virtual const char* GetTitleName() { return "NPC"; }
	virtual UINT GetTitleId() { return 0xffffffff; }
	virtual BOOL OnPageShow(CScriptTarget* pTarget, CScriptView* pView, const char* pszPage) { return TRUE; }

	Execute_Result getExecuteResult()const { return m_ExecuteResult; }
	CallParamEx& getExecuteResultValue() { return m_dwResultValue; }
	CScriptObject* getScriptObject() { return m_pScriptObject; }

	//VOID setExecuteResult( Execute_Result result, DWORD dwResultValue = 0 ){ m_ExecuteResult = result; m_dwResultValue.nParam = dwResultValue;}
	//VOID setExecuteResult( Execute_Result result, const char * pszResult ){ m_ExecuteResult = result; if( m_dwResultValue.pszParam ) delete []m_dwResultValue.pszParam; m_dwResultValue.pszParam = copystring( pszResult ); }

	VOID setExecuteResult(Execute_Result result, const char* pszValue = nullptr)
	{
		m_ExecuteResult = result;
		m_xVarList.AddVar("returnvalue", (char*)pszValue);
		m_dwResultValue.nParam = pszValue == nullptr ? 0 : StringToInteger(pszValue);
	}
	VOID setCallParams(CallParamEx* pParams) { m_pCallParams = pParams; }
	CallParamEx* getCallParams() { return m_pCallParams; }
protected:
	xVarList<32> m_xVarList;	//	_return, _p1, _p2, _p3, _p4
	CScriptObject* m_pScriptObject;
	Execute_Result m_ExecuteResult;
	CallParamEx	 m_dwResultValue;
	CallParamEx* m_pCallParams;
};
