#include "StdAfx.h"
#include ".\timesystem.h"
#include "gameworld.h"

CTimeSystem::CTimeSystem(void)
{
	m_tmrUpdate.Savetime();
	m_startupTime = CSystemTime();
	m_xTimeEventObjectQueue.create(64);
	m_wTime = m_startupTime.GetHour() * 4 + m_startupTime.GetMinute() / 15;
}

CTimeSystem::~CTimeSystem(void)
{
}

VOID CTimeSystem::Update()
{
	if (m_tmrUpdate.IsTimeOut(1000))
	{
		m_tmrUpdate.Savetime();
		CSystemTime stOld = m_curTime;
		m_curTime = CSystemTime();
		if (stOld.GetYear() != m_curTime.GetYear())
			OnYearChange();
		if (stOld.GetMonth() != m_curTime.GetMonth())
			OnMonthChange();
		if (stOld.GetDay() != m_curTime.GetDay())
			OnDayChange();
		if (stOld.GetHour() != m_curTime.GetHour())
			OnHourChange();
		if (stOld.GetMinute() != m_curTime.GetMinute())
			OnMinuteChange();
		if (stOld.GetSecond() != m_curTime.GetSecond())
			OnSecondChange();
	}
}

BOOL CTimeSystem::RegisterTimeEvent(CTimeEventObject* pTimeEvent)
{
	if (pTimeEvent == nullptr)return FALSE;
	if (this->m_xTimeEventObjectQueue.push(pTimeEvent))
		return TRUE;
	return FALSE;
}

BOOL CTimeSystem::UnRegisterTimeEvent(CTimeEventObject* pTimeEvent)
{
	if (pTimeEvent == nullptr) return FALSE;

	int count = m_xTimeEventObjectQueue.getcount();
	if (count <= 0) return FALSE;

	CTimeEventObject* pEvent = nullptr;
	BOOL removedAny = FALSE;

	for (int i = 0; i < count; i++)
	{
		pEvent = m_xTimeEventObjectQueue.pop();
		if (pEvent == nullptr) continue;
		if (pEvent == pTimeEvent)
		{
			removedAny = TRUE;
			continue;
		}
		m_xTimeEventObjectQueue.push(pEvent);
	}
	return removedAny;
}

VOID CTimeSystem::OnSecondChange()
{
	int count = m_xTimeEventObjectQueue.getcount();
	CTimeEventObject* pEventObject = nullptr;
	for (int i = 0; i < count; i++)
	{
		pEventObject = m_xTimeEventObjectQueue.pop();
		if (pEventObject == nullptr)continue;
		pEventObject->OnSecondChange(m_curTime);
		m_xTimeEventObjectQueue.push(pEventObject);
	}
}

VOID CTimeSystem::OnMinuteChange()
{
	int count = m_xTimeEventObjectQueue.getcount();
	CTimeEventObject* pEventObject = nullptr;
	for (int i = 0; i < count; i++)
	{
		pEventObject = m_xTimeEventObjectQueue.pop();
		if (pEventObject == nullptr)continue;
		pEventObject->OnMinuteChange(m_curTime);
		m_xTimeEventObjectQueue.push(pEventObject);
	}
}

VOID CTimeSystem::OnHourChange()
{
	int count = m_xTimeEventObjectQueue.getcount();
	CTimeEventObject* pEventObject = nullptr;
	for (int i = 0; i < count; i++)
	{
		pEventObject = m_xTimeEventObjectQueue.pop();
		if (pEventObject == nullptr)continue;
		pEventObject->OnHourChange(m_curTime);
		m_xTimeEventObjectQueue.push(pEventObject);
	}
}

VOID CTimeSystem::OnDayChange()
{
	int count = m_xTimeEventObjectQueue.getcount();
	CTimeEventObject* pEventObject = nullptr;
	for (int i = 0; i < count; i++)
	{
		pEventObject = m_xTimeEventObjectQueue.pop();
		if (pEventObject == nullptr)continue;
		pEventObject->OnDayChange(m_curTime);
		m_xTimeEventObjectQueue.push(pEventObject);
	}
}

VOID CTimeSystem::OnMonthChange()
{
	int count = m_xTimeEventObjectQueue.getcount();
	CTimeEventObject* pEventObject = nullptr;
	for (int i = 0; i < count; i++)
	{
		pEventObject = m_xTimeEventObjectQueue.pop();
		if (pEventObject == nullptr)continue;
		pEventObject->OnMonthChange(m_curTime);
		m_xTimeEventObjectQueue.push(pEventObject);
	}
}

VOID CTimeSystem::OnYearChange()
{
	int count = m_xTimeEventObjectQueue.getcount();
	CTimeEventObject* pEventObject = nullptr;
	for (int i = 0; i < count; i++)
	{
		pEventObject = m_xTimeEventObjectQueue.pop();
		if (pEventObject == nullptr)continue;
		pEventObject->OnYearChange(m_curTime);
		m_xTimeEventObjectQueue.push(pEventObject);
	}
}