#include "StdAfx.h"
#include "TopManager.h"
#include "HumanPlayer.h"
#include "NpcManager.h"
#include "GuildManagerEx.h"
#include "ScriptNpc.h"
#include "GuildEx.h"

CTopManager::CTopManager(void)
{
	memset(&m_ProTopNpc, 0, sizeof(m_ProTopNpc));
	m_szListFile[0] = 0;
	m_iTop100Count = 0;
}

CTopManager::~CTopManager(void)
{
}

VOID CTopManager::EnterTop100(CHumanPlayer* pPlayer)
{
	WORD wLevel = pPlayer->GetPropValue(PI_LEVEL);
	BYTE btPerCent = pPlayer->GetPropValue(PI_EXPPERCENT);
	DWORD dwDBId = pPlayer->GetDBId();

	int fitpos = -1;
	int mypos = -1;

	for (int i = 0; i < m_iTop100Count; i++)
	{
		if (m_Top100[i].dwDBId == dwDBId)
			mypos = i;
		else if (wLevel > m_Top100[i].wLevel || (wLevel == m_Top100[i].wLevel && btPerCent > m_Top100[i].btPerCent))
		{
			fitpos = i;
			if (mypos != -1)break;
		}
	}
	if (fitpos == -1)
	{
		if (mypos == -1)
		{
			if (m_iTop100Count < 100)
			{
				m_Top100[m_iTop100Count].btClass = pPlayer->GetPro();
				m_Top100[m_iTop100Count].btSex = pPlayer->GetSex();
				m_Top100[m_iTop100Count].dwDBId = dwDBId;
				m_Top100[m_iTop100Count].btPerCent = btPerCent;
				GetLocalTime(&m_Top100[m_iTop100Count].stRanking);
				m_Top100[m_iTop100Count].wLevel = wLevel;
				o_strncpy(m_Top100[m_iTop100Count].szGuild, pPlayer->GetGuildName(), 30);
				o_strncpy(m_Top100[m_iTop100Count].szName, pPlayer->GetName(), 16);
				m_iTop100Count++;
				return;
			}
			else
				return;
		}
		else
		{
			if (mypos < m_iTop100Count - 1)
			{
				TopCharInfo info = m_Top100[mypos];
				memmove(&m_Top100[mypos], &m_Top100[mypos + 1], sizeof(TopCharInfo) * (m_iTop100Count - mypos - 1));
				m_Top100[m_iTop100Count - 1] = info;
			}
			return;
		}
	}
	else
	{
		if (mypos == -1)
		{
			if (m_iTop100Count >= 100) // 通知某人, 你落榜了～
				memmove(&m_Top100[fitpos + 1], &m_Top100[fitpos], sizeof(TopCharInfo) * (m_iTop100Count - fitpos - 1));
			else
			{
				memmove(&m_Top100[fitpos + 1], &m_Top100[fitpos], sizeof(TopCharInfo) * (m_iTop100Count - fitpos));
				m_iTop100Count++;
			}
			m_Top100[fitpos].btClass = pPlayer->GetPro();
			m_Top100[fitpos].btSex = pPlayer->GetSex();
			m_Top100[fitpos].dwDBId = dwDBId;
			m_Top100[fitpos].btPerCent = btPerCent;
			GetLocalTime(&m_Top100[fitpos].stRanking);
			m_Top100[fitpos].wLevel = wLevel;
			o_strncpy(m_Top100[fitpos].szGuild, pPlayer->GetGuildName(), 30);
			o_strncpy(m_Top100[fitpos].szName, pPlayer->GetName(), 16);
			return;
		}
		else
		{
			TopCharInfo info = m_Top100[mypos];
			if (fitpos < mypos)
				memmove(&m_Top100[fitpos + 1], &m_Top100[fitpos], sizeof(TopCharInfo) * (mypos - fitpos));
			else
				memmove(&m_Top100[mypos], &m_Top100[mypos + 1], sizeof(TopCharInfo) * (fitpos - mypos));
			m_Top100[fitpos] = info;
		}
	}
	return;
}

