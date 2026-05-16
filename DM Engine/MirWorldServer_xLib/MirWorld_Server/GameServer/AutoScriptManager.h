#pragma once
#include "localdefine.h"
#include "scripttarget.h"

typedef struct tagEventTimeup
{
	tagEventTimeup()
	{
		memset(this, 0xffff, sizeof(*this));
	}
	WORD wYear;
	WORD wMonth;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wDayOfWeek;
}EventTimeup;

typedef struct tagTimeScript
{
	tagTimeScript()
	{
		pszScriptPage = nullptr;
		pNext = nullptr;
	}
	char* pszScriptPage;
	EventTimeup m_eTimeUp;
	tagTimeScript* pNext;
}TimeScript;

class CHumanPlayer;
class CAutoScriptManager : public CTimeEventObject, public xSingletonClass<CAutoScriptManager>
{
public:
	CAutoScriptManager(void);
	virtual ~CAutoScriptManager(void);
	VOID Destroy();
	// 加载自动脚本配置
	VOID Load(const char* pszSettingFile);
	//获取自动脚本目标
	CHumanPlayer* GetScriptTarget() { return m_pScriptTarget; }
protected:
	// 添加时间脚本配置
	VOID AddTimeScript(EventTimeup* pTimeup, const char* pszPage);
	// 秒变化时的处理
	VOID OnSecondChange(CSystemTime& curTime);
	// 分钟变化时的处理
	VOID OnMinuteChange(CSystemTime& curTime);
	// 小时变化时的处理
	VOID OnHourChange(CSystemTime& curTime);
	// 日期变化时的处理
	VOID OnDayChange(CSystemTime& curTime);
	// 月份变化时的处理
	VOID OnMonthChange(CSystemTime& curTime);
	// 年份变化时的处理
	VOID OnYearChange(CSystemTime& curTime);
protected:
	TimeScript* m_pTimeScript;
	CHumanPlayer* m_pScriptTarget;
};