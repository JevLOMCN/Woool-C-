#pragma once
typedef struct tagScriptTimerDef
{
	DWORD dwStartTime;
	DWORD dwStopTime;
}ScriptTimerDef;
class CScriptTimerManager :
	public xSingletonClass<CScriptTimerManager>
{
public:
	CScriptTimerManager(void);
	virtual ~CScriptTimerManager(void);
	BOOL StartTimer(UINT nIndex);
	BOOL StopTimer(UINT nIndex);
	DWORD GetTimerTime(UINT nIndex);
protected:
	ScriptTimerDef m_xTimer[256];
};