BOOL CTopManager::EnterProfessionTop(CHumanPlayer* pPlayer)
{
	BYTE btClass = pPlayer->GetPro();
	BYTE btSex = pPlayer->GetSex();
	WORD wLevel = pPlayer->GetPropValue(PI_LEVEL);
	BYTE btPerCent = pPlayer->GetPropValue(PI_EXPPERCENT);
	UINT index = btClass * 2 + btSex;
	if (index >= 6)return FALSE;
	if (wLevel < m_ProfessionTop[index].wLevel || (wLevel == m_ProfessionTop[index].wLevel &&
		btPerCent <= m_ProfessionTop[index].btPerCent))
		return FALSE;
	m_ProfessionTop[index].btClass = btClass;
	m_ProfessionTop[index].btSex = btSex;
	m_ProfessionTop[index].dwDBId = pPlayer->GetDBId();
	m_ProfessionTop[index].btPerCent = btPerCent;
	m_ProfessionTop[index].wLevel = wLevel;
	GetLocalTime(&m_ProfessionTop[index].stRanking);
	o_strncpy(m_ProfessionTop[index].szName, pPlayer->GetName(), 16);
	Save(m_szListFile);
	if (m_ProTopNpc[index])
	{
		char szName[128];
		sprintf_s(szName, sizeof(szName), "%s\\%s\\", m_ProTopNpc[index]->GetName(), pPlayer->GetName());
		m_ProTopNpc[index]->SetLongName(szName);
		m_ProTopNpc[index]->SendChangeName();
	}
	return TRUE;
}

