#include "StdAfx.h"
#include <vector>
#include "server.h"
#include ".\monstermanagerex.h"
#include "monsterex.h"
#include "monitemsmgr.h"
#include "gameworld.h"

CMonsterManagerEx::CMonsterManagerEx(void)
{
	m_xMonsterList.create(100000);
	m_xDeleteQueue.create(50000);
	m_pActiveMonster = nullptr;
	m_nCurFreeIndex = 0;
}

CMonsterManagerEx::~CMonsterManagerEx(void)
{
	m_xMonsterList.destroy();
	m_xDeleteQueue.destroy();
}

VOID CMonsterManagerEx::ClearMonsterData()
{
	xListHost<MonsterClass>::xListNode* pNode = m_xMonsterClassPool.GetUsedHeadNode();
	while (pNode != nullptr)
	{
		MonsterClass* pClass = pNode->getObject();
		if (pClass != nullptr)
		{
			if (pClass->pszBornScript != nullptr)
			{
				delete[] pClass->pszBornScript;
				pClass->pszBornScript = nullptr;
			}
			if (pClass->pszGotTargetScript != nullptr)
			{
				delete[] pClass->pszGotTargetScript;
				pClass->pszGotTargetScript = nullptr;
			}
			if (pClass->pszKillTargetScript != nullptr)
			{
				delete[] pClass->pszKillTargetScript;
				pClass->pszKillTargetScript = nullptr;
			}
			if (pClass->pszHurtScript != nullptr)
			{
				delete[] pClass->pszHurtScript;
				pClass->pszHurtScript = nullptr;
			}
			if (pClass->pszDeathScript != nullptr)
			{
				delete[] pClass->pszDeathScript;
				pClass->pszDeathScript = nullptr;
			}
		}
		pNode = pNode->getNext();
	}

	m_MonsterClassHash.~CNameHash();
	new (&m_MonsterClassHash) CNameHash();
}

CMonsterEx* CMonsterManagerEx::CreateMonster(MonsterClass* pClass, int mapid, int x, int y, MONSTERGEN* pGen)
{
	if (pClass == nullptr) return nullptr;
	CMonsterEx* pMonster = newObject();//创一个新的怪物, 并设置怪物ID
	if (pMonster == nullptr) return nullptr;
	if (!pMonster->Init(pClass, mapid, x, y, pGen))//如果不能初始化怪物
	{
		deleteObject(pMonster);
		return nullptr;
	}

	if (pGen)
		pGen->curcount++;
	pClass->nCount++;
	return pMonster;
}

CMonsterEx* CMonsterManagerEx::CreateMonster(const char* pszName, int mapid, int x, int y, MONSTERGEN* pGen)
{
	MonsterClass* pClass = GetClassByName(pszName);//通过名字获取哈希表中怪物结构体
	return CreateMonster(pClass, mapid, x, y, pGen);
}

BOOL CMonsterManagerEx::DeleteMonster(CMonsterEx* pMonster)
{
	pMonster->ClearGen();
	if (!m_xDeleteQueue.push(pMonster))
	{
		return DeleteMonsterImm(pMonster);
	}
	pMonster->SetDelTimer();
	return TRUE;
}

BOOL CMonsterManagerEx::AddExtraMonster(CMonsterEx* pMonster)
{
	UINT id = m_xMonsterList.addObject(pMonster);
	if (id == 0)return FALSE;
	id |= (OBJ_MONSTER << 24);
	pMonster->SetId(id);
	return TRUE;
}

BOOL CMonsterManagerEx::DeleteMonsterImm(CMonsterEx* pMonster)
{
	pMonster->ClearGen();
	deleteObject(pMonster);
	return TRUE;
}

VOID CMonsterManagerEx::UpdateDeleteMonster()
{
	const UINT MAX_PROCESS_PER_TICK = 100;
	UINT processed = 0;
	std::vector<CMonsterEx*> pendingObjects;
	pendingObjects.reserve(MAX_PROCESS_PER_TICK);
	while (processed < MAX_PROCESS_PER_TICK)
	{
		CMonsterEx* pMonster = m_xDeleteQueue.pop();
		if (pMonster == nullptr)
			break;
		if (pMonster->IsDelTimerTimeOut(1000))
			DeleteMonsterImm(pMonster);
		else
			pendingObjects.push_back(pMonster);
		processed++;
	}
	for (auto pMonster : pendingObjects)
	{
		if (m_xDeleteQueue.getcount() < 45000)
			m_xDeleteQueue.push(pMonster);
		else
			DeleteMonsterImm(pMonster);
	}
}

CMonsterEx* CMonsterManagerEx::newObject()//创建一个怪物OBJECT, 并设置ID
{
	CMonsterEx* pMonster = m_xMonsterPool.newObject();
	if (pMonster == nullptr)
		return nullptr;

	UINT id = m_xMonsterList.addObject(pMonster);

	if (id == 0)
	{
		m_xMonsterPool.deleteObject(pMonster);
		pMonster = nullptr;
	}
	else
	{
		id |= (OBJ_MONSTER << 24);
		pMonster->SetId(id);
	}
	return pMonster;
}

VOID CMonsterManagerEx::deleteObject(CMonsterEx* pObject)
{
	if (pObject == nullptr)return;
	m_xMonsterList.delObject(pObject->GetId() & 0xffffff);
	pObject->Clean();
	m_xMonsterPool.deleteObject(pObject);
}

BOOL CMonsterManagerEx::LoadMonsters(const char* pszPath)
{
	return StartFind(pszPath, "*.txt", TRUE);
}

VOID CMonsterManagerEx::OnFoundFile(const char* pszFilename, UINT nParam1)
{
	CStringFile sfMonster(pszFilename);
	MonsterClass* pClass = nullptr;
	sfMonster.MakeDeflate();
	for (int i = 0; i < sfMonster.GetLineCount(); i++)
	{
		if (*sfMonster[i] == '@')
		{
			if (pClass != nullptr)
			{
				if (pClass->base.szClassName[0] != 0)
				{
					MonsterClass* pClassOld = (MonsterClass*)m_MonsterClassHash.HGet(pClass->base.szClassName);
					if (pClassOld != nullptr)
					{
						if (memcmp(pClass, pClassOld, sizeof(MonsterClass)) != 0)
						{
							memcpy(pClassOld, pClass, sizeof(MonsterClass));
							PRINT(CYAN, "怪物 %s 被更新\n", pClass->base.szClassName);
						}
					}
					else if (m_MonsterClassHash.HAdd(pClass->base.szClassName, pClass))
						pClass = nullptr;
				}
			}
			if (pClass == nullptr) pClass = m_xMonsterClassPool.newObject();
			if (pClass)
			{
				memset(pClass, 0, sizeof(MonsterClass));
				o_strncpy(pClass->base.szClassName, sfMonster[i] + 1, 16);
				pClass->pDownItems = CMonItemsMgr::GetInstance()->GetMonItems(pClass->base.szClassName);
			}
		}
		else if (*sfMonster[i] == '#') continue;
		else
		{
			if (pClass == nullptr)continue;
			char* pName = strchr(sfMonster[i], ':');
			if (pName == nullptr)continue;
			*pName++ = 0;
			if (_stricmp(sfMonster[i], "base") == 0)//怪物基本设置
			{
				xStringsExpander<6> base(pName, '/');//分割成6组
				if (base.getCount() < 6) continue;
				o_strncpy(pClass->base.szViewName, base[0], 16); // 显示名
				if (_stricmp(pClass->base.szViewName, "<e>") == 0)
					pClass->base.szViewName[0] = 0;
				pClass->base.btRace = (BYTE)StringToInteger(base[1]); // 族系
				pClass->base.btImage = (WORD)StringToInteger(base[2]); // OutView外观-普通
				pClass->base.btLevel = (BYTE)StringToInteger(base[3]); // 等级
				pClass->base.btNameColor = (BYTE)StringToInteger(base[4]); // 颜色
				pClass->base.raceimg = (BYTE)StringToInteger(base[5]); // Raceimg
			}
			else if (_stricmp(sfMonster[i], "prop") == 0)//怪物属性值
			{
				xStringsExpander<19> prop(pName, '/');
				if (prop.getCount() < 19)continue;
				pClass->prop.hp = (WORD)StringToInteger(prop[0]); // HP
				pClass->prop.mp = (WORD)StringToInteger(prop[1]); // MP
				pClass->prop.hit = (BYTE)StringToInteger(prop[2]); // 命中
				pClass->prop.speed = (BYTE)StringToInteger(prop[3]); // 躲避
				pClass->prop.ac1 = (BYTE)StringToInteger(prop[4]); // 防御
				pClass->prop.ac2 = (BYTE)StringToInteger(prop[5]); 
				pClass->prop.dc1 = (BYTE)StringToInteger(prop[6]); // 攻击
				pClass->prop.dc2 = (BYTE)StringToInteger(prop[7]);
				pClass->prop.mac1 = (BYTE)StringToInteger(prop[8]); // 魔御
				pClass->prop.mac2 = (BYTE)StringToInteger(prop[9]);
				pClass->prop.mc1 = (BYTE)StringToInteger(prop[10]); // 魔攻
				pClass->prop.mc2 = (BYTE)StringToInteger(prop[11]);
				pClass->prop.exp = (DWORD)StringToInteger(prop[12]); // 经验值
				pClass->prop.aidelay = (WORD)StringToInteger(prop[13]); // AI时间
				pClass->prop.walkdelay = (WORD)StringToInteger(prop[14]); // 行走等待
				pClass->prop.recoverhp = (WORD)StringToInteger(prop[15]); // 回血点
				pClass->prop.recoverhptime = (WORD)StringToInteger(prop[16]); // 回血时间
				pClass->prop.recovermp = (WORD)StringToInteger(prop[17]); // 回蓝点
				pClass->prop.recovermptime = (WORD)StringToInteger(prop[18]); // 回蓝时间
			}
			else if (_stricmp(sfMonster[i], "sprop") == 0)
			{
				xStringsExpander<6> sprop(pName, '/');
				if (sprop.getCount() < 5)continue;
				pClass->sprop.pFlag = (DWORD)StringToInteger(sprop[0]); // 标识
				pClass->sprop.CallRate = (BYTE)StringToInteger(sprop[1]); // 召唤系数
				pClass->sprop.AntSoulWall = (BYTE)StringToInteger(sprop[2]); // 抗灵魂墙
				pClass->sprop.AntTrouble = (BYTE)StringToInteger(sprop[3]); // 抗困魔咒
				pClass->sprop.MonsterRate = (BYTE)StringToInteger(sprop[4]); // 生怪几率
				if (sprop.getCount() > 5)
					o_strncpy(pClass->sprop.MonsterName, sprop[5], 64); // 生怪名字列表
			}
			else if (_stricmp(sfMonster[i], "aiset") == 0)
			{
				xStringsExpander<8> aiset(pName, '/');
				if (aiset.getCount() < 7)continue;
				pClass->aiset.MoveStyle = (BYTE)StringToInteger(aiset[0]); // 移动方式
				pClass->aiset.DieStyle = (BYTE)StringToInteger(aiset[1]); // 死亡方式
				pClass->aiset.TargetSelect = (BYTE)StringToInteger(aiset[2]); // 目标选择
				pClass->aiset.TargetFlag = (BYTE)StringToInteger(aiset[3]); // 目标标识
				pClass->aiset.ViewDistance = (BYTE)StringToInteger(aiset[4]); //视觉范围
				pClass->aiset.CoolEyes = (BYTE)StringToInteger(aiset[5]); //反隐范围
				pClass->aiset.EscapeDistance = (BYTE)StringToInteger(aiset[6]); // 逃跑距离
				if (aiset.getCount() > 7)
					pClass->aiset.LockDir = (BYTE)StringToInteger(aiset[7]); // 固定方向
			}
			else if (_stricmp(sfMonster[i], "petset") == 0)//宠物属性
			{
				xStringsExpander<2> petset(pName, '/');
				if (petset.getCount() < 2)continue;
				pClass->petset.Type = (BYTE)StringToInteger(petset[0]); // 类别
				pClass->petset.StopAt = (BYTE)StringToInteger(petset[1]); // 停靠
			}
			else if (_stricmp(sfMonster[i], "attack") == 0)//攻击设置
			{
				xStringsExpander<10> attack(pName, '/');
				if (attack.getCount() < 10)continue;
				pClass->attackdesc.AttackStyle = StringToInteger(attack[0]); // 攻击方式
				pClass->attackdesc.AttackDistance = StringToInteger(attack[1]); // 攻击距离
				pClass->attackdesc.Delay = StringToInteger(attack[2]); // 等待
				pClass->attackdesc.DamageStyle = StringToInteger(attack[3]);// 伤害方式
				pClass->attackdesc.DamageRange = StringToInteger(attack[4]); // 伤害范围
				pClass->attackdesc.DamageType = StringToInteger(attack[5]); // 属性
				pClass->attackdesc.AppendEffect = StringToInteger(attack[6]); // 攻击附加
				pClass->attackdesc.AppendRate = StringToInteger(attack[7]); // 附加几率
				pClass->attackdesc.CostHp = StringToInteger(attack[8]); // 消耗HP
				pClass->attackdesc.CostMp = StringToInteger(attack[9]); // 消耗MP
			}
			else if (_stricmp(sfMonster[i], "chg1") == 0) // 变身1
			{
				xStringsExpander<7> change(pName, '/');
				if (change.getCount() < 7)continue;
				pClass->changeinto[0].situation1.Situation = StringToInteger(change[0]); // 条件1
				pClass->changeinto[0].situation1.Param = StringToInteger(change[1]); // 参数1
				pClass->changeinto[0].situation2.Situation = StringToInteger(change[2]); // 条件2
				pClass->changeinto[0].situation2.Param = StringToInteger(change[3]); // 参数2
				o_strncpy(pClass->changeinto[0].szChangeInto, change[4], 16); // 变身为
				pClass->changeinto[0].AppendEffect = StringToInteger(change[5]); // 附加效果
				pClass->changeinto[0].bAnim = StringToInteger(change[6]) > 0; // 动画
				pClass->changeinto[0].bEnabled = (pClass->changeinto[0].szChangeInto[0] != 0); // 启用
			}
			else if (_stricmp(sfMonster[i], "chg2") == 0) // 变身2
			{
				xStringsExpander<7> change(pName, '/');
				if (change.getCount() < 7)continue;
				pClass->changeinto[1].situation1.Situation = StringToInteger(change[0]); // 条件1
				pClass->changeinto[1].situation1.Param = StringToInteger(change[1]); // 参数1
				pClass->changeinto[1].situation2.Situation = StringToInteger(change[2]); // 条件2
				pClass->changeinto[1].situation2.Param = StringToInteger(change[3]); // 参数2
				o_strncpy(pClass->changeinto[1].szChangeInto, change[4], 16); // 变身为
				pClass->changeinto[1].AppendEffect = StringToInteger(change[5]); // 附加效果
				pClass->changeinto[1].bAnim = StringToInteger(change[6]) > 0; // 动画
				pClass->changeinto[1].bEnabled = (pClass->changeinto[1].szChangeInto[1] != 0); // 启用
			}
			else if (_stricmp(sfMonster[i], "chg3") == 0) // 变身3
			{
				xStringsExpander<7> change(pName, '/');
				if (change.getCount() < 7)continue;
				pClass->changeinto[2].situation1.Situation = StringToInteger(change[0]); // 条件1
				pClass->changeinto[2].situation1.Param = StringToInteger(change[1]); // 参数1
				pClass->changeinto[2].situation2.Situation = StringToInteger(change[2]); // 条件2
				pClass->changeinto[2].situation2.Param = StringToInteger(change[3]); // 参数2
				o_strncpy(pClass->changeinto[2].szChangeInto, change[4], 16); // 变身为
				pClass->changeinto[2].AppendEffect = StringToInteger(change[5]); // 附加效果
				pClass->changeinto[2].bAnim = StringToInteger(change[6]) > 0; // 动画
				pClass->changeinto[2].bEnabled = (pClass->changeinto[2].szChangeInto[2] != 0); // 启用
			}
			else if (_stricmp(sfMonster[i], "append") == 0)//附加属性
			{
				xStringsExpander<4> change(pName, '/');
				if (change.getCount() < 1)continue;
				pClass->base.dwFeature = (DWORD)StringToInteger(change[0]); // 特性值-由武器值和衣服、衣服颜色、头发、头发颜色 、性别通过位运算组合后的值.
				if (change.getCount() > 1)
					pClass->attackdesc.Action = (WORD)StringToInteger(change[1]); // 动作
				else
					pClass->attackdesc.Action = 0;
				if (change.getCount() > 2)
					pClass->attackdesc.AppendTime = (WORD)StringToInteger(change[2]); // 附加时间
				else
					pClass->attackdesc.AppendTime = 0;
				if (change.getCount() > 3)
					pClass->attackdesc.AppendType = (WORD)StringToInteger(change[3]); // 附加子类型
				else
					pClass->attackdesc.AppendType = 0;
			}
		}
	}
	if (pClass != nullptr)
	{
		if (pClass->base.szClassName[0] != 0)
		{
			MonsterClass* pClassOld = (MonsterClass*)m_MonsterClassHash.HGet(pClass->base.szClassName);
			if (pClassOld != nullptr)
			{
				pClass->pszBornScript = pClassOld->pszBornScript;
				pClass->pszGotTargetScript = pClassOld->pszGotTargetScript;
				pClass->pszKillTargetScript = pClassOld->pszKillTargetScript;
				pClass->pszHurtScript = pClassOld->pszHurtScript;
				pClass->pszDeathScript = pClassOld->pszDeathScript;

				if (memcmp(pClass, pClassOld, sizeof(MonsterClass)) != 0)
				{
					memcpy(pClassOld, pClass, sizeof(MonsterClass));
					PRINT(CYAN, "怪物 %s 被更新\n", pClass->base.szClassName);
				}
			}
			else if (m_MonsterClassHash.HAdd(pClass->base.szClassName, pClass))
				pClass = nullptr;
		}
		if (pClass != nullptr)
		{
			m_xMonsterClassPool.deleteObject(pClass);
			pClass = nullptr;
		}
	}
}

VOID CMonsterManagerEx::LoadMonsterScript(const char* pszFileName)
{
	CStringFile sf(pszFileName);
	char* pLine = nullptr;
	for (int i = 0; i < sf.GetLineCount(); i++)
	{
		pLine = TrimEx(sf[i]);
		if (*pLine == 0 ||
			*pLine == '#')
			continue;
		xStringsExtracter<2> line(pLine, "=", " \t");
		if (line.getCount() < 2)continue;
		MonsterClass* pClass = this->GetClassByName(line[0]);
		if (pClass == nullptr)continue;
		xStringsExtracter<10> pages(line[1], ",", " \t");
		if (pClass->pszBornScript)
		{
			delete[]pClass->pszBornScript;
			pClass->pszBornScript = nullptr;
		}
		if (pages.getCount() > 0 && pages[0][0] != 0)
		{
			pClass->pszBornScript = copystring(pages[0]);
			PRINT(CYAN, "怪物 %s 设置出生脚本: %s\n", line[0], pages[0]);
		}

		if (pClass->pszGotTargetScript)
		{
			delete[]pClass->pszGotTargetScript;
			pClass->pszGotTargetScript = nullptr;
		}
		if (pages.getCount() > 1 && pages[1][0] != 0)
		{
			pClass->pszGotTargetScript = copystring(pages[1]);
			PRINT(CYAN, "怪物 %s 设置切换目标脚本: %s\n", line[0], pages[1]);
		}

		if (pClass->pszKillTargetScript)
		{
			delete[]pClass->pszKillTargetScript;
			pClass->pszKillTargetScript = nullptr;
		}
		if (pages.getCount() > 2 && pages[2][0] != 0)
		{
			pClass->pszKillTargetScript = copystring(pages[2]);
			PRINT(CYAN, "怪物 %s 设置被杀脚本: %s\n", line[0], pages[2]);
		}

		if (pClass->pszHurtScript)
		{
			delete[]pClass->pszHurtScript;
			pClass->pszHurtScript = nullptr;
		}
		if (pages.getCount() > 3 && pages[3][0] != 0)
		{
			pClass->pszHurtScript = copystring(pages[3]);
			PRINT(CYAN, "怪物 %s 设置受伤脚本: %s\n", line[0], pages[3]);
		}

		if (pClass->pszDeathScript)
		{
			delete[]pClass->pszDeathScript;
			pClass->pszDeathScript = nullptr;
		}
		if (pages.getCount() > 4 && pages[4][0] != 0)
		{
			pClass->pszDeathScript = copystring(pages[4]);
			PRINT(CYAN, "怪物 %s 设置死亡脚本: %s\n", line[0], pages[4]);
		}
	}
}

VOID CMonsterManagerEx::UpdateFreeObjects()
{
	CMonsterEx* pMonster = nullptr;
	const DWORD dwUpdateKey = CGameWorld::GetInstance()->GetUpdateKey();
	int nCount = m_xMonsterList.getCount();
	for (UINT i = 0; i < 500; i++)
	{
		if (m_nCurFreeIndex > m_xMonsterList.getCurPtr())
			m_nCurFreeIndex = 0;
		pMonster = m_xMonsterList.getObject(m_nCurFreeIndex);
		if (pMonster != nullptr && pMonster->IsDeath())
		{
			pMonster->SetUpdateKey(dwUpdateKey);
			pMonster->Update();
		}
		m_nCurFreeIndex++;
		if (i >= nCount - 1)return;
	}
}
