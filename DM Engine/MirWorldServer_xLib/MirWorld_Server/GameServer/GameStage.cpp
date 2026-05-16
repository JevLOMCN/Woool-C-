#include "stdafx.h"
#include "GameStage.h"
#include "tinyxml.h"
#include "HumanPlayer.h"

CGameStage::CGameStage(void)
{
	m_nStageCount = 0;
	memset(m_pStageList, 0, sizeof(m_pStageList));
	m_pStageVar = nullptr;
}

CGameStage::~CGameStage(void)
{
	Clear();
}

VOID CGameStage::Clear()
{
	delete m_pStageVar;
	m_pStageVar = nullptr;

	for (int i = 0; i < m_nStageCount; i++)
	{
		STAGEINFO* pStageInfo = m_pStageList[i];
		if (pStageInfo != nullptr)
		{
			STAGECONTENT* pContent = pStageInfo->pContents;
			while (pContent != nullptr)
			{
				pStageInfo->pContents = pContent->pNext;
				STAGEITEM* pItem = pContent->pItems;
				while (pItem != nullptr)
				{
					pContent->pItems = pItem->pNext;
					delete pItem;
					pItem = pContent->pItems;
				}
				delete pContent;
				pContent = pStageInfo->pContents;
			}
			m_StageHash.HDel(pStageInfo->szName);
			delete pStageInfo;
		}
		m_pStageList[i] = nullptr;
	}
	m_nStageCount = 0;
}

VOID CGameStage::Load(const char* pszFilename)
{
	TiXmlDocument doc(pszFilename);
	if (!doc.LoadFile(TIXML_ENCODING_UNKNOWN))
	{
		PRINT(ERROR_RED, "加载 GameStage.xml 文件失败: %s \n", pszFilename);
		return;
	}
	TiXmlElement* root = doc.FirstChildElement("GameStage");
	if (root == nullptr)
	{
		PRINT(ERROR_RED, "GameStage.xml 文件格式错误: 找不到 GameStage 根节点 \n");
		return;
	}
	// 读取StageVar节点（在GameStage根节点下）
	TiXmlElement* stageVarElem = root->FirstChildElement("StageVar");
	if (stageVarElem != nullptr)
	{
		m_pStageVar = new STAGEVAR;
		memset(m_pStageVar, 0, sizeof(STAGEVAR));
		const char* pszCurStage = stageVarElem->Attribute("cur_stage");
		if (pszCurStage != nullptr)
			o_strncpy(m_pStageVar->szCurStage, pszCurStage, 127);
		const char* pszCurDay = stageVarElem->Attribute("cur_day");
		if (pszCurDay != nullptr)
			o_strncpy(m_pStageVar->szCurDay, pszCurDay, 127);
		const char* pszSelfLv = stageVarElem->Attribute("self_lv");
		if (pszSelfLv != nullptr)
			o_strncpy(m_pStageVar->szSelfLv, pszSelfLv, 127);
	}

	for (TiXmlElement* stageElem = root->FirstChildElement("Stage"); stageElem != nullptr; stageElem = stageElem->NextSiblingElement("Stage"))
	{
		STAGEINFO* pStageInfo = new STAGEINFO;
		memset(pStageInfo, 0, sizeof(STAGEINFO));
		// 读取Stage属性
		stageElem->QueryIntAttribute("id", &pStageInfo->nId);
		const char* pszName = stageElem->Attribute("name");
		if (pszName != nullptr)
			o_strncpy(pStageInfo->szName, pszName, 63);

		stageElem->QueryIntAttribute("day", &pStageInfo->nDay);
		stageElem->QueryIntAttribute("maxlevel", &pStageInfo->nMaxLevel);
		// 读取Content列表
		STAGECONTENT* pLastContent = nullptr;
		for (TiXmlElement* contentElem = stageElem->FirstChildElement("Content"); contentElem != nullptr; contentElem = contentElem->NextSiblingElement("Content"))
		{
			STAGECONTENT* pContent = new STAGECONTENT;
			memset(pContent, 0, sizeof(STAGECONTENT));
			const char* pszContentName = contentElem->Attribute("name");
			if (pszContentName != nullptr)
				o_strncpy(pContent->szName, pszContentName, 63);
			contentElem->QueryIntAttribute("looks", &pContent->nLooks);
			// 读取Item列表
			STAGEITEM* pLastItem = nullptr;
			for (TiXmlElement* itemElem = contentElem->FirstChildElement("Item"); itemElem != nullptr; itemElem = itemElem->NextSiblingElement("Item"))
			{
				STAGEITEM* pItem = new STAGEITEM;
				memset(pItem, 0, sizeof(STAGEITEM));

				itemElem->QueryIntAttribute("looks", &pItem->nLooks);

				const char* pszDesc = itemElem->Attribute("desc");
				if (pszDesc != nullptr)
					o_strncpy(pItem->szDesc, pszDesc, 127);
				if (pLastItem == nullptr)
					pContent->pItems = pItem;
				else
					pLastItem->pNext = pItem;
				pLastItem = pItem;
			}
			if (pLastContent == nullptr)
				pStageInfo->pContents = pContent;
			else
				pLastContent->pNext = pContent;
			pLastContent = pContent;
		}

		if (m_nStageCount < MAXGAMESTAGE)
		{
			m_pStageList[m_nStageCount++] = pStageInfo;
			m_StageHash.HAdd(pStageInfo->szName, (LPVOID)pStageInfo);
		}
		else
		{
			STAGECONTENT* pContent = pStageInfo->pContents;
			while (pContent != nullptr)
			{
				pStageInfo->pContents = pContent->pNext;
				STAGEITEM* pItem = pContent->pItems;
				while (pItem != nullptr)
				{
					pContent->pItems = pItem->pNext;
					delete pItem;
					pItem = pContent->pItems;
				}
				delete pContent;
				pContent = pStageInfo->pContents;
			}
			delete pStageInfo;
			PRINT(ERROR_RED, "GameStage 列表已满, 无法加载更多数据\n");
		}
	}
}