VOID CTopManager::SendRank(CHumanPlayer* pPlayer, int nType, int StartPos)
{
	if (pPlayer == nullptr) return;
	EnterTop100(pPlayer);
	if (nType == 0) return;
	if (StartPos > 500 || StartPos < 0) StartPos = 1;
	if (StartPos == 0)
	{
		if (nType >= 0x100 && nType < 0x200) // 自己的英雄榜位置
		{
			LevelRank LevelRank;
			LevelRank.wIndex = GetTopIndex(pPlayer->GetName());
			o_strncpy(LevelRank.szName, pPlayer->GetName(), 13);
			LevelRank.btLevel = pPlayer->GetPropValue(PI_LEVEL);
			LevelRank.btJob = pPlayer->GetPro();
			LevelRank.btSex = pPlayer->GetSex();
			if (CGuildEx* pGuild = pPlayer->GetGuild())
				o_strncpy(LevelRank.szGuildName, pGuild->GetName(), 29);
			else
				o_strncpy(LevelRank.szGuildName, "暂无", 29);
			LevelRank.btPerCent = pPlayer->GetPropValue(PI_HPPERCENT);
			pPlayer->SendMsg(LevelRank.wIndex, 0x9704, nType, MAKEWORD(0, 1), 65535, &LevelRank, sizeof(LevelRank));
		}
		if (nType >= 0x200 && nType < 0x300) // 天下名师榜
		{
			MasterRank MasterRank;
			MasterRank.wIndex = 1;
			o_strncpy(MasterRank.szName, "名师名字", 13);
			MasterRank.wTudiCount = 13;
			MasterRank.btLevel = 55;
			MasterRank.btJob = 0;
			MasterRank.btSex = 0;
			o_strncpy(MasterRank.szGuild, "名师行会", 29);
			MasterRank.wRepute = 5;
			MasterRank.btNowTudiCount = 3;
			DWORD TotlePos = 10;
			pPlayer->SendMsg(TotlePos, 0x9704, nType, MAKEWORD(0, 2), 65535, &MasterRank, sizeof(MasterRank));
		}
		if (nType >= 0x300 && nType < 0x400) // 元神排行榜
		{
			ShadowRank ShadowRank;
			ShadowRank.wIndex = 1;
			o_strncpy(ShadowRank.szName, "我元神名字", 13);
			ShadowRank.btLevel = 7;
			ShadowRank.btJob = 0;
			ShadowRank.btSex = 0;
			ShadowRank.btTiPo = 0;
			o_strncpy(ShadowRank.szMasterName, "主体名字", 13);
			ShadowRank.btMasterLevel = 35;
			o_strncpy(ShadowRank.szMasterGuildName, "主体行会", 29);
			DWORD TotlePos = 10;
			pPlayer->SendMsg(TotlePos, 0x9704, nType, 0, 65535, &ShadowRank, sizeof(ShadowRank));
		}
		if (nType >= 0x400 && nType < 0x500) // 天地灵兽榜
		{
			PetRank PetRank;
			PetRank.wIndex = 1;
			o_strncpy(PetRank.szName, "豹子名称", 13);
			PetRank.wLevel = 7;
			PetRank.btPerCent = 199;
			o_strncpy(PetRank.szFengHao, "天第一豹", 13);
			o_strncpy(PetRank.szMaster, "是我", 13);
			DWORD TotlePos = 10;
			pPlayer->SendMsg(TotlePos, 0x9704, nType, MAKEWORD(0, 4), 65535, &PetRank, sizeof(PetRank));
		}

		if (nType >= 0x600 && nType < 0x700) // 行会榜
		{ 
			CGuildEx* guild = pPlayer->GetGuild();
			if (!guild) return;
			GuildRank GuildRank;
			CGuildManagerEx::GetInstance()->GetPlayerTopGuild(guild, GuildRank);
			DWORD TotlePos = 10;
			pPlayer->SendMsg(GuildRank.wIndex, 0x9704, nType, MAKEWORD(0, 6), 65535, &GuildRank, sizeof(GuildRank));
		}
		return;
	}
	if (nType >= 0x100 && nType < 0x200) // 英雄榜
	{
		int SendCount = 0;
		LevelRank LevelRankArray[10];
		for (int i = StartPos-1; i < m_iTop100Count; i++)
		{
			TopCharInfo* charInfo = GetTop100CharInfo(i, nType);
			if (charInfo == nullptr) continue;
			LevelRankArray[SendCount].wIndex = GetTopIndex(charInfo->szName);
			o_strncpy(LevelRankArray[SendCount].szName, charInfo->szName, 13);
			LevelRankArray[SendCount].btLevel = charInfo->wLevel;
			LevelRankArray[SendCount].btJob = charInfo->btClass;
			LevelRankArray[SendCount].btSex = charInfo->btSex;
			if (charInfo->szGuild[0] != '\0')
				o_strncpy(LevelRankArray[SendCount].szGuildName, charInfo->szGuild, 29);
			else
				o_strncpy(LevelRankArray[SendCount].szGuildName, "暂无", 29);
			LevelRankArray[SendCount].btPerCent = charInfo->btPerCent;
			SendCount++;
			if (SendCount == 10) break;
		}
		pPlayer->SendMsg(GetTopIndex(pPlayer->GetName()), 0x9704, nType, MAKEWORD(0, 1), SendCount, &LevelRankArray, sizeof(LevelRank) * SendCount);
	}
	if (nType >= 0x200 && nType < 0x300) // 天下名师榜
	{
		int SendCount = 0;
		DWORD TotlePos = 10;
		MasterRank MasterRankArray[10];
		for (int i = 0; i < TotlePos; i++)
		{
			MasterRankArray[i].wIndex = i + 1;
			o_strncpy(MasterRankArray[i].szName, "名师名字", 13);
			MasterRankArray[i].wTudiCount = 13;
			MasterRankArray[i].btLevel = 55;
			MasterRankArray[i].btJob = 0;
			MasterRankArray[i].btSex = 0;
			o_strncpy(MasterRankArray[i].szGuild, "名师行会", 29);
			MasterRankArray[i].wRepute = 5;
			MasterRankArray[i].btNowTudiCount = 3;
			SendCount++;
			if (SendCount == 10) break;
		}
		pPlayer->SendMsg(SendCount, 0x9704, nType, MAKEWORD(0, 2), SendCount, &MasterRankArray, sizeof(MasterRank) * 10);
	}
	if (nType >= 0x300 && nType < 0x400) // 元神排行榜
	{
		int SendCount = 0;
		DWORD TotlePos = 10;
		ShadowRank ShadowRankArray[10];
		for (int i = 0; i < TotlePos; i++)
		{
			ShadowRankArray[SendCount].wIndex = i + 1;
			o_strncpy(ShadowRankArray[SendCount].szName, "元神名字", 13);
			ShadowRankArray[SendCount].btLevel = 7;
			ShadowRankArray[SendCount].btJob = 0;
			ShadowRankArray[SendCount].btSex = 0;
			ShadowRankArray[SendCount].btTiPo = 2;
			o_strncpy(ShadowRankArray[SendCount].szMasterName, "主人名字", 13);
			ShadowRankArray[SendCount].btMasterLevel = 7;
			o_strncpy(ShadowRankArray[SendCount].szMasterGuildName, "主人行会", 29);
			SendCount++;
			if (SendCount == 10) break;
		}
		pPlayer->SendMsg(SendCount, 0x9704, nType, 0, SendCount, &ShadowRankArray, sizeof(ShadowRank) * SendCount);
	}
	if (nType >= 0x400 && nType < 0x500) // 天地灵兽榜
	{
		PetRank PetRankArray[10];
		int SendCount = 0;
		DWORD TotlePos = 10;
		for (int i = 0; i < TotlePos; i++)
		{
			PetRankArray[i].wIndex = i + 1;
			o_strncpy(PetRankArray[i].szName, "豹子名称", 13);
			PetRankArray[i].wLevel = MIN(i, 7);
			PetRankArray[i].btPerCent = 199;
			o_strncpy(PetRankArray[i].szFengHao, "天第一豹", 13);
			o_strncpy(PetRankArray[i].szMaster, "是我", 13);
			SendCount++;
			if (SendCount == 10) break;
		}
		pPlayer->SendMsg(SendCount, 0x9704, nType, MAKEWORD(0, 4), SendCount, &PetRankArray, sizeof(PetRank) * SendCount);
	}
	if (nType >= 0x600 && nType < 0x700) // 行会榜
	{
		CGuildEx* guild = pPlayer->GetGuild();
		GuildRank pGuildRank; //获取玩家的行会排名
		WORD wIndex = 0;
		if (guild)
		{
			CGuildManagerEx::GetInstance()->GetPlayerTopGuild(guild, pGuildRank);
			wIndex = pGuildRank.wIndex;
		}
		DWORD TotlePos = 10;
		GuildRank GuildRankArray[10];
		int SendCount = CGuildManagerEx::GetInstance()->GetTopGuilds(GuildRankArray, TotlePos);
		pPlayer->SendMsg(wIndex, 0x9704, nType, MAKEWORD(0,6), SendCount, &GuildRankArray, sizeof(GuildRank)* SendCount);
	}
}

VOID CTopManager::LoadFigure(const char* pszFile)
{
	CSettingFile sf;
	char szItemName[200];
	if (sf.Open(pszFile))
	{
		for (UINT i = 0; i < 6; i++)
		{
			sprintf(szItemName, "top_%u_%u", i / 2, i % 2);
			char* p = (char*)sf.GetString("npc", szItemName, "");
			if (p[0] == 0)continue;
			m_ProTopNpc[i] = CNpcManager::GetInstance()->AddNpc(p);
		}
		sf.Close();
	}
}

VOID CTopManager::Load(const char* pszFile)
{
	CSettingFile sf;
	o_strncpy(m_szListFile, pszFile, 1023);
	char szItemName[200];
	if (sf.Open(pszFile))
	{
		for (UINT i = 0; i < 6; i++)
		{
			// name,dbid,class,sex,level,exp,date
			sprintf(szItemName, "top_%u_%u", i / 2, i % 2);
			char* p = (char*)sf.GetString("player", szItemName, "");
			if (p[0] == 0)continue;
			xStringsExtracter<8> top(p, ",");
			if (top.getCount() < 7)continue;
			m_ProfessionTop[i].btClass = (BYTE)StringToInteger(top[2]);
			m_ProfessionTop[i].btSex = (BYTE)StringToInteger(top[3]);
			m_ProfessionTop[i].dwDBId = (DWORD)StringToInteger(top[1]);
			m_ProfessionTop[i].btPerCent = (DWORD)StringToInteger(top[5]);
			o_strncpy(m_ProfessionTop[i].szName, top[0], 16);
			m_ProfessionTop[i].wLevel = (WORD)StringToInteger(top[4]);
			GetTimeFromString(m_ProfessionTop[i].stRanking, top[6]);
			if (top.getCount() > 7)
				o_strncpy(m_ProfessionTop[i].szGuild, top[7], 60);
			if (m_ProfessionTop[i].szName[0] != 0)
			{
				if (m_ProTopNpc[i])
				{
					char szName[128];
					sprintf(szName, "%s\\%s\\", m_ProTopNpc[i]->GetName(), m_ProfessionTop[i].szName);
					m_ProTopNpc[i]->SetLongName(szName);
					m_ProTopNpc[i]->SendChangeName();
				}
			}
		}
		sf.Close();
	}
}

VOID CTopManager::Save(const char* pszFile)const
{
	if (pszFile[0] == 0)return;
	FILE* fp = fopen(pszFile, "w");
	if (fp == nullptr)return;
	fprintf(fp, "[player]\n");
	for (UINT i = 0; i < 6; i++)
	{
		// name/dbid/class/sex/level/exp/date
		if (m_ProfessionTop[i].szName[0] != 0)
		{
			fprintf(fp, "top_%u_%u = %s,%u,%u,%u,%u,%u,%04u-%02u-%02u,%s\n", i / 2, i % 2,
				m_ProfessionTop[i].szName, m_ProfessionTop[i].dwDBId, m_ProfessionTop[i].btClass, m_ProfessionTop[i].btSex,
				m_ProfessionTop[i].wLevel, m_ProfessionTop[i].btPerCent, m_ProfessionTop[i].stRanking.wYear,
				m_ProfessionTop[i].stRanking.wMonth, m_ProfessionTop[i].stRanking.wDay, m_ProfessionTop[i].szGuild
			);
		}
		else
			fprintf(fp, "top_%u_%u = \n", i / 2, i % 2);
	}
	fclose(fp);
}

VOID CTopManager::SetTopNpc(CScriptNpc* pNpc, BYTE btPro, BYTE btSex)
{
	UINT index = btPro * 2 + btSex;
	if (index >= 6)return;
	m_ProTopNpc[index] = pNpc;
}

BOOL CTopManager::UpdateTopInfo(CHumanPlayer* pPlayer)
{
	BYTE btClass = pPlayer->GetPro();
	BYTE btSex = pPlayer->GetSex();
	UINT index = btClass * 2 + btSex;
	if (index >= 6)return FALSE;
	if (m_ProfessionTop[index].dwDBId == pPlayer->GetDBId())
	{
		if (CGuildEx* pGuild = pPlayer->GetGuild())
			o_strncpy(m_ProfessionTop[index].szGuild, pGuild->GetName(), 60);
		else
			m_ProfessionTop[index].szGuild[0] = 0;
		WORD wLevel = pPlayer->GetPropValue(PI_LEVEL);
		DWORD btPerCent = pPlayer->GetPropValue(PI_EXPPERCENT);
		m_ProfessionTop[index].btPerCent = btPerCent;
		m_ProfessionTop[index].wLevel = wLevel;
		Save(m_szListFile);
		return TRUE;
	}
	return FALSE;
}

int CTopManager::GetTopView(int index)
{
	static int view[6] = { 0x64, 0x65, 0x68, 0x69, 0x66, 0x67 };
	if (index < 0 || index >= 6)return 0x64;
	if (m_ProTopNpc[index])
		return m_ProTopNpc[index]->GetView();
	return view[index];
}

int CTopManager::GetTopIndex(const char* szName)const
{
	for (int i = 0; i < m_iTop100Count; i++)
	{
		if (strcmp(m_Top100[i].szName, szName) == 0)
			return i + 1;
	}
	return -1;
}