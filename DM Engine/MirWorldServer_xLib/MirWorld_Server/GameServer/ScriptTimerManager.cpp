#include "StdAfx.h"
#include ".\scripttimermanager.h"

CScriptTimerManager::CScriptTimerManager(void)
{
	memset(this->m_xTimer, 0, sizeof(this->m_xTimer));
}

CScriptTimerManager::~CScriptTimerManager(void)
{
}

BOOL CScriptTimerManager::StartTimer(UINT nIndex)
{
	if (nIndex > 256 || nIndex == 0)return FALSE;
	this->m_xTimer[nIndex - 1].dwStartTime = timeGetTime();
	return TRUE;
}

BOOL CScriptTimerManager::StopTimer(UINT nIndex)
{
	if (nIndex > 256 || nIndex == 0)return FALSE;
	this->m_xTimer[nIndex - 1].dwStopTime = timeGetTime();
	return TRUE;
}

DWORD CScriptTimerManager::GetTimerTime(UINT nIndex)
{
	if (nIndex > 256 || nIndex == 0)return 0;
	DWORD dwTime = (DWORD)ROUND(GetTimeToTime(m_xTimer[nIndex - 1].dwStartTime, m_xTimer[nIndex - 1].dwStopTime) / 1000);
	return dwTime;
}