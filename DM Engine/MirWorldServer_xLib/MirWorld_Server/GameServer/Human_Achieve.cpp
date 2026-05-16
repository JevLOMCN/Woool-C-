#include "stdafx.h"
#include "TimeAchieve.h"
#include "HumanPlayer.h"
#include "systemscript.h"

VOID CHumanPlayer::InitAchievement(int nCount)
{
	m_Achievement.btLevel = 0;
	m_Achievement.dwExp = 0;
	m_Achievement.btStatus.assign(nCount, 0);
	m_Achievement.btRecentCount.assign(nCount, 0);
	m_Achievement.dwCompleteTime.assign(nCount, 0);
}

BOOL CHumanPlayer::ChangeAchieveGroupExp(BYTE btGroupId, BYTE btType, DWORD btRecentCount)
{
	if (btType>2) return FALSE;
	// 获取该组的所有成就索引
	const std::vector<int>* pIndexList = CTimeAchieve::GetInstance()->FindAchieveGroupById(btGroupId);
	if (pIndexList == nullptr || pIndexList->empty()) return FALSE;
	DWORD dwNow = (DWORD)time(nullptr);
	// 遍历该组所有成就，增加进度值
	for (int nIndex : *pIndexList)
	{
		const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->GetAchieve(nIndex);
		const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(pAchieveItem->nId);
		if (pAchieveItem == nullptr || pIndex == -1) continue;
		// 检查成就是否已完成或已领取
		if (m_Achievement.btStatus[pIndex] >= 1) continue;
		DWORD& refCount = m_Achievement.btRecentCount[pIndex];
		switch (btType)
		{
		case 0:
			if (refCount > (0xffffffff - btRecentCount)) refCount = 0xffffffff;
			else refCount += btRecentCount;
		break;
		case 1:
			if (refCount < btRecentCount) refCount = 0;
			else refCount -= btRecentCount;
		break;
		case 2:
			refCount = btRecentCount;
		break;
		}
		// 检查是否达到完成条件
		if (pAchieveItem->nMaxExp > 0 && m_Achievement.btRecentCount[pIndex] >= (DWORD)pAchieveItem->nMaxExp)
		{
			m_Achievement.dwExp += pAchieveItem->nPoint; // 增加成就点
			m_Achievement.btStatus[pIndex] = 1; // 标记为可领取
			m_Achievement.dwCompleteTime[pIndex] = dwNow; // 设置完成时间戳
			setSParam(0, pAchieveItem->szName); // 成就名
			setVParam(1, nIndex); // 成就ID
			CSystemScript::GetInstance()->Execute(GetScriptTarget(), "成就系统.完成奖励");
		}
		CheckAchieveLevelUp();
	}
	return TRUE;
}

BOOL CHumanPlayer::ChangeAchieveExp(WORD wId, BYTE btType, DWORD btRecentCount)
{
	if (btType > 2) return FALSE;
	// 获取成就对象
	const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->FindAchieveById(wId);
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pAchieveItem == nullptr || pIndex == -1) return FALSE;
	// 检查成就是否已完成或已领取
	if (m_Achievement.btStatus[pIndex] >= 1) return FALSE;
	DWORD& refCount = m_Achievement.btRecentCount[pIndex];
	switch (btType)
	{
	case 0:
		if (refCount > (0xffffffff - btRecentCount)) refCount = 0xffffffff;
		else refCount += btRecentCount;
		break;
	case 1:
		if (refCount < btRecentCount) refCount = 0;
		else refCount -= btRecentCount;
		break;
	case 2:
		refCount = btRecentCount;
		break;
	}
	// 检查是否达到完成条件
	if (pAchieveItem->nMaxExp > 0 && refCount >= (DWORD)pAchieveItem->nMaxExp)
	{
		m_Achievement.dwExp += pAchieveItem->nPoint; // 增加成就点
		m_Achievement.btStatus[pIndex] = 1; // 标记为可领取
		DWORD dwNow = (DWORD)time(nullptr);
		m_Achievement.dwCompleteTime[pIndex] = dwNow; // 设置完成时间戳
		setSParam(0, pAchieveItem->szName); // 成就名
		setVParam(1, wId); // 成就ID
		CSystemScript::GetInstance()->Execute(GetScriptTarget(), "成就系统.完成奖励");
	}
	CheckAchieveLevelUp();
	return TRUE;
}

BOOL CHumanPlayer::SetAchieveState(WORD wId, BYTE btStatu)
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return FALSE;
	m_Achievement.btStatus[pIndex] = btStatu;
	return TRUE;
}

BOOL CHumanPlayer::SetAchieveTime(WORD wId, DWORD dwTime)
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return FALSE;
	m_Achievement.dwCompleteTime[pIndex] = dwTime;
	return TRUE;
}

BOOL CHumanPlayer::SendGotAchieve(WORD wId)
{
	// 获取成就对象
	const TIMEACHIEVE_ITEM* pAchieveItem = CTimeAchieve::GetInstance()->FindAchieveById(wId);
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pAchieveItem == nullptr || pIndex == -1) return FALSE;

	char g_szTempBuffer[64];
	xPacket packet(g_szTempBuffer, 64);
	int nValue = 1;
	packet.push(&nValue, 1);
	packet.push(4);
	nValue = 1;
	packet.push(&nValue, 4);
	nValue = pAchieveItem->nId;
	packet.push(&nValue, 4);
	nValue = 1;
	packet.push(&nValue, 4);
	packet.push(4);
	nValue = 1;
	packet.push(&nValue, 4);
	packet.push(4);
	packet.push(4);
	packet.push(4);
	packet.push(5);
	SendMsg(GetId(), 0x959, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	return TRUE;
}

BOOL CHumanPlayer::ChangeAchievePoint(BYTE btType, DWORD dwExp)
{
	
	DWORD dwMaxExp = CTimeAchieve::GetInstance()->GetLevelMaxExp(m_Achievement.btLevel);
	if (dwMaxExp == -1) return FALSE;
	switch (btType)
	{
	case 0:
		m_Achievement.dwExp += dwExp;
		break;
	case 1:
		m_Achievement.dwExp -= dwExp;
		break;
	case 2:
		m_Achievement.dwExp = dwExp;
		break;
	}
	// 检查是否达到升级条件
	if (m_Achievement.dwExp >= dwMaxExp)
	{
		m_Achievement.dwExp = 0;
		m_Achievement.btLevel++;
		SaySystem("恭喜您，成就等级提升到 %u 级!", m_Achievement.btLevel);
	}
	return TRUE;
}

DWORD CHumanPlayer::GetAchieveExpById(WORD wId) const
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return 0;
	return m_Achievement.btRecentCount[pIndex];
}

BYTE CHumanPlayer::GetAchieveStateById(WORD wId) const
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return 0;
	return m_Achievement.btStatus[pIndex];
}

DWORD CHumanPlayer::GetAchieveCompleteTimeById(WORD wId) const
{
	const int pIndex = CTimeAchieve::GetInstance()->FindIndexById(wId);
	if (pIndex == -1) return 0;
	return m_Achievement.dwCompleteTime[pIndex];
}

VOID CHumanPlayer::CheckAchieveLevelUp() 
{
	BYTE& refLevel = m_Achievement.btLevel;
	DWORD& refExp = m_Achievement.dwExp;
	DWORD dwMaxExp = CTimeAchieve::GetInstance()->GetLevelMaxExp(refLevel);
	if (dwMaxExp == -1) return;
	while (refExp >= dwMaxExp)
	{
		refExp -= dwMaxExp;
		refLevel++;
		SaySystem("恭喜您，成就等级提升到 %u 级!", refLevel);
		dwMaxExp = CTimeAchieve::GetInstance()->GetLevelMaxExp(refLevel);
		if (dwMaxExp == -1) break;
	}
}

VOID CHumanPlayer::PacketAchieve(xPacket& packet, BYTE btType, int nAchieveCount)
{
	switch (btType)
	{
	case 0:// i=0时,状态值(0未完成,1已完成/可领取,2已领取)
	{
		for (int i = 0; i < nAchieveCount; i++)
		{
			DWORD dwStatus = (DWORD)m_Achievement.btStatus[i];
			packet.push(&dwStatus, 4);
		}
	}
	break;
	case 1:// i=1时,近期获得成就
	{
		for (int i = 0; i < nAchieveCount; i++)
			packet.push(&m_Achievement.btRecentCount[i], 4);
	}
	break;
	case 2:// i=2时,完成时间戳
	{
		for (int i = 0; i < nAchieveCount; i++)
			packet.push(&m_Achievement.dwCompleteTime[i], 4);
	}
	break;
	}
}