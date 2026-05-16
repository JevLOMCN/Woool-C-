#include "StdAfx.h"
#include ".\sandcity.h"
#include "scdoor.h"
#include "scpalacedoor.h"
#include "guildmanagerex.h"
#include "palacewall.h"
#include "scarcher.h"
#include "server.h"
#include "humanplayer.h"
#include "logicmap.h"
#include "guildex.h"
#include "LogicMapMgr.h"
#include "timesystem.h"
#include "scriptnpc.h"
#include "npcmanager.h"
#include "topmanager.h"
#include "scriptobject.h"

CSandCity::CSandCity(void) :
	m_bIdentifyStart(FALSE), m_dwCastleMapId(0), m_dwCastlePalaceDoorX(0), m_dwCastlePalaceDoorY(0),
	m_dwHomeMapId(0), m_dwHomeX(0), m_dwHomeY(0), m_dwPalaceDoorX(0), m_dwPalaceDoorY(0), m_dwPalaceMapId(0), 
	m_dwWarRangeX(0), m_dwWarRangeY(0), m_dwTodayIncome(0), m_dwTotalGold(0), m_pOwnerGuild(nullptr)
{
	m_pPalaceDoor = new CSCPalaceDoor;
	m_pMainGate = new CSCDoor;
	m_pLeftWall = new CPalaceWall;
	m_pCenterWall = new CPalaceWall;
	m_pRightWall = new CPalaceWall;
	m_szName[0] = 0;
	m_bWarStarted = FALSE;
	m_pPalaceMap = nullptr;
	memset(m_pArchers, 0, sizeof(m_pArchers));
	memset(m_AttackRequest, 0, sizeof(m_AttackRequest));
	m_iAttackRequestCount = 0;
	memset(m_pWarGuild, 0, sizeof(m_pWarGuild));
	m_iWarGuildCount = 0;
	m_pSabukMaster = nullptr;
	m_fTexRate = 0.5f;
	m_dwRebate = 80;
	m_dwTexRate = 100;
}

CSandCity::~CSandCity(void)
{
	delete m_pPalaceDoor;
	delete m_pMainGate;
	delete m_pLeftWall;
	delete m_pCenterWall;
	delete m_pRightWall;
}

CGuildEx* CSandCity::GetOwnerGuild()
{
	return m_pOwnerGuild;
}

BOOL CSandCity::Init()
{
	CSettingFile sf;
	if (!sf.Open(".\\Data\\GuildBase\\SabukW.txt"))
	{
		setError(1003, "沙城描述文件打不开!");
		return FALSE;
	}
	o_strncpy(m_szName, sf.GetString("Setup", "CastleName", "沙城"), 20);
	char* pszGuild = (char*)sf.GetString("Setup", "OwnGuild", "");
	if (*pszGuild)
	{
		m_pOwnerGuild = CGuildManagerEx::GetInstance()->FindGuild(pszGuild);
	}
	else
		m_pOwnerGuild = nullptr;
	m_ChangeTime = (char*)sf.GetString("Setup", "ChangeDate", "2005-9-25 0:0:0");
	m_WarTime = (char*)sf.GetString("Setup", "WarDate", "2005-9-25 0:0:0");
	m_IncomeTime = (char*)sf.GetString("Setup", "IncomeToday", "2005-9-25 0:0:0");

	this->m_dwTodayIncome = (DWORD)sf.GetInteger("Setup", "TodayIncome", 0);
	this->m_dwTotalGold = (DWORD)sf.GetInteger("Setup", "TotalGold", 0);

	DWORD dwTexRate = sf.GetInteger("Setup", "TexRate", 50);
	SetRebate(sf.GetInteger("Setup", "Rebate", 70));
	SetTexRate(dwTexRate);

	m_dwHomeX = sf.GetInteger("Defense", "CastleHomeX", 224);
	m_dwHomeY = sf.GetInteger("Defense", "CastleHomeY", 436);
	m_dwHomeMapId = sf.GetInteger("Defense", "CastleHomeMap", 116);
	m_dwCastleMapId = sf.GetInteger("Defense", "CastleMap", 116);
	m_dwWarRangeX = sf.GetInteger("Defense", "CastleWarRangeX", 100);
	m_dwWarRangeY = sf.GetInteger("Defense", "CastleWarRangeY", 100);

	m_dwPalaceMapId = sf.GetInteger("Defense", "PalaceMap", 123);
	m_dwPalaceDoorX = sf.GetInteger("Defense", "PalaceDoorX", 18);
	m_dwPalaceDoorY = sf.GetInteger("Defense", "PalaceDoorY", 25);
	m_dwCastlePalaceDoorX = sf.GetInteger("Defense", "CastlePalaceDoorX", 167);
	m_dwCastlePalaceDoorY = sf.GetInteger("Defense", "CastlePalaceDoorY", 370);

	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(m_dwCastleMapId);
	if (pMap != nullptr)
		pMap->SetMapEventFlagRect(m_dwHomeX, m_dwHomeY, m_dwWarRangeX, m_dwWarRangeY, EVENTFLAG_WAREVENT);
	pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(m_dwPalaceMapId);
	if (pMap != nullptr)
		pMap->SetMapEventFlagRect(10, 10, 100, 100, EVENTFLAG_WAREVENT);

	int mapid = m_dwCastleMapId;
	int x, y;
	x = sf.GetInteger("Defense", "CenterWallX", 164);
	y = sf.GetInteger("Defense", "CenterWallY", 372);
	m_pCenterWall->Init(sf.GetString("Defense", "CenterWallName", "皇宫墙中"), mapid, x, y, sf.GetInteger("Defense", "CenterWallHP", 5000));
	x = sf.GetInteger("Defense", "LeftWallX", 158);
	y = sf.GetInteger("Defense", "LeftWallY", 371);
	m_pLeftWall->Init(sf.GetString("Defense", "LeftWallName", "皇宫墙左"), mapid, x, y, sf.GetInteger("Defense", "LeftWallHP", 5000));
	x = sf.GetInteger("Defense", "RightWallX", 170);
	y = sf.GetInteger("Defense", "RightWallY", 366);
	m_pRightWall->Init(sf.GetString("Defense", "RightWallName", "皇宫墙右"), mapid, x, y, sf.GetInteger("Defense", "RightWallHP", 5000));
	x = sf.GetInteger("Defense", "MainDoorX", 206);
	y = sf.GetInteger("Defense", "MainDoorY", 416);
	m_pMainGate->Init(sf.GetString("Defense", "MainDoorName", "沙城大门"), mapid, x, y, sf.GetInteger("Defense", "MainDoorHP", 10000), sf.GetInteger("Defense", "MainDoorOpen", 1));

	int iArcherCount = 0;
	char szValueName[128];
	char* pszName = nullptr;
	for (int i = 0; i < 12; i++)
	{
		sprintf(szValueName, "Archer_%d_X", i + 1);
		x = sf.GetInteger("Defense", szValueName, -1);
		sprintf(szValueName, "Archer_%d_Y", i + 1);
		y = sf.GetInteger("Defense", szValueName, -1);
		sprintf(szValueName, "Archer_%d_Name", i + 1);
		pszName = (char*)sf.GetString("Defense", szValueName, nullptr);

		if (x == -1 || y == -1 || pszName == nullptr)continue;

		m_pArchers[iArcherCount] = new CSCArcher;
		sprintf(szValueName, "Archer_%d_Hp", i + 1);
		if (!m_pArchers[iArcherCount]->Init(pszName, mapid, x, y, sf.GetInteger("Defense", szValueName, 0)))
		{
			delete m_pArchers[iArcherCount];
			m_pArchers[iArcherCount] = nullptr;
		}
		else
			iArcherCount++;
	}

	m_pCenterWall->SetSystemFlag(SF_NODAMAGE, TRUE);
	m_pLeftWall->SetSystemFlag(SF_NODAMAGE, TRUE);
	m_pRightWall->SetSystemFlag(SF_NODAMAGE, TRUE);
	m_pMainGate->SetSystemFlag(SF_NODAMAGE, TRUE);

	//	皇宫入口门点
	x = sf.GetInteger("Defense", "CastlePalaceDoorX", 416);
	y = sf.GetInteger("Defense", "CastlePalaceDoorY", 416);
	int PalaceMap = sf.GetInteger("Defense", "PalaceMap", 123);
	int PalaceDoorX = sf.GetInteger("Defense", "PalaceDoorX", 20);
	int PalaceDoorY = sf.GetInteger("Defense", "PalaceDoorY", 20);

	if (!m_pPalaceDoor->Create(mapid, x, y, PalaceMap, PalaceDoorX, PalaceDoorY))
		PRINT(ERROR_RED, "沙城皇宫入口门点创建失败!\n");
	m_pPalaceMap = CLogicMapMgr::GetInstance()->GetLogicMapById(PalaceMap);
	if (m_pPalaceMap)
		m_pPalaceMap->SetFlag("SABUKPALACE");
	else
		PRINT(ERROR_RED, "沙城皇宫设置失败, 沙城功能可能无法全部启用!\n");

	m_bWarStarted = FALSE;

	char* pSabukMasterFigure = (char*)sf.GetString("Setup", "MasterFigure", "");
	if (pSabukMasterFigure[0] != 0)
	{
		this->m_pSabukMaster = CNpcManager::GetInstance()->AddNpc(pSabukMasterFigure);
		if (m_pSabukMaster)
		{
			char* pSabukMaster = (char*)sf.GetString("Setup", "TopSabukMaster", "");
			if (pSabukMaster[0] != 0)
			{
				xStringsExtracter<20> top(pSabukMaster);
				if (top.getCount() == 7)
				{
					m_SabukMaster.btClass = (BYTE)StringToInteger(top[2]);
					m_SabukMaster.btSex = (BYTE)StringToInteger(top[3]);
					m_SabukMaster.dwDBId = (DWORD)StringToInteger(top[1]);
					m_SabukMaster.btPerCent = (DWORD)StringToInteger(top[5]);
					o_strncpy(m_SabukMaster.szName, top[0], 16);
					m_SabukMaster.wLevel = (WORD)StringToInteger(top[4]);
					GetTimeFromString(m_SabukMaster.stRanking, top[6]);
					//m_SabukMaster
					if (m_SabukMaster.szName[0] != 0 && m_pOwnerGuild != nullptr)
					{
						char szName[128];
						sprintf(szName, "%s\\%s\\%s\\", m_pSabukMaster->GetName(), m_pOwnerGuild->GetName(), m_SabukMaster.szName);
						m_pSabukMaster->SetLongName(szName);
						m_pSabukMaster->SendChangeName();
						int index = m_SabukMaster.btClass * 2 + m_SabukMaster.btSex;
						m_pSabukMaster->SetView(CTopManager::GetInstance()->GetTopView(index));
						m_pSabukMaster->SendFeatureChanged();
					}
				}
			}
		}
	}
	sf.Close();
	LoadAttackRequest();
	CTimeSystem::GetInstance()->RegisterTimeEvent(this);
	return TRUE;
}

BOOL CSandCity::StartWar()
{
	if (m_iWarGuildCount <= 0)
		this->PrepareAttackGuild(CSystemTime());
	if (m_iWarGuildCount <= 0)
	{
		setError(0x9100, "没有攻城行会!");
		return FALSE;
	}
	m_pCenterWall->SetSystemFlag(SF_NODAMAGE, FALSE);
	m_pLeftWall->SetSystemFlag(SF_NODAMAGE, FALSE);
	m_pRightWall->SetSystemFlag(SF_NODAMAGE, FALSE);
	m_pMainGate->SetSystemFlag(SF_NODAMAGE, FALSE);
	char szText[1024];
	if (m_pOwnerGuild)
		sprintf(szText, "攻城战开始, 守城方为 %s!", m_pOwnerGuild->GetName());
	else
		sprintf(szText, "攻城战开始, 没有守城方!");

	CGameWorld::GetInstance()->PostSystemMessage(szText);

	m_bWarStarted = TRUE;
	m_bIdentifyStart = FALSE;

	for (int i = 0; i < m_iWarGuildCount; i++)
	{
		if (m_pWarGuild[i])
		{
			m_pWarGuild[i]->SetAttackSabuk(TRUE);
			sprintf(szText, "%s 的攻城战斗开始!", m_pWarGuild[i]->GetName());
			CGameWorld::GetInstance()->PostSystemMessage(szText, i * 500 + 1000);
		}
	}
	//隐藏NPC
	CGameWorld::GetInstance()->HideSandCityNpc();
	CGameWorld::GetInstance()->AddGlobeProcess(EP_WARSTART);
	m_tmrWar.Savetime();
	return TRUE;
}

BOOL CSandCity::EndWar()
{
	m_pCenterWall->SetSystemFlag(SF_NODAMAGE, TRUE);
	m_pLeftWall->SetSystemFlag(SF_NODAMAGE, TRUE);
	m_pRightWall->SetSystemFlag(SF_NODAMAGE, TRUE);
	m_pMainGate->SetSystemFlag(SF_NODAMAGE, TRUE);
	CGameWorld::GetInstance()->PostSystemMessage("攻城战结束!");
	m_bWarStarted = FALSE;
	m_bIdentifyStart = FALSE;

	for (int i = 0; i < m_iWarGuildCount; i++)
	{
		if (m_pWarGuild[i])
			m_pWarGuild[i]->SetAttackSabuk(FALSE);
	}
	m_iWarGuildCount = 0;
	//显示NPC
	CGameWorld::GetInstance()->ShowSandCityNpc();
	CGameWorld::GetInstance()->AddGlobeProcess(EP_WAREND);
	return TRUE;
}

VOID CSandCity::ProcIdentify()
{
	CGuildEx* pGuild = nullptr;
	BOOL bIdentStart = FALSE;
	if (m_pPalaceMap != nullptr)
	{
		xListHelper<CMapObject> list;
		m_pPalaceMap->GetObjList(list);
		for (CMapObject* pObject = list.first(); pObject != nullptr; pObject = list.next())
		{
			if (pObject->GetType() == OBJ_PLAYER)
			{
				CHumanPlayer* pPlayer = (CHumanPlayer*)pObject;
				if (pPlayer->IsDeath())
					continue;
				CGuildEx* pTGuild = pPlayer->GetGuild();
				//	有没有门派的人在
				if (pTGuild == nullptr)
				{
					StopIdentify();
					return;
				}
				//	有沙城成员在
				if (pTGuild == m_pOwnerGuild)
				{
					StopIdentify();
					return;
				}
				if (pGuild == nullptr)
					pGuild = pTGuild;
				else if (pGuild != pTGuild)
				{
					StopIdentify();
					return;
				}
				//	如果不是攻城者, 就停止认证
				if (!pGuild->IsAttackSabuk())
				{
					StopIdentify();
					return;
				}
				//	接下来, 从里面找会长
				if (pGuild->IsMaster(pPlayer)) bIdentStart = TRUE;
			}
		}
		if (bIdentStart) StartIdentify();
	}
}

VOID CSandCity::OnLeavePalace(CHumanPlayer* pPlayer)
{
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (m_bIdentifyStart)
	{
		//	如果开始验证, 并且离开的人不是老大, 就不需要再次验证
		if (pGuild && !pGuild->IsMaster(pPlayer)) return;
	}
	ProcIdentify();
}

VOID CSandCity::OnEnterPalace(CHumanPlayer* pPlayer)
{
	//	Search Palace Map
	CGuildEx* pGuild = pPlayer->GetGuild();
	//	进来的人是没有行会的, 或者是沙城的人
	if (pGuild == nullptr || pGuild == m_pOwnerGuild)
	{
		StopIdentify();
		return;
	}
	//	如果没有开始验证
	if (!m_bIdentifyStart)
	{
		//	并且进来的不是行会老大, 就不测试是否开始验证
		if (!pGuild->IsMaster(pPlayer)) return;
	}
	ProcIdentify();
}

VOID CSandCity::StartIdentify()
{
	if (m_bIdentifyStart)return;
	m_bIdentifyStart = TRUE;
	m_tmrIdentify.Savetime();
}

VOID CSandCity::StopIdentify()
{
	m_bIdentifyStart = FALSE;
}

BOOL CSandCity::IdentifyEnd()
{
	if (m_pPalaceMap == nullptr)return FALSE;
	BOOL bIdentSucc = FALSE;
	xListHelper<CMapObject> list;
	m_pPalaceMap->GetObjList(list);
	CGuildEx* pGuild = nullptr;

	for (CMapObject* pObject = list.first(); pObject != nullptr; pObject = list.next())
	{
		if (pObject->GetType() == OBJ_PLAYER)
		{
			CHumanPlayer* pPlayer = (CHumanPlayer*)pObject;
			if (pPlayer->IsDeath())
				continue;
			CGuildEx* pTGuild = pPlayer->GetGuild();
			if (pTGuild == nullptr)
			{
				StopIdentify();
				return FALSE;
			}
			if (pTGuild == m_pOwnerGuild)
			{
				StopIdentify();
				return FALSE;
			}
			if (pGuild == nullptr)
				pGuild = pTGuild;
			else if (pGuild != pTGuild)
			{
				StopIdentify();
				return FALSE;
			}
			//	接下来, 从里面找会长
			if (pGuild->IsMaster(pPlayer))
				bIdentSucc = TRUE;
		}
	}
	if (pGuild && bIdentSucc)
		ChangeOwner(pGuild); // 换主人～～
	return bIdentSucc; // 夺城成功～
}

VOID CSandCity::ChangeOwner(CGuildEx* pOwner)
{
	CGuildEx* pOldOwner = m_pOwnerGuild;
	m_pOwnerGuild = pOwner;
	if (pOldOwner)
	{
		pOldOwner->SetAttackSabuk(TRUE);
		pOldOwner->RefreshMemberName();
	}
	if (pOwner)
	{
		char szText[1024];
		sprintf(szText, "沙城被 %s 行会取得!", pOwner->GetName());
		CGameWorld::GetInstance()->PostSystemMessage(szText);
		pOwner->SetAttackSabuk(FALSE);
		pOwner->RefreshMemberName();
	}
	m_ChangeTime = CSystemTime();
	memset(&m_SabukMaster, 0, sizeof(m_SabukMaster));
	UpdateSabukMasterFigure();
	Save();
}

VOID CSandCity::UpdateWar()
{
	if (IsWarStarted())
	{
		if (m_bIdentifyStart)
		{
			if (m_tmrIdentify.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_SANDCITYTAKETIME) * 1000))
			{
				if (IdentifyEnd()) // 检测是否要结束战争
				{
					StopIdentify(); //	夺城成功
					return;
				}
				else
					StopIdentify();
			}
		}
		if (m_tmrWar.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_WARTIMELONG) * 60000))
			EndWar();
	}
}

VOID CSandCity::OpenGate()
{
	this->m_pMainGate->Open();
}

VOID CSandCity::CloseGate()
{
	this->m_pMainGate->Close();
}

VOID CSandCity::RepairGate()
{
	this->m_pMainGate->Repair();
}

VOID CSandCity::RepairWall(int index)
{
	switch (index)
	{
	case 1:
		m_pLeftWall->Repair();
		break;
	case 2:
		m_pCenterWall->Repair();
		break;
	case 3:
		m_pRightWall->Repair();
		break;
	}
}

VOID CSandCity::Save()
{
	FILE* fp = fopen(".\\Data\\GuildBase\\SabukW.txt", "w");
	if (fp == nullptr)
	{
		PRINT(ERROR_RED, "无法打开 SabukW.txt, 沙城存储失败~!\n");
		return;
	}
	fprintf(fp, "[Setup]\nCastleName=%s\nOwnGuild=%s\nChangeDate=\"%d-%d-%d %d:%02d:%02d\"\n"
		"WarDate=\"%d-%d-%d %d:%02d:%02d\"\nIncomeToday=\"%d-%d-%d %d:%02d:%02d\"\n"
		"TotalGold=%u\nTodayIncome=%u\nTexRate=%u\nRebate=%u\n\n", m_szName, m_pOwnerGuild == nullptr ? "" : m_pOwnerGuild->GetName(),
		this->m_ChangeTime.GetYear(), this->m_ChangeTime.GetMonth(), this->m_ChangeTime.GetDay(),
		this->m_ChangeTime.GetHour(), this->m_ChangeTime.GetMinute(), this->m_ChangeTime.GetSecond(),
		this->m_WarTime.GetYear(), this->m_WarTime.GetMonth(), this->m_WarTime.GetDay(),
		this->m_WarTime.GetHour(), this->m_WarTime.GetMinute(), this->m_WarTime.GetSecond(),
		this->m_IncomeTime.GetYear(), this->m_IncomeTime.GetMonth(), this->m_IncomeTime.GetDay(),
		this->m_IncomeTime.GetHour(), this->m_IncomeTime.GetMinute(), this->m_IncomeTime.GetSecond(),
		m_dwTotalGold, m_dwTodayIncome, GetTexRate(), GetRebate());

	if (m_pSabukMaster != nullptr)
	{
		fprintf(fp, "MasterFigure=%s/1/%d/%d/%d/%d/1/%s\n",
			m_pSabukMaster->GetName(), m_pSabukMaster->GetView(), m_pSabukMaster->GetMapId(), m_pSabukMaster->getX(),
			m_pSabukMaster->getY(), m_pSabukMaster->getScriptObject() != nullptr ? m_pSabukMaster->getScriptObject()->getName() : "沙城霸主");
		if (m_SabukMaster.szName[0] != 0)
		{
			fprintf(fp, "TopSabukMaster=%s/%u/%u/%u/%u/%u/%04u-%02u-%02u\n",
				m_SabukMaster.szName, m_SabukMaster.dwDBId, m_SabukMaster.btClass, m_SabukMaster.btSex,
				m_SabukMaster.wLevel, m_SabukMaster.btPerCent, m_SabukMaster.stRanking.wYear,
				m_SabukMaster.stRanking.wMonth, m_SabukMaster.stRanking.wDay);
		}
	}

	fprintf(fp, "[Defense]\n");
	fprintf(fp, "CastleMap=%u\nCastleHomeMap=%u\nCastleHomeX=%u\nCastleHomeY=%u\nCastleWarRangeX=%u\nCastleWarRangeY=%u\n\n",
		m_dwCastleMapId, m_dwHomeMapId, m_dwHomeX, m_dwHomeY, m_dwWarRangeX, m_dwWarRangeY);
	fprintf(fp, "PalaceMap=%u\nPalaceDoorX=%u\nPalaceDoorY=%u\nCastlePalaceDoorX=%u\nCastlePalaceDoorY=%u\n\n",
		m_dwPalaceMapId, m_dwPalaceDoorX, m_dwPalaceDoorY, m_dwCastlePalaceDoorX, m_dwCastlePalaceDoorY);
	fprintf(fp, "MainDoorName=%s\nMainDoorX=%u\nMainDoorY=%u\nMainDoorOpen=%u\nMainDoorHP=%u\n\n",
		m_pMainGate->GetDesc() == nullptr ? "沙城大门" : m_pMainGate->GetDesc()->base.szClassName,
		m_pMainGate->getX(), m_pMainGate->getY(), m_pMainGate->IsOpened(), m_pMainGate->GetPropValue(PI_CURHP));
	fprintf(fp, "LeftWallName=%s\nLeftWallX=%u\nLeftWallY=%u\nLeftWallHP=%u\n\n",
		m_pLeftWall->GetDesc() == nullptr ? "皇宫墙左" : m_pLeftWall->GetDesc()->base.szClassName,
		m_pLeftWall->getX(), m_pLeftWall->getY(), m_pLeftWall->GetPropValue(PI_CURHP));
	fprintf(fp, "CenterWallName=%s\nCenterWallX=%u\nCenterWallY=%u\nCenterWallHP=%u\n\n",
		m_pCenterWall->GetDesc() == nullptr ? "皇宫墙中" : m_pCenterWall->GetDesc()->base.szClassName,
		m_pCenterWall->getX(), m_pCenterWall->getY(), m_pCenterWall->GetPropValue(PI_CURHP));
	fprintf(fp, "RightWallName=%s\nRightWallX=%u\nRightWallY=%u\nRightWallHP=%u\n\n",
		m_pRightWall->GetDesc() == nullptr ? "皇宫墙右" : m_pRightWall->GetDesc()->base.szClassName,
		m_pRightWall->getX(), m_pRightWall->getY(), m_pRightWall->GetPropValue(PI_CURHP));

	for (int i = 0; i < 12; i++)
	{
		if (m_pArchers[i] == nullptr)continue;
		fprintf(fp, "Archer_%u_Name=%s\nArcher_%u_X=%u\nArcher_%u_Y=%u\nArcher_%u_HP=%u\n\n",
			i + 1, m_pArchers[i]->GetDesc() == nullptr ? "Archer" : m_pArchers[i]->GetDesc()->base.szClassName,
			i + 1, m_pArchers[i]->getX(), i + 1, m_pArchers[i]->getY(), i + 1, m_pArchers[i]->GetPropValue(PI_CURHP));
	}
	fclose(fp);
}

VOID CSandCity::Home(CHumanPlayer* pPlayer)
{
	if (pPlayer != nullptr)
		pPlayer->FlyTo(m_dwHomeMapId, m_dwHomeX, m_dwHomeY);
}

BOOL CSandCity::AddAttackRequest(CGuildEx* pGuild, BOOL bToday)
{
	if (m_iAttackRequestCount >= MAX_ATTACKREQUEST)
	{
		setError(0x9114, "达到最大的攻城申请数目, 无法继续申请～!");
		return FALSE;
	}
	if (pGuild == nullptr)
	{
		setError(0x9115, "您还没有加入行会!");
		return FALSE;
	}

	if (pGuild == m_pOwnerGuild)
	{
		setError(0x9116, "不能攻击自己的城堡!");
		return FALSE;
	}

	char* pszGuild = (char*)pGuild->GetName();
	for (int i = 0; i < this->m_iAttackRequestCount; i++)
	{
		if (this->m_AttackRequest[i].pGuild == pGuild)
		{
			setError(0x9117, "您已经申请过攻城战了!");
			return FALSE;
		}
	}
	o_strncpy(m_AttackRequest[m_iAttackRequestCount].guild, pszGuild, 31);
	GetLocalTime(&m_AttackRequest[m_iAttackRequestCount].stTime);

	if (!bToday)stPlusDay(m_AttackRequest[m_iAttackRequestCount].stTime, 2); //调整到2天后
	m_AttackRequest[m_iAttackRequestCount].stTime.wHour = 20; //调整到八点
	m_AttackRequest[m_iAttackRequestCount].stTime.wMinute = 0;
	m_AttackRequest[m_iAttackRequestCount].stTime.wSecond = 0;
	m_AttackRequest[m_iAttackRequestCount].pGuild = pGuild;
	m_iAttackRequestCount++;
	SaveAttackRequest();
	return TRUE;
}

static AttackSabukRequest g_AttackRequestT[100];
VOID CSandCity::PrepareAttackGuild(CSystemTime& curTime)
{
	int count = 0;
	this->m_iWarGuildCount = 0;
	for (int i = 0; i < this->m_iAttackRequestCount; i++)
	{
		if (curTime.GetYear() == m_AttackRequest[i].stTime.wYear &&
			curTime.GetMonth() == m_AttackRequest[i].stTime.wMonth &&
			curTime.GetDay() == m_AttackRequest[i].stTime.wDay)
		{
			//iWarCount++;
			m_pWarGuild[m_iWarGuildCount++] = m_AttackRequest[i].pGuild;
			//m_pWarGuild[m_iWarGuildCount++]->SetAttackSabuk( TRUE );
		}
		else
			g_AttackRequestT[count++] = m_AttackRequest[i];
	}
	if (count > 0)
		memcpy(m_AttackRequest, g_AttackRequestT, count * sizeof(AttackSabukRequest));
	m_iAttackRequestCount = count;
	SaveAttackRequest();
}

VOID CSandCity::OnHourChange(CSystemTime& curTime)
{
	if (curTime.GetHour() == CGameWorld::GetInstance()->GetVar(EVI_WARSTARTTIME))
	{
		if (IsWarStarted()) return;
		//	找出所有的今天的攻城申请, 
		PrepareAttackGuild(curTime);
		if (m_iWarGuildCount > 0) StartWar();
	}
}

VOID CSandCity::SaveAttackRequest()
{
	FILE* fp = fopen(".\\Data\\GuildBase\\Attackreq.txt", "w");
	if (fp == nullptr)
	{
		setError(0x1000, "无法存储攻城请求!");
		return;
	}
	for (int i = 0; i < m_iAttackRequestCount; i++)
	{
		fprintf(fp, "%s|\"%u-%u-%u\"\n", m_AttackRequest[i].pGuild->GetName(), m_AttackRequest[i].stTime.wYear,
			m_AttackRequest[i].stTime.wMonth, m_AttackRequest[i].stTime.wDay);
	}
	fclose(fp);
}

VOID CSandCity::LoadAttackRequest()
{
	CStringFile sf(".\\Data\\GuildBase\\Attackreq.txt");
	sf.MakeDeflate();
	this->m_iAttackRequestCount = 0;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		if (*sf[i] == '#')continue;
		xStringsExpander<20> str(sf[i], '|');
		if (str.getCount() != 2)continue;
		CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(str[0]);
		if (pGuild)
		{
			int len = (int)strlen(str[1]);
			if (len <= 0)
				GetLocalTime(&m_AttackRequest[m_iAttackRequestCount].stTime);
			else
			{
				char* p = (char*)str[1];
				if (*(p + len - 1) == '\"')
					*(p + len - 1) = 0;
				if (*p == '\"')p++;
				GetTimeFromString(m_AttackRequest[m_iAttackRequestCount].stTime, p);
			}
			m_AttackRequest[m_iAttackRequestCount].pGuild = pGuild;
			o_strncpy(m_AttackRequest[m_iAttackRequestCount].guild, str[0], 20);
			m_iAttackRequestCount++;
		}
	}
}

#define PAGEREQUESTCOUNT 8
int CSandCity::PrePareAttackRequestPage(UINT nPage, char* pBuffer)
{
	//	每个页面显示
	if (m_iAttackRequestCount <= 0)
	{
		strcpy(pBuffer, "没有攻城请求\\\\\\<知道了/@exit>\\");
		return (int)strlen(pBuffer);
	}
	UINT nLastPage = (m_iAttackRequestCount + PAGEREQUESTCOUNT - 1) / PAGEREQUESTCOUNT;
	if (nPage > nLastPage)nPage = nLastPage;

	int start = nPage * PAGEREQUESTCOUNT;
	int end = nPage * PAGEREQUESTCOUNT + PAGEREQUESTCOUNT;
	end = end > m_iAttackRequestCount ? m_iAttackRequestCount : end;
	char szLine[240];
	pBuffer[0] = 0;
	for (int i = start; i < end; i++)
	{
		sprintf(szLine, "%u年-%u月-%u日 %u:%u:%u\"%s\"\\", 
			m_AttackRequest[i].stTime.wYear, m_AttackRequest[i].stTime.wMonth, m_AttackRequest[i].stTime.wDay, 
			m_AttackRequest[i].stTime.wHour, m_AttackRequest[i].stTime.wMinute, m_AttackRequest[i].stTime.wSecond,
			m_AttackRequest[i].guild);
		strcat(pBuffer, szLine);
	}
	if (nPage != 0)
	{
		sprintf(szLine, "<上一页/@AttackRequestPage%u>  ", nPage - 1);
		strcat(pBuffer, szLine);
	}
	if (end != m_iAttackRequestCount)
	{
		sprintf(szLine, "<下一页/@AttackRequestPage%u>  ", nPage + 1);
		strcat(pBuffer, szLine);
	}
	strcat(pBuffer, "\\<知道了/@exit>\\");
	return (int)strlen(pBuffer);
}

BOOL CSandCity::SetSabukMaster(CHumanPlayer* m_pPlayer)
{
	if (m_pOwnerGuild == nullptr)return FALSE;
	if (m_pSabukMaster == nullptr)return FALSE;
	if (!m_pOwnerGuild->IsFirstMaster(m_pPlayer))return FALSE;
	m_SabukMaster.btClass = m_pPlayer->GetPro();
	m_SabukMaster.btSex = m_pPlayer->GetSex();
	m_SabukMaster.dwDBId = m_pPlayer->GetDBId();
	m_SabukMaster.btPerCent = m_pPlayer->GetPropValue(PI_EXPPERCENT);
	GetLocalTime(&m_SabukMaster.stRanking);
	m_SabukMaster.wLevel = m_pPlayer->GetPropValue(PI_LEVEL);
	o_strncpy(m_SabukMaster.szName, m_pPlayer->GetName(), 16);
	if (m_SabukMaster.szName[0] != 0)
	{
		char szName[128];
		sprintf(szName, "%s\\%s\\%s\\", m_pSabukMaster->GetName(), m_pOwnerGuild->GetName(), m_SabukMaster.szName);
		m_pSabukMaster->SetLongName(szName);
		m_pSabukMaster->SendChangeName();
		int index = m_SabukMaster.btClass * 2 + m_SabukMaster.btSex;
		m_pSabukMaster->SetView(CTopManager::GetInstance()->GetTopView(index));
		m_pSabukMaster->SendFeatureChanged();
		this->Save();
	}
	return TRUE;
}

VOID CSandCity::UpdateSabukMasterFigure()
{
	if (m_pSabukMaster == nullptr)return;
	if (m_SabukMaster.szName[0] != 0)
	{
		char szName[128];
		sprintf(szName, "%s\\%s\\%s\\", m_pSabukMaster->GetName(), m_pOwnerGuild->GetName(), m_SabukMaster.szName);
		m_pSabukMaster->SetLongName(szName);
		m_pSabukMaster->SendChangeName();
		int index = m_SabukMaster.btClass * 2 + m_SabukMaster.btSex;
		m_pSabukMaster->SetView(CTopManager::GetInstance()->GetTopView(index));
		m_pSabukMaster->SendFeatureChanged();
	}
	else
	{
		m_pSabukMaster->SetLongName(m_pSabukMaster->GetName());
		m_pSabukMaster->SendChangeName();
	}
}

BOOL CSandCity::AddIncoming(DWORD dwIncoming)
{
	dwIncoming *= this->m_fTexRate;
	if (!AddTotalGold(dwIncoming))return FALSE;
	this->m_dwTodayIncome += dwIncoming;
	CSystemTime t;
	if (t.GetDay() != this->m_IncomeTime.GetDay())
		this->m_dwTodayIncome = 0;
	this->m_IncomeTime = t;
	return TRUE;
}

BOOL CSandCity::AddTotalGold(DWORD dwAddGold)
{
	if (((DWORD)-1 - this->m_dwTotalGold) < dwAddGold)return FALSE;
	this->m_dwTotalGold += dwAddGold;
	return TRUE;
}

BOOL CSandCity::DecTotalGold(DWORD dwDecGold)
{
	if (this->m_dwTotalGold < dwDecGold)return FALSE;
	this->m_dwTotalGold -= dwDecGold;
	return TRUE;
}