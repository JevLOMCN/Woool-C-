#include "StdAfx.h"
#include "logicmap.h"
#include "logicmapmgr.h"
//#include ".\minelist.h"
//
//CMineList::CMineList(void)
//{
//}
//
//CMineList::~CMineList(void)
//{
//}
VOID LoadMineList(const char* pszFileName)
{
	CStringFile sf(pszFileName);
	sf.MakeDeflate();
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		if (*sf[i] == '#')continue;
		xStringsExtracter<20> mine(sf[i], ",", " \t");
		//	mapid, name, duramin, duramax, rate
		if (mine.getCount() >= 5)
		{
			UINT nMapId = StringToInteger(mine[0]);
			CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
			if (pMap)
			{
				pMap->AddMineItem(mine[1], (WORD)StringToInteger(mine[2]), (WORD)StringToInteger(mine[3]), (WORD)StringToInteger(mine[4]));
			}
		}
	}
}