static char g_szTempBuffer[65536];
VOID CGameStage::SendPlayerMapJumpHome(CHumanPlayer* pPlayer)
{
	xPacket packet(g_szTempBuffer, 65535);
	const char* s1C = "CheckPlayerMapJump";
	packet.push(s1C);
	packet.push(1);
	int nValue = 0x61;
	packet.push((LPVOID)&nValue, 1);
	nValue = m_nStageCount + 1;
	packet.push((LPVOID)&nValue, 4);
	//插入当前阶段名字
	char szCurStage[128];
	ProcFmtText(m_pStageVar->szCurStage, szCurStage, 128, pPlayer->GetScriptTarget());
	int nCurStage = StringToInteger(szCurStage);
	STAGEINFO* pStageInfo = m_pStageList[nCurStage];
	packet.push(pStageInfo->szName);
	packet.push(1);
	//插入阶段排序名字
	for (int i = 0; i < m_nStageCount; i++)
	{
		STAGEINFO* pStage = m_pStageList[i];
		if (pStage)
		{
			packet.push(pStage->szName);
			packet.push(1);
		}
	}
	nValue = 1;
	packet.push((LPVOID)&nValue, 4);
	char szCurDay[128];
	ProcFmtText(m_pStageVar->szCurDay, szCurDay, 128, pPlayer->GetScriptTarget());
	int nStageVar = StringToInteger(szCurDay);
	packet.push((LPVOID)&nStageVar, 4);
	packet.push(12); // 12个空
	nValue = 1;
	packet.push((LPVOID)&nValue, 4);
	packet.push((LPVOID)&nStageVar, 4);
	packet.push(4); // 4个空
	pPlayer->SendMsg(pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	// 发送当前阶段
	if (pStageInfo)
		SendPlayerMapJumpPage(pPlayer, pStageInfo->szName);
}

VOID CGameStage::SendPlayerMapJumpPage(CHumanPlayer* pPlayer, const char* pszName)
{
	STAGEINFO* pStageInfo = (STAGEINFO*)m_StageHash.HGet(pszName);
	if (pStageInfo == nullptr) return;
	xPacket packet(g_szTempBuffer, 65535);
	const char* s1C = "CheckPlayerMapJump";
	packet.push(s1C);
	packet.push(1);
	int nValue = 0x62;
	packet.push((LPVOID)&nValue, 1);
	STAGECONTENT* pContent = pStageInfo->pContents;
	int nContentCount = 0;
	while (pContent != nullptr)
	{
		nContentCount++;
		pContent = pContent->pNext;
	}
	packet.push((LPVOID)&nContentCount, 4);
	pContent = pStageInfo->pContents;
	while (pContent != nullptr)
	{
		char szBuffer[2048] = { 0 };
		int nPos = 0;
		nPos += sprintf_s(szBuffer + nPos, sizeof(szBuffer) - nPos, "%d;%s;",
			pContent->nLooks, pContent->szName);
		STAGEITEM* pItem = pContent->pItems;
		while (pItem != nullptr)
		{
			nPos += sprintf_s(szBuffer + nPos, sizeof(szBuffer) - nPos, "%d;%s;",
				pItem->nLooks, pItem->szDesc);
			pItem = pItem->pNext;
		}
		packet.push(szBuffer);
		packet.push(1);
		pContent = pContent->pNext;
	}
	nValue = 3;
	packet.push((LPVOID)&nValue, 4);
	nValue = pStageInfo->nDay;
	packet.push((LPVOID)&nValue, 4);
	nValue = pStageInfo->nMaxLevel;
	packet.push((LPVOID)&nValue, 4);
	char szSelfLv[128];
	ProcFmtText(m_pStageVar->szSelfLv, szSelfLv, 128, pPlayer->GetScriptTarget());
	int nSelfLv = StringToInteger(szSelfLv);
	packet.push((LPVOID)&nSelfLv, 4);
	packet.push(12); // 12个空
	nValue = 3;
	packet.push((LPVOID)&nValue, 4);
	nValue = pStageInfo->nDay;
	packet.push((LPVOID)&nValue, 4);
	packet.push(12); // 12个空
	packet.push((LPVOID)&nSelfLv, 4);
	packet.push(4); // 4个空
	pPlayer->SendMsg(pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
}