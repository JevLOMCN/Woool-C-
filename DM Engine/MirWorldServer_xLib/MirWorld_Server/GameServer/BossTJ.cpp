#include "stdafx.h"
#include "BossTJ.h"
#include "tinyxml.h"
#include "HumanPlayer.h"
#include "SystemScript.h"
#include "AutoScriptManager.h"

CBossTJ::CBossTJ(void)
{
	m_tmrUpdate.Savetime();
	m_nBossTJCount = 0;
	memset(m_pBossTJList, 0, sizeof(m_pBossTJList));
}

CBossTJ::~CBossTJ(void)
{
	Clear();
}

VOID CBossTJ::Clear()
{
	for (int i = 0; i < m_nBossTJCount; i++)
	{
		BOSS_TJ* pBossTJ = m_pBossTJList[i];
		if (pBossTJ != nullptr)
		{
			BOSSITEM* pItem = pBossTJ->pItems;
			while (pItem != nullptr)
			{
				pBossTJ->pItems = pItem->pNext;
				delete pItem;
				pItem = pBossTJ->pItems;
			}
			m_BossTJHash.HDel(pBossTJ->szName);
			delete pBossTJ;
		}
		m_pBossTJList[i] = nullptr;
	}
	m_nBossTJCount = 0;
}

VOID CBossTJ::Load(const char* pszFilename)
{
	TiXmlDocument doc(pszFilename);
	if (!doc.LoadFile(TIXML_ENCODING_UNKNOWN))
	{
		PRINT(ERROR_RED, "加载 BossTJ.xml 文件失败: %s \n", pszFilename);
		return;
	}

	TiXmlElement* root = doc.FirstChildElement("BossTJ");
	if (root == nullptr)
	{
		PRINT(ERROR_RED, "BossTJ.xml 文件格式错误: 找不到 BossTJ 根节点\n");
		return;
	}

	for (TiXmlElement* bossElem = root->FirstChildElement("Boss"); bossElem != nullptr; bossElem = bossElem->NextSiblingElement("Boss"))
	{
		BOSS_TJ* pBossTJ = new BOSS_TJ;
		memset(pBossTJ, 0, sizeof(BOSS_TJ));

		const char* pszName = bossElem->Attribute("name");
		if (pszName == nullptr)
		{
			delete pBossTJ;
			continue;
		}
		o_strncpy(pBossTJ->szName, pszName, 63);

		bossElem->QueryIntAttribute("cate", &pBossTJ->nCate);
		bossElem->QueryIntAttribute("enable", &pBossTJ->nEnable);

		bossElem->QueryIntAttribute("time_type", &pBossTJ->nTimeType);
		const char* pszTime = bossElem->Attribute("time");
		if (pszTime != nullptr)
		{
			char szTimeBuffer[16] = { 0 };
			if (pBossTJ->nTimeType == 0) // 时间类型0, time="11:00",转换成秒数
			{
				int h = (pBossTJ->szTime[0] - '0') * 10 + (pBossTJ->szTime[1] - '0');
				int m = (pBossTJ->szTime[3] - '0') * 10 + (pBossTJ->szTime[4] - '0');
				int nSeconds = h * 3600 + m * 60;
				_itoa(nSeconds, szTimeBuffer, 10);
				pszTime = szTimeBuffer;
			}
			else if (pBossTJ->nTimeType == 1)
			{
				pBossTJ->nTime = 0;//用于计算nTimeType=1的时间中,按次数计算
			}
			o_strncpy(pBossTJ->szTime, pszTime, 255);
		}
		
		bossElem->QueryIntAttribute("lv_recomm", &pBossTJ->nLvRecomm);
		bossElem->QueryIntAttribute("enter_lv", &pBossTJ->nEnterLv);

		const char* pszMap = bossElem->Attribute("map");
		if (pszMap != nullptr)
		{
			o_strncpy(pBossTJ->szMap, pszMap, 63);
		}

		bossElem->QueryIntAttribute("fresh_time", &pBossTJ->nFreshTime);
		bossElem->QueryIntAttribute("pic", &pBossTJ->nPic);
		bossElem->QueryIntAttribute("move_btn", &pBossTJ->nMoveBtn);

		const char* pszOpenDesc = bossElem->Attribute("open_desc");
		if (pszOpenDesc != nullptr)
		{
			o_strncpy(pBossTJ->szOpenDesc, pszOpenDesc, 255);
		}

		const char* pszDesc = bossElem->Attribute("desc");
		if (pszDesc != nullptr)
		{
			o_strncpy(pBossTJ->szDesc, pszDesc, 511);
		}

		BOSSITEM* pLastItem = nullptr;
		for (TiXmlElement* itemElem = bossElem->FirstChildElement("Item"); itemElem != nullptr; itemElem = itemElem->NextSiblingElement("Item"))
		{
			BOSSITEM* pItem = new BOSSITEM;
			memset(pItem, 0, sizeof(BOSSITEM));

			const char* pszShowName = itemElem->Attribute("show_name");
			if (pszShowName != nullptr)
			{
				o_strncpy(pItem->szShowName, pszShowName, 63);
			}

			const char* pszItemName = itemElem->Attribute("name");
			if (pszItemName != nullptr)
			{
				o_strncpy(pItem->szName, pszItemName, 63);
			}

			if (pLastItem == nullptr)
			{
				pBossTJ->pItems = pItem;
			}
			else
			{
				pLastItem->pNext = pItem;
			}
			pLastItem = pItem;
		}

		if (m_nBossTJCount < MAXBOSSTJ)
		{
			m_pBossTJList[m_nBossTJCount++] = pBossTJ;
			m_BossTJHash.HAdd(pBossTJ->szName, (LPVOID)pBossTJ);
		}
		else
		{
			delete pBossTJ;
			PRINT(ERROR_RED, "BossTJ 列表已满, 无法加载更多数据\n");
		}
	}
}

static char g_szTempBuffer[65536];
VOID CBossTJ::SendBossList(CHumanPlayer* pPlayer) const
{
	xPacket packet(g_szTempBuffer, 65535);
	const char* s1C = "LianYu18";
	packet.push(s1C);
	packet.push(1);
	int nVal1 = 1;
	packet.push((LPVOID)&nVal1, 1);

	struct TempStatus
	{
		char szName[64];
		int nCate;
	};
	std::vector<TempStatus> TempsStatus;//缓存BOSS状态
	for (int i = 0; i < m_nBossTJCount; i++) //插入列表
	{
		BOSS_TJ* pBossTJ = m_pBossTJList[i];
		if (pBossTJ != nullptr && pBossTJ->nEnable == 1)
		{
			if (pPlayer->GetPropValue(PI_LEVEL) < pBossTJ->nEnterLv)//如果玩家小于进入时间
				continue;
			TempStatus ti{};
			_snprintf(ti.szName, sizeof(ti.szName), "%s", pBossTJ->szName);
			ti.nCate = pBossTJ->nCate;
			TempsStatus.push_back(ti);
		}
	}
	int nNum = (int)TempsStatus.size();
	packet.push(&nNum, 4);
	for (const auto& ti : TempsStatus) //插入名字列表
	{
		packet.push(ti.szName);
		packet.push(1);
	}
	packet.push(&nNum, 4);
	for (const auto& ti : TempsStatus) //插入分类列表
	{
		int nCate = ti.nCate;
		packet.push(&nCate, 4);
	}
	pPlayer->SendMsg(pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CBossTJ::SendBoss(CHumanPlayer* pPlayer, const char* pszName)
{
	xPacket packet(g_szTempBuffer, 65535);
	const char* s1C = "LianYu18";
	packet.push(s1C);
	packet.push(1);
	int nVal1 = 2;
	packet.push((LPVOID)&nVal1, 1);

	BOSS_TJ* pBoss = (BOSS_TJ*)m_BossTJHash.HGet(pszName);
	if (pBoss == nullptr) return;
	BOSSITEM* pItem = pBoss->pItems;

	struct TempItem
	{
		char szName[128];
	};
	std::vector<TempItem> tempItems;
	while (pItem != nullptr)
	{
		TempItem ti{};
		_snprintf(ti.szName, sizeof(ti.szName), "%s,%s",
			pItem->szShowName, pItem->szName);
		tempItems.push_back(ti);
		pItem = pItem->pNext;
	}
	int nItemCount = (int)tempItems.size();
	packet.push((LPVOID)&nItemCount, 4);
	for (const auto& ti : tempItems)
	{
		packet.push(ti.szName);
		packet.push(1);
	}
	nVal1 = 5;
	packet.push((LPVOID)&nVal1, 4);
	packet.push((LPVOID)&pBoss->nLvRecomm, 4);
	packet.push((LPVOID)&pBoss->nFreshTime, 4);
	packet.push((LPVOID)&pBoss->nPic, 4);
	nVal1 = 1;
	packet.push((LPVOID)&nVal1, 4);
	if (pBoss->nCate == 0)
		nVal1 = 72;
	if (pBoss->nCate == 1)
		nVal1 = 368;
	packet.push((LPVOID)&nVal1, 4);
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 6;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 1;
	packet.push((LPVOID)&nVal1, 4);//固定
	packet.push(pBoss->szDesc);
	packet.push(1);
	nVal1 = 2;
	packet.push((LPVOID)&nVal1, 4);//固定
	packet.push(pBoss->szName);
	packet.push(1);
	nVal1 = 3;
	packet.push((LPVOID)&nVal1, 1);//固定
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 4;
	packet.push((LPVOID)&nVal1, 1);//固定
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 5;
	packet.push((LPVOID)&nVal1, 4);//固定
	packet.push(pBoss->szMap);
	packet.push(1);
	nVal1 = 6;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 49;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 2);//固定
	nVal1 = 5;
	packet.push((LPVOID)&nVal1, 4);//固定
	packet.push((LPVOID)&pBoss->nLvRecomm, 4);
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 4);//固定
	packet.push((LPVOID)&pBoss->nFreshTime, 4);
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 4);//固定
	packet.push((LPVOID)&pBoss->nPic, 4);
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 1;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 4);//固定
	if (pBoss->nCate == 0)
		nVal1 = 72;
	if (pBoss->nCate == 1)
		nVal1 = 368;
	packet.push((LPVOID)&nVal1, 4);//固定
	nVal1 = 0;
	packet.push((LPVOID)&nVal1, 4);//固定
	pPlayer->SendMsg(pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CBossTJ::Update()
{
	if (m_tmrUpdate.IsTimeOut(1000)) // 每秒更新
	{
		m_tmrUpdate.Savetime();
		for (int i = 0; i < m_nBossTJCount; i++) // 更新列表中BOSS刷新时间
		{
			BOSS_TJ* pBossTJ = m_pBossTJList[i];
			if (pBossTJ != nullptr && pBossTJ->nFreshTime >= 0)
			{
				pBossTJ->nFreshTime--;
				if (pBossTJ->nFreshTime <= 0)
				{
					CHumanPlayer* pAuto = CAutoScriptManager::GetInstance()->GetScriptTarget();
					pAuto->setSParam(0, pBossTJ->szName); // BOSS名字
					pAuto->setSParam(1, pBossTJ->szMap); // 地图名字
					pAuto->setSParam(2, pBossTJ->szDesc); // 简单描述
					CSystemScript::GetInstance()->Execute(pAuto->GetScriptTarget(), "BOSS图鉴.刷怪");

					if (pBossTJ->nTimeType == 0)
						pBossTJ->nFreshTime = atoi(pBossTJ->szTime);
					else if (pBossTJ->nTimeType == 1)//时间类型1, time="0,3600,7200,10800,14400"
					{
						char* Params[24]; //最大24次时间
						int nParam = SearchParam(pBossTJ->szTime, Params, 24, ',');
						if (nParam == 0) continue;
						if (pBossTJ->nTime >= nParam)//重置次数
							pBossTJ->nTime = 0;
						else
							pBossTJ->nTime++;
						BYTE nTime = pBossTJ->nTime;
						int nFreshTime = 0;
						auto SafeAtoi = [](const char* str) -> int{ return (str != nullptr) ? atoi(str) : 0; };
						if (nTime > 0 && nTime < nParam)
						{
							nFreshTime = SafeAtoi(Params[nTime]) - SafeAtoi(Params[nTime - 1]);
							if (nFreshTime < 0) nFreshTime = SafeAtoi(Params[0]);
						}
						else
							nFreshTime = SafeAtoi(Params[0]);
						pBossTJ->nFreshTime = nFreshTime;
					}
				}
			}
		}
	}
}