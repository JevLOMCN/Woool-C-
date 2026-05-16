#include "StdAfx.h"
#include <sstream>
#include "GuildManagerEx.h"
#include "GuildEx.h"

CGuildManagerEx::CGuildManagerEx(void)
{
}

CGuildManagerEx::~CGuildManagerEx(void)
{
}

CGuildEx* CGuildManagerEx::FindGuild(const char* pszName)
{
	return (CGuildEx*)m_xGuildList.ObjectOf(pszName);
}

BOOL CGuildManagerEx::BuildGuild(CHumanPlayer* pCreator, const char* pszName)
{
	CGuildEx* pGuild = FindGuild(pszName);
	if (pGuild != nullptr) return FALSE;
	pGuild = NewGuild(pszName);
	if (m_xGuildList.Add(pszName, (LPVOID)pGuild) == -1)
	{
		DeleteGuild(pGuild);
		return FALSE;
	}
	if (!pGuild->Create(pCreator, pszName))
	{
		m_xGuildList.Delete(pszName);
		DeleteGuild(pGuild);
		return FALSE;
	}
	return TRUE;
}

VOID CGuildManagerEx::OnFoundFile(const char* pszFileName, UINT nParam)
{
	CGuildEx* pGuild = NewGuild();
	if (pGuild)
	{
		if (pGuild->Load(pszFileName))
		{
			if (m_xGuildList.Add(pGuild->GetName(), (LPVOID)pGuild) == -1)
			{
				DeleteGuild(pGuild);
				return;
			}
		}
		else
			DeleteGuild(pGuild);
	}
}

VOID CGuildManagerEx::LoadGuildUpExp(const char* pszfile, DWORD dwCount)
{
	CSettingFile sfGuild;
	if (!sfGuild.Open(pszfile)) return;
	char sectionName[64];
	sprintf(sectionName, "GuildUpExp");
	for (int i = 0; i <= dwCount; i++)
	{
		char itemName[5];
		sprintf(itemName, "lv%d", i);
		m_UpExp.push_back(sfGuild.GetDword(sectionName, itemName, 0));
	}
	sfGuild.Close();
}

CGuildEx* CGuildManagerEx::NewGuild(const char* pszName)
{
	CGuildEx* pGuild = this->m_xGuildPool.newObject();
	if (pGuild == nullptr) return nullptr;
	if (pszName)pGuild->SetName(pszName);
	return pGuild;
}

VOID CGuildManagerEx::DeleteGuild(CGuildEx* pGuild)
{
	pGuild->Clear();
	this->m_xGuildPool.deleteObject(pGuild);
}

VOID CGuildManagerEx::SaveAll()
{
	for (UINT i = 0; i < this->m_xGuildList.GetCount(); i++)
	{
		if (this->m_xGuildList[i]->lpObject)
		{
			CGuildEx* p = (CGuildEx*)this->m_xGuildList[i]->lpObject;
			p->Save();
		}
	}
}

VOID CGuildManagerEx::GuildsRankInfo(GuildRankInfo*& pTempArray, int& nValidCount)
{
	int guildListCount = this->m_xGuildList.GetCount();
	// 创建临时数组存储所有行会信息
	pTempArray = new GuildRankInfo[guildListCount];
	// 收集所有有效的行会
	for (int i = 0; i < guildListCount; i++)
	{
		if (this->m_xGuildList[i]->lpObject)
		{
			CGuildEx* pGuild = (CGuildEx*)this->m_xGuildList[i]->lpObject;
			if (pGuild && nValidCount < guildListCount)
			{
				pTempArray[nValidCount].pGuild = pGuild;
				pTempArray[nValidCount].nLevel = pGuild->GetLevel();
				pTempArray[nValidCount].nExp = pGuild->GetExp();
				nValidCount++;
			}
		}
	}
	// 按等级排序, 等级相同则按经验排序
	for (int i = 0; i < nValidCount - 1; i++)
	{
		for (int j = 0; j < nValidCount - i - 1; j++)
		{
			if (pTempArray[j].nLevel < pTempArray[j + 1].nLevel ||
				(pTempArray[j].nLevel == pTempArray[j + 1].nLevel && pTempArray[j].nExp < pTempArray[j + 1].nExp))
			{
				GuildRankInfo temp = pTempArray[j];
				pTempArray[j] = pTempArray[j + 1];
				pTempArray[j + 1] = temp;
			}
		}
	}
}

VOID CGuildManagerEx::GetPlayerTopGuild(CGuildEx* pGuild, GuildRank& pGuildRank)
{
	if (pGuild == nullptr) return;
	// 获取临时数组存储所有行会排序信息
	GuildRankInfo* pTempArray = nullptr;
	int nValidCount = 0;
	GuildsRankInfo(pTempArray, nValidCount);
	if (pTempArray == nullptr) return;
	// 找到行会排名
	for (int i = 0; i < nValidCount; i++)
	{
		if (strcmp(pTempArray[i].pGuild->GetName(), pGuild->GetName()) == 0)
		{
			pGuildRank.wIndex = i + 1;
			o_strncpy(pGuildRank.szGuildName, pTempArray[i].pGuild->GetName(), 29);
			pGuildRank.wNum46 = pGuild->GetMemberCountFor46Level();
			pGuildRank.wNumYs = 0;
			pGuildRank.wNum = pTempArray[i].pGuild->GetMemberCount();
			pGuildRank.dwExp = pTempArray[i].pGuild->GetExp();
			pGuildRank.wLevel = pTempArray[i].pGuild->GetLevel();
			break;
		}
	}
	delete[] pTempArray;
}

int CGuildManagerEx::GetTopGuilds(GuildRank* pGuilds, int maxCount)
{
	if (pGuilds == nullptr || maxCount == 0) return 0;
	// 获取临时数组存储所有行会排序信息
	GuildRankInfo* pTempArray = nullptr;
	int nValidCount = 0;
	GuildsRankInfo(pTempArray, nValidCount);
	if (pTempArray == nullptr) return 0;
	// 复制前maxCount个到输出数组
	int nResult = (nValidCount < maxCount) ? nValidCount : maxCount;
	for (int i = 0; i < nResult; i++)
	{
		pGuilds[i].wIndex = i + 1;
		o_strncpy(pGuilds[i].szGuildName, pTempArray[i].pGuild->GetName(), 29);
		pGuilds[i].wNum46 = pTempArray[i].pGuild->GetMemberCountFor46Level();
		pGuilds[i].wNumYs = 0;
		pGuilds[i].wNum = pTempArray[i].pGuild->GetMemberCount();
		pGuilds[i].dwExp = pTempArray[i].pGuild->GetExp();
		pGuilds[i].wLevel = pTempArray[i].pGuild->GetLevel();
	}
	delete[] pTempArray;
	return nResult;
}

std::string CGuildManagerEx::GetRecruitStateList()
{
	std::string sGuildList;
	int guildListCount = this->m_xGuildList.GetCount();
	for (int i = 0; i < guildListCount; i++)
	{
		if (this->m_xGuildList[i]->lpObject)
		{
			CGuildEx* pGuild = (CGuildEx*)this->m_xGuildList[i]->lpObject;
			if (pGuild && pGuild->GetRecruitState())
			{
				std::stringstream ss;
				ss << pGuild->GetName() << "#" << pGuild->GetLevel() << "#"
					<< pGuild->GetFirstOwnerName() << "#" << pGuild->GetDeclarationList() << "#"
					<< pGuild->GetMemberCount() << "#" << "100";
				sGuildList += ss.str();
			}
		}
	}
	return sGuildList;
}