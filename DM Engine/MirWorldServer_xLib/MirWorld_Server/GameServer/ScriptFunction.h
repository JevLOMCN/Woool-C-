#pragma once
class CFunctionRegisterAgent
{
public:
	CFunctionRegisterAgent(const char* pszName, fnCommandProc proc);
};

#define	DEFINE_SCRIPT_FUNCTION( func ) DWORD Proc_##func( CScriptShell * pShell, CScriptTarget * pTarget, CScriptView * pView, CallParamEx* Params, UINT nParam, BOOL & bContinue );\
	CFunctionRegisterAgent RegAgent_##func( #func, (fnCommandProc)Proc_##func );\
	DWORD Proc_##func( CScriptShell * pShell, CScriptTarget * pTarget, CScriptView * pView, CallParamEx* Params, UINT nParam, BOOL & bContinue ){try{ \
	CHumanPlayer * pPlayer = ((CScriptTargetForPlayer*)pTarget)->GetOwner();
#define	END_SCRIPT_FUNCTION	return 1; \
}\
	catch(...){\
		pView->AppendWordsEx( "函数 %s 出现问题在 %u 行!\\", __FUNCTION__, __LINE__ );\
		return 0;\
	}\
}

// 比较两个数值的大小
inline BOOL CompareValue(int value1, const char* sign, int value2)
{
	char sign1 = sign[0];
	char sign2 = sign[1];
	switch (sign1)
	{
	case '<':
		if (sign2 == '=')
			return (value1 <= value2);
		return (value1 < value2);
	case '>':
		if (sign2 == '=')
			return (value1 >= value2);
		return (value1 > value2);
	case '=':
		if (sign2 == '=')
			return (value1 == value2);
		return (value1 == value2);
	case '!':
		if (sign2 == '=')
			return (value1 != value2);
		return (value1 != value2);
	default:
		return FALSE;
	}
}

// 计算两个数值的值
inline int CalcValue(int value1, const char* sign, int value2)
{
	switch (sign[0])
	{
	case '+':
		return value1 + value2;
	case '-':
		return value1 - value2;
	case '*':
		return value1 * value2;
	case '/':
		if (value2 != 0)
			return value1 / value2;
		return 0;
	case '=':
		return value1 = value2;
	default:
		return 0;
	}
}