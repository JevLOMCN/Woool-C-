#include "StdAfx.h"
#include ".\autoscriptmanager.h"
#include ".\timesystem.h"
#include ".\SystemScript.h"
#include ".\HumanPlayerMgr.h"
#include ".\humanplayer.h"

CAutoScriptManager::CAutoScriptManager(void)
{
	m_pTimeScript = nullptr;
	CTimeSystem::GetInstance()->RegisterTimeEvent(this);
	m_pScriptTarget = new CHumanPlayer;
}

CAutoScriptManager::~CAutoScriptManager(void)
{
	if (CTimeSystem::GetInstance())
		CTimeSystem::GetInstance()->UnRegisterTimeEvent(this);
	Destroy();
	if (m_pScriptTarget)
		delete m_pScriptTarget;
}

VOID CAutoScriptManager::Destroy()
{
	TimeScript* p = m_pTimeScript;
	while (p)
	{
		m_pTimeScript = p->pNext;
		if (p->pszScriptPage)
			delete[]p->pszScriptPage;
		delete p;
		p = m_pTimeScript;
	}
	m_pTimeScript = nullptr;
}

static void MakeTimeup(char* pszTime1, char* pszTime2, char* pszTime3, EventTimeup& timeup)
{
	memset(&timeup, 0xff, sizeof(timeup));
	char szTimeBuf[16];
	if (pszTime1 && *pszTime1 == '[')
	{
		pszTime1++;
		strcpy(szTimeBuf, pszTime1);
		xStringsExtracter<4> utime(pszTime1, "-:", " \t");
		if (utime.getCount() == 3)
		{
			if (strchr(szTimeBuf, ':') != nullptr)
			{
				if (*utime[0] != '*')
					timeup.wHour = (WORD)StringToInteger(utime[0]);
				if (*utime[1] != '*')
					timeup.wMinute = (WORD)StringToInteger(utime[1]);
				if (*utime[2] != '*')
					timeup.wSecond = (WORD)StringToInteger(utime[2]);
			}
			else
			{
				if (*utime[0] != '*')
					timeup.wYear = (WORD)StringToInteger(utime[0]);
				if (*utime[1] != '*')
					timeup.wMonth = (WORD)StringToInteger(utime[1]);
				if (*utime[2] != '*')
					timeup.wDay = (WORD)StringToInteger(utime[2]);
			}
		}
		else if (utime.getCount() == 2)
		{
			if (*utime[0] != '*')
				timeup.wHour = (WORD)StringToInteger(utime[0]);
			if (*utime[1] != '*')
				timeup.wMinute = (WORD)StringToInteger(utime[1]);
		}
		else if (utime.getCount() == 1)
		{
			if (*utime[0] != '*')
				timeup.wDayOfWeek = (WORD)StringToInteger(utime[0]);
		}
	}
	if (pszTime2 && *pszTime2 == '[')
	{
		pszTime2++;
		strcpy(szTimeBuf, pszTime2);
		xStringsExtracter<4> utime(pszTime2, "-:", " \t");
		if (utime.getCount() == 3)
		{
			if (strchr(szTimeBuf, ':') != nullptr)
			{
				if (*utime[0] != '*')
					timeup.wHour = (WORD)StringToInteger(utime[0]);
				if (*utime[1] != '*')
					timeup.wMinute = (WORD)StringToInteger(utime[1]);
				if (*utime[2] != '*')
					timeup.wSecond = (WORD)StringToInteger(utime[2]);
			}
			else
			{
				if (*utime[0] != '*')
					timeup.wYear = (WORD)StringToInteger(utime[0]);
				if (*utime[1] != '*')
					timeup.wMonth = (WORD)StringToInteger(utime[1]);
				if (*utime[2] != '*')
					timeup.wDay = (WORD)StringToInteger(utime[2]);
			}
		}
		else if (utime.getCount() == 2)
		{
			if (*utime[0] != '*')
				timeup.wHour = (WORD)StringToInteger(utime[0]);
			if (*utime[1] != '*')
				timeup.wMinute = (WORD)StringToInteger(utime[1]);
		}
		else if (utime.getCount() == 1)
		{
			if (*utime[0] != '*')
				timeup.wDayOfWeek = (WORD)StringToInteger(utime[0]);
		}
	}
	if (pszTime3 && *pszTime3 == '[')
	{
		pszTime3++;
		strcpy(szTimeBuf, pszTime3);
		xStringsExtracter<4> utime(pszTime3, "-:", " \t");
		if (utime.getCount() == 3)
		{
			if (strchr(szTimeBuf, ':') != nullptr)
			{
				if (*utime[0] != '*')
					timeup.wHour = (WORD)StringToInteger(utime[0]);
				if (*utime[1] != '*')
					timeup.wMinute = (WORD)StringToInteger(utime[1]);
				if (*utime[2] != '*')
					timeup.wSecond = (WORD)StringToInteger(utime[2]);
			}
			else
			{
				if (*utime[0] != '*')
					timeup.wYear = (WORD)StringToInteger(utime[0]);
				if (*utime[1] != '*')
					timeup.wMonth = (WORD)StringToInteger(utime[1]);
				if (*utime[2] != '*')
					timeup.wDay = (WORD)StringToInteger(utime[2]);
			}
		}
		else if (utime.getCount() == 2)
		{
			if (*utime[0] != '*')
				timeup.wHour = (WORD)StringToInteger(utime[0]);
			if (*utime[1] != '*')
				timeup.wMinute = (WORD)StringToInteger(utime[1]);
		}
		else if (utime.getCount() == 1)
		{
			if (*utime[0] != '*')
				timeup.wDayOfWeek = (WORD)StringToInteger(utime[0]);
		}
	}
}

