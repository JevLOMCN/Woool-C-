#include "StdAfx.h"
#include ".\mapscriptmanager.h"
#include "scriptevent.h"

CMapScriptManager::CMapScriptManager(void)
{
}

CMapScriptManager::~CMapScriptManager(void)
{
}

static BOOL MakeEventMapPosition(char* pszEvent, EventMapPosition& pos)
{
	if (*pszEvent == '[')pszEvent++;
	xStringsExtracter<5> spos(pszEvent, ",:|", "\t ");
	if (spos.getCount() < 3)return FALSE;
	pos.nMapId = StringToInteger(spos[0]);
	pos.x = StringToInteger(spos[1]);
	pos.y = StringToInteger(spos[2]);
	pos.dwFlag = 0;
	if (spos.getCount() > 3)
	{
		xStringsExtracter<2> flag(spos[3], "&", "\t ");
		for (UINT n = 0; n < flag.getCount(); n++)
		{
			if (_stricmp(flag[n], "enter") == 0)
				pos.dwFlag |= SEF_ENTER;
			else if (_stricmp(flag[n], "leave") == 0)
				pos.dwFlag |= SEF_LEAVE;
		}
	}
	else
		pos.dwFlag = SEF_ENTER;
	return TRUE;
}

VOID CMapScriptManager::Load(const char* pszFilename)
{
	//	[16:(300,300)]
	CStringFile sf(pszFilename);
	char* pLine = nullptr;
	xCharSet wht(" \t");
	xCharSet spl("]");
	int nParam;
	char* Params[4];
	EventMapPosition pos;
	for (UINT i = 0; i < sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == '#' || *pLine == 0)continue;
		nParam = ExtractStrings(pLine, wht, spl, Params, 4, FALSE, TRUE);
		if (nParam < 2)continue;
		if (*Params[0] != '[')continue;
		if (!MakeEventMapPosition(Params[0], pos))continue;
		//	addmapevent
		//AddTimeScript( &timeup, p );
		AddMapScript(&pos, Params[1]);
	}
}

VOID CMapScriptManager::AddMapScript(EventMapPosition* pPos, const char* pszPage)
{
	if (CScriptEvent::Create(pPos->nMapId, pPos->x, pPos->y, pPos->dwFlag, pszPage) != nullptr)
	{
		PRINT(ORANGE, "在%u地图(%u,%u)设置%s%s%s事件脚本 %s\n", pPos->nMapId, pPos->x, pPos->y,
			( pPos->dwFlag & SEF_ENTER ) == 0 ? "": "进入", 
			(( pPos->dwFlag & SEF_ENTER ) != 0 && ( pPos->dwFlag & SEF_LEAVE ) != 0 ) ? "和":"",
			( pPos->dwFlag & SEF_LEAVE ) == 0 ? "": "离开",
			pszPage );
	}
}