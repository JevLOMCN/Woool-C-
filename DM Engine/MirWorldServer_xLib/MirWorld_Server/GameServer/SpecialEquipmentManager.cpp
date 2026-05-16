#include "StdAfx.h"
#include ".\specialequipmentmanager.h"
#include ".\humanplayer.h"
#include ".\Itemmanager.h"

CSpecialEquipmentManager::CSpecialEquipmentManager(void)
{
	m_pSpecialEquipmentArray = nullptr;
	m_iSpecialEquipmentCount = 0;
}

CSpecialEquipmentManager::~CSpecialEquipmentManager(void)
{
	if (m_pSpecialEquipmentArray)
	{
		delete[]m_pSpecialEquipmentArray;
		m_pSpecialEquipmentArray = nullptr;
	}
	m_iSpecialEquipmentCount = 0;
}

BOOL CSpecialEquipmentManager::MatchFunction(CHumanPlayer* pPlayer, special_equipment_func func)
{
	DWORD dwPosFlag;
	return MatchFunction(pPlayer, func, dwPosFlag);
}

BOOL CSpecialEquipmentManager::MatchFunction(CHumanPlayer* pPlayer, special_equipment_func func, DWORD& dwPosFlag)
{
	if ((int)func < 0 || func >= SEF_MAX)return FALSE;
	int index = this->m_SpecialFunctionDefine[func].index;
	if (index < 0 || index >= m_iSpecialEquipmentCount)return FALSE;
	int groupid = -1;
	match_state s = MS_MATCHGROUP;
	int posout = -1;
	dwPosFlag = 0;
	for (int i = index; i < m_iSpecialEquipmentCount; i++)
	{
		if (func != this->m_pSpecialEquipmentArray[i].func)
			break;
		if (s == MS_NEXTGROUP)
		{
			if (this->m_pSpecialEquipmentArray[i].group == groupid)continue;
			s = MS_MATCHGROUP;
			dwPosFlag = 0;
			posout = -1;
			groupid = -1;
		}
		if (groupid != this->m_pSpecialEquipmentArray[i].group)
		{
			if (groupid != -1)break;
			groupid = this->m_pSpecialEquipmentArray[i].group;
		}
		if (!pPlayer->CheckEquipment(this->m_pSpecialEquipmentArray[i].pos, this->m_pSpecialEquipmentArray[i].stdmode,
			this->m_pSpecialEquipmentArray[i].image, posout))
			s = MS_NEXTGROUP;
		if (posout != -1)
		{
			dwPosFlag |= (1 << posout);
		}
	}
	if (s == MS_NEXTGROUP || groupid == -1)
		return FALSE;
	return TRUE;
}

VOID CSpecialEquipmentManager::LoadSpecialEquipmentFunction(const char* pszFilename)
{
	UINT nElementCount = 0;
	CSettingFile sf;
	char szGroupname[20];
	if (!sf.Open(pszFilename))
		return;
	for (int t = 0; t < SEF_MAX; t++)
	{
		int groupcount = sf.GetInteger(g_special_equipment_table[t], "Count", 0);
		if (groupcount == 0)continue;
		for (int i = 0; i < groupcount; i++)
		{
			sprintf(szGroupname, "Group%u", i + 1);
			char* p = (char*)sf.GetString(g_special_equipment_table[t], szGroupname, nullptr);
			if (p == nullptr)continue;
			nElementCount += countStringChar(p, '&') + 1;
		}
	}
	nElementCount += SEF_MAX;
	this->m_pSpecialEquipmentArray = new SpecialEquipment[nElementCount];
	m_iSpecialEquipmentCount = 0;
	for (int t = 0; t < SEF_MAX; t++)
	{
		m_SpecialFunctionDefine[t].index = m_iSpecialEquipmentCount;
		int groupcount = sf.GetInteger(g_special_equipment_table[t], "Count", 0);
		char* pSpecial = (char*)sf.GetString(g_special_equipment_table[t], "Special", nullptr);
		if (pSpecial == nullptr)
			m_SpecialFunctionDefine[t].pszSpecial = nullptr;
		else
			m_SpecialFunctionDefine[t].pszSpecial = copystring(pSpecial);
		pSpecial = (char*)sf.GetString(g_special_equipment_table[t], "Params", nullptr);
		if (pSpecial != nullptr)
		{
			xStringsExtracter<100> spp(pSpecial, ",", " \t");
			m_SpecialFunctionDefine[t].paramcount = spp.getCount();
			m_SpecialFunctionDefine[t].pParams = new DWORD[m_SpecialFunctionDefine[t].paramcount];
			for (int paramindex = 0; paramindex < m_SpecialFunctionDefine[t].paramcount; paramindex++)
			{
				m_SpecialFunctionDefine[t].pParams[paramindex] = StringToInteger(spp[paramindex]);
			}
		}
		else
			m_SpecialFunctionDefine[t].paramcount = 0;
		if (groupcount == 0)continue;
		int groupindex = 0;
		for (int i = 0; i < groupcount; i++)
		{
			sprintf(szGroupname, "Group%u", i + 1);
			char* p = (char*)sf.GetString(g_special_equipment_table[t], szGroupname, nullptr);
			if (p == nullptr)continue;
			groupindex++;
			xStringsExtracter<20> ss(p, "&", " \t[]");
			for (UINT j = 0; j < ss.getCount(); j++)
			{
				xStringsExtracter<20> q(ss[j], ":", " \t");
				if (q.getCount() < 2 || q.getCount() > 3)continue;
				m_pSpecialEquipmentArray[m_iSpecialEquipmentCount].func = (special_equipment_func)t;
				m_pSpecialEquipmentArray[m_iSpecialEquipmentCount].group = groupindex;
				if (q.getCount() == 3)
				{
					m_pSpecialEquipmentArray[m_iSpecialEquipmentCount].pos = q[2][0] == '*' ? (-1) : StringToInteger(q[2]);
					m_pSpecialEquipmentArray[m_iSpecialEquipmentCount].image = q[1][0] == '*' ? (-1) : StringToInteger(q[1]);
					m_pSpecialEquipmentArray[m_iSpecialEquipmentCount].stdmode = q[0][0] == '*' ? (-1) : StringToInteger(q[0]);
				}
				else if (q.getCount() == 2)
				{
					ITEMCLASS* pClass = CItemManager::GetInstance()->GetItemClassByName(q[0]);
					if (pClass == nullptr)continue;
					m_pSpecialEquipmentArray[m_iSpecialEquipmentCount].pos = q[1][0] == '*' ? (-1) : StringToInteger(q[1]);
					m_pSpecialEquipmentArray[m_iSpecialEquipmentCount].image = pClass->wImage;
					m_pSpecialEquipmentArray[m_iSpecialEquipmentCount].stdmode = pClass->stdMode;
				}
				m_iSpecialEquipmentCount++;
			}
		}
	}
	sf.Close();
}