VOID CAutoScriptManager::Load(const char* pszSettingFile)
{
	Destroy();
	CStringFile sf(pszSettingFile);
	char* pLine = nullptr;
	xCharSet wht(" \t");
	xCharSet spl("]");
	int nParam;
	char* Params[4];
	EventTimeup timeup;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == '#' || *pLine == 0)continue;
		nParam = ExtractStrings(pLine, wht, spl, Params, 4, FALSE, TRUE);
		if (nParam < 2)continue;
		if (*Params[0] != '[')continue;
		char* p = nullptr;
		if (nParam >= 4)
		{
			MakeTimeup(Params[0], Params[1], Params[2], timeup);
			p = Params[3];
		}
		else if (nParam >= 3)
		{
			MakeTimeup(Params[0], Params[1], nullptr, timeup);
			p = Params[2];
		}
		else
		{
			MakeTimeup(Params[0], nullptr, nullptr, timeup);
			p = Params[1];
		}
		AddTimeScript(&timeup, p);
	}
}

VOID CAutoScriptManager::AddTimeScript(EventTimeup* pTimeup, const char* pszPage)
{
	static const char* weekname[7] = { "日", "一", "二", "三", "四", "五", "六" };
	char szYear[20], szMonth[20], szDay[20], szHour[20], szMinute[20], szSecond[20], szWeek[32];
	if (pTimeup->wYear != 0xffff)sprintf(szYear, "%u", pTimeup->wYear);
	if (pTimeup->wMonth != 0xffff)sprintf(szMonth, "%u", pTimeup->wMonth);
	if (pTimeup->wDay != 0xffff)sprintf(szDay, "%u", pTimeup->wDay);
	if (pTimeup->wHour != 0xffff)sprintf(szHour, "%u", pTimeup->wHour);
	if (pTimeup->wMinute != 0xffff)sprintf(szMinute, "%u", pTimeup->wMinute);
	if (pTimeup->wSecond != 0xffff)sprintf(szSecond, "%u", pTimeup->wSecond);
	if (pTimeup->wDayOfWeek != 0xffff)sprintf(szWeek, "星期%s", weekname[pTimeup->wDayOfWeek % 7]);
	PRINT(STRING_GREEN, "注册%s年%s月%s日%s - %s时%s分%s秒 - 自动执行脚本 %s\n",
		pTimeup->wYear == 0xffff ? "每" : szYear,
		pTimeup->wMonth == 0xffff ? "每" : szMonth,
		pTimeup->wDay == 0xffff ? "每" : szDay,
		pTimeup->wDayOfWeek == 0xffff ? "" : szWeek,
		pTimeup->wHour == 0xffff ? "每" : szHour,
		pTimeup->wMinute == 0xffff ? "每" : szMinute,
		pTimeup->wSecond == 0xffff ? "每" : szSecond,
		pszPage);
	TimeScript* p = new TimeScript();
	p->pszScriptPage = nullptr;
	p->pNext = m_pTimeScript;
	m_pTimeScript = p;
	p->m_eTimeUp = *pTimeup;
	p->pszScriptPage = copystring(pszPage);
}

VOID CAutoScriptManager::OnSecondChange(CSystemTime& curTime)
{
	WORD wYear = curTime.GetYear();
	WORD wMonth = curTime.GetMonth();
	WORD wDay = curTime.GetDay();
	WORD wDayOfWeek = curTime.GetDayOfWeek();
	WORD wHour = curTime.GetHour();
	WORD wMinute = curTime.GetMinute();
	WORD wSecond = curTime.GetSecond();
	for (TimeScript* p = m_pTimeScript; p != nullptr; p = p->pNext)
	{
		if (p->m_eTimeUp.wYear != 0xffff && p->m_eTimeUp.wYear != wYear)
			continue;
		if (p->m_eTimeUp.wMonth != 0xffff && p->m_eTimeUp.wMonth != wMonth)
			continue;
		if (p->m_eTimeUp.wDay != 0xffff && p->m_eTimeUp.wDay != wDay)
			continue;
		if (p->m_eTimeUp.wDayOfWeek != 0xffff && p->m_eTimeUp.wDayOfWeek != wDayOfWeek)
			continue;
		if (p->m_eTimeUp.wHour != 0xffff && p->m_eTimeUp.wHour != wHour)
			continue;
		if (p->m_eTimeUp.wMinute != 0xffff && p->m_eTimeUp.wMinute != wMinute)
			continue;
		if (p->m_eTimeUp.wSecond != 0xffff && p->m_eTimeUp.wSecond != wSecond)
			continue;
		CSystemScript::GetInstance()->Execute(m_pScriptTarget->GetScriptTarget(), p->pszScriptPage, FALSE);
	}
}

VOID CAutoScriptManager::OnMinuteChange(CSystemTime& curTime)
{
	WORD wYear = curTime.GetYear();
	WORD wMonth = curTime.GetMonth();
	WORD wDay = curTime.GetDay();
	WORD wDayOfWeek = curTime.GetDayOfWeek();
	WORD wHour = curTime.GetHour();
	WORD wMinute = curTime.GetMinute();
	for (TimeScript* p = m_pTimeScript; p != nullptr; p = p->pNext)
	{
		if (p->m_eTimeUp.wYear != 0xffff && p->m_eTimeUp.wYear != wYear)
			continue;
		if (p->m_eTimeUp.wMonth != 0xffff && p->m_eTimeUp.wMonth != wMonth)
			continue;
		if (p->m_eTimeUp.wDay != 0xffff && p->m_eTimeUp.wDay != wDay)
			continue;
		if (p->m_eTimeUp.wDayOfWeek != 0xffff && p->m_eTimeUp.wDayOfWeek != wDayOfWeek)
			continue;
		if (p->m_eTimeUp.wHour != 0xffff && p->m_eTimeUp.wHour != wHour)
			continue;
		if (p->m_eTimeUp.wMinute != 0xffff && p->m_eTimeUp.wMinute != wMinute)
			continue;
		CSystemScript::GetInstance()->Execute(m_pScriptTarget->GetScriptTarget(), p->pszScriptPage, FALSE);
	}
}

VOID CAutoScriptManager::OnHourChange(CSystemTime& curTime)
{
	WORD wYear = curTime.GetYear();
	WORD wMonth = curTime.GetMonth();
	WORD wDay = curTime.GetDay();
	WORD wDayOfWeek = curTime.GetDayOfWeek();
	WORD wHour = curTime.GetHour();
	for (TimeScript* p = m_pTimeScript; p != nullptr; p = p->pNext)
	{
		if (p->m_eTimeUp.wYear != 0xffff && p->m_eTimeUp.wYear != wYear)
			continue;
		if (p->m_eTimeUp.wMonth != 0xffff && p->m_eTimeUp.wMonth != wMonth)
			continue;
		if (p->m_eTimeUp.wDay != 0xffff && p->m_eTimeUp.wDay != wDay)
			continue;
		if (p->m_eTimeUp.wDayOfWeek != 0xffff && p->m_eTimeUp.wDayOfWeek != wDayOfWeek)
			continue;
		if (p->m_eTimeUp.wHour != 0xffff && p->m_eTimeUp.wHour != wHour)
			continue;
		CSystemScript::GetInstance()->Execute(m_pScriptTarget->GetScriptTarget(), p->pszScriptPage, FALSE);
	}
}

VOID CAutoScriptManager::OnDayChange(CSystemTime& curTime)
{
	WORD wYear = curTime.GetYear();
	WORD wMonth = curTime.GetMonth();
	WORD wDay = curTime.GetDay();
	WORD wDayOfWeek = curTime.GetDayOfWeek();
	for (TimeScript* p = m_pTimeScript; p != nullptr; p = p->pNext)
	{
		if (p->m_eTimeUp.wYear != 0xffff && p->m_eTimeUp.wYear != wYear)
			continue;
		if (p->m_eTimeUp.wMonth != 0xffff && p->m_eTimeUp.wMonth != wMonth)
			continue;
		if (p->m_eTimeUp.wDay != 0xffff && p->m_eTimeUp.wDay != wDay)
			continue;
		if (p->m_eTimeUp.wDayOfWeek != 0xffff && p->m_eTimeUp.wDayOfWeek != wDayOfWeek)
			continue;
		CSystemScript::GetInstance()->Execute(m_pScriptTarget->GetScriptTarget(), p->pszScriptPage, FALSE);
	}
}

VOID CAutoScriptManager::OnMonthChange(CSystemTime& curTime)
{
	WORD wYear = curTime.GetYear();
	WORD wMonth = curTime.GetMonth();
	for (TimeScript* p = m_pTimeScript; p != nullptr; p = p->pNext)
	{
		if (p->m_eTimeUp.wYear != 0xffff && p->m_eTimeUp.wYear != wYear)
			continue;
		if (p->m_eTimeUp.wMonth != 0xffff && p->m_eTimeUp.wMonth != wMonth)
			continue;
		CSystemScript::GetInstance()->Execute(m_pScriptTarget->GetScriptTarget(), p->pszScriptPage, FALSE);
	}
}

VOID CAutoScriptManager::OnYearChange(CSystemTime& curTime)
{
	WORD wYear = curTime.GetYear();
	for (TimeScript* p = m_pTimeScript; p != nullptr; p = p->pNext)
	{
		if (p->m_eTimeUp.wYear != 0xffff && p->m_eTimeUp.wYear != wYear)
			continue;
		CSystemScript::GetInstance()->Execute(m_pScriptTarget->GetScriptTarget(), p->pszScriptPage, FALSE);
	}
}