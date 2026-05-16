#include "StdAfx.h"
#include ".\guildwarmanager.h"
#include ".\gameworld.h"
#include ".\guildex.h"

CGuildWarManager::CGuildWarManager(void)
{
	memset(m_pGuildWars, 0, sizeof(m_pGuildWars));
	m_nWarCount = 0;
	m_nUpdatePtr = 0;
}

CGuildWarManager::~CGuildWarManager(void)
{
}

BOOL CGuildWarManager::RequestWar(CGuildEx* pReqGuild, CGuildEx* pAtkGuild)
{
	if (this->m_nWarCount >= MAX_GUILD_WAR)
	{
		xError::setError(1001, "已经达到最大战争数!");
		return FALSE;
	}
	if (pReqGuild->IsAllyGuild(pAtkGuild))
		pReqGuild->BreakAlly(pAtkGuild->GetName());
	if (pAtkGuild->IsAllyGuild(pReqGuild))
		pAtkGuild->BreakAlly(pReqGuild->GetName());

	for (UINT n = 0; n < this->m_nWarCount; n++)
	{
		if ((this->m_pGuildWars[n]->pAtkGuild == pAtkGuild && this->m_pGuildWars[n]->pReqGuild == pReqGuild) ||
			(this->m_pGuildWars[n]->pAtkGuild == pReqGuild && this->m_pGuildWars[n]->pReqGuild == pAtkGuild))
		{
			xError::setError(1002, "无法重复申请行会战!");
			return FALSE;
		}
	}
	m_pGuildWars[this->m_nWarCount] = this->m_xWarPool.newObject();
	if (m_pGuildWars[this->m_nWarCount] == nullptr)
	{
		xError::setError(1003, "当前战争资源紧缺, 无法进行行会战!");
		return FALSE;
	}
	if (!pAtkGuild->AddKillGuild(pReqGuild))
	{
		xError::setError(1004, "和对方进行行会战的行会已经达到上限, 请稍候再试");
		return FALSE;
	}
	if (!pReqGuild->AddKillGuild(pAtkGuild))
	{
		pAtkGuild->RemoveKillGuild(pReqGuild);
		xError::setError(1005, "和您的行会进行行会战的行会已经达到上限, 请稍候再试");
		return FALSE;
	}
	m_pGuildWars[this->m_nWarCount]->pAtkGuild = pAtkGuild;
	m_pGuildWars[this->m_nWarCount]->pReqGuild = pReqGuild;
	m_pGuildWars[this->m_nWarCount]->tmrWar.Savetime(CGameWorld::GetInstance()->GetVar(EVI_GUILDWARTIME) * 1000);

	char szText[256];
	sprintf(szText, "%s 和 %s 行会战争开始, 持续三小时", pReqGuild->GetName(), pAtkGuild->GetName());
	pAtkGuild->SendWords(szText);
	pAtkGuild->ReviewAroundNameColor();
	pReqGuild->SendWords(szText);
	pReqGuild->ReviewAroundNameColor();
	this->m_nWarCount++;
	return TRUE;
}

//行会1 正在和您的行会进行行会战争
//行会1 和 行会2 行会战争开始, 持续三小时
VOID CGuildWarManager::Update()
{
	if (m_nWarCount > 0)
	{
		if (m_nUpdatePtr >= m_nWarCount)
			m_nUpdatePtr = 0;
		if (this->m_pGuildWars[m_nUpdatePtr]->tmrWar.IsTimeOut())
		{
			m_pGuildWars[m_nUpdatePtr]->pAtkGuild->RemoveKillGuild(m_pGuildWars[m_nUpdatePtr]->pReqGuild);
			m_pGuildWars[m_nUpdatePtr]->pReqGuild->RemoveKillGuild(m_pGuildWars[m_nUpdatePtr]->pAtkGuild);

			m_pGuildWars[m_nUpdatePtr]->pAtkGuild->ReviewAroundNameColor();
			m_pGuildWars[m_nUpdatePtr]->pReqGuild->ReviewAroundNameColor();

			char szText[256];
			sprintf(szText, "%s 和 %s 行会战争结束", m_pGuildWars[m_nUpdatePtr]->pReqGuild->GetName(), m_pGuildWars[m_nUpdatePtr]->pAtkGuild->GetName());
			m_pGuildWars[m_nUpdatePtr]->pAtkGuild->SendWords(szText);
			m_pGuildWars[m_nUpdatePtr]->pReqGuild->SendWords(szText);

			this->m_xWarPool.deleteObject(m_pGuildWars[m_nUpdatePtr]);
			m_nWarCount--;
			m_pGuildWars[m_nUpdatePtr] = m_pGuildWars[m_nWarCount];
			return;
		}
		m_nUpdatePtr++;
	}
}
