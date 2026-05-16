#pragma once

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		ÃèÊö£º
//		×¢ÊÍ£º
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CCommandManager : public xSingletonClass<CCommandManager>
{
public:
	CCommandManager();
	BOOL AddCommand(const char* pszCommand, fnCommandProc proc);
	fnCommandProc GetCommandProc(const char* pszCommand);
	BOOL ChangeCommandName(const char* pszCommand, const char* pszNewName);
protected:
	xStringList<1024> m_xCommandList;
};