#include "StdAfx.h"
#include "humanplayer.h"
#include "gameworld.h"
#include "logicmap.h"
#include "math.h"
#include "fireburnevent.h"
#include "FireRainEvent.h"
#include "monstermanagerex.h"
#include "monsterex.h"
#include "gsclientobj.h"
#include "SpecialEquipmentManager.h"
#include "damageevent.h"
#include "magicmanager.h"
#include "monstertrapper.h"

AbilityShell g_xSkillAbilityShell42[4];
AbilityShell g_xSkillAbilityShell61[8];

VOID CHumanPlayer::SendSpecialStatusChanged(BOOL bToAround)
{
	WORD wFlag = 0; // 护身真气
	WORD wSecond = 0;
	if (IsSystemFlagSeted(SF_WINDSHIELD)) // 风影盾
		wFlag |= 2;
	if (IsSystemFlagSeted(SF_STRONGSHIELD)) // 金刚护体
		wFlag |= 1;
	if (IsSystemFlagSeted(SF_GODBLESS)) // 护身
	{
		wFlag |= 4;
		wSecond = m_SystemFlag.GetTimeOut(SF_GODBLESS) / 1000;
	}
	if (bToAround)
		SendAroundMsg(GetId(), 0x532c, wFlag, wSecond, 0);
	SendMsg(GetId(), 0x532c, wFlag, wSecond, 0);
}

VOID CHumanPlayer::resetHushenBuff(int x, int y, UINT nTarget, WORD wMagicId)
{
	USERMAGIC* pMagic = GetMagic(wMagicId);
	switch (wMagicId)
	{
	case 42: // 护身真气
	{
		SendAroundMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[hushenleve]);
		SendMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[hushenleve]);
	}
	break;
	case 61: // 金刚护体
	{
		SendAroundMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[hushenleve]);
		SendMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[hushenleve]);
	}
	break;
	}
}

BOOL CHumanPlayer::GetHushen()
{
	if (IsStatusSet(SI_HUSHENZHENQI)) return TRUE;
	return FALSE;
}

BOOL CHumanPlayer::GetJingang()
{
	if (IsSystemFlagSeted(SF_STRONGSHIELD)) return TRUE;
	return FALSE;
}

DWORD CHumanPlayer::GetTianXuanCallLevel(BYTE btLevel)
{
	auto* pSpecialEquipMgr = CSpecialEquipmentManager::GetInstance();
	if (this->IsSpecialEquipmentFunctionOn(SEF_TIANXUANALL))
		return pSpecialEquipMgr->GetFunctionParam(SEF_TIANXUANALL, 0);
	else if (this->IsSpecialEquipmentFunctionOn(SEF_TIANXUANHALF))
		return pSpecialEquipMgr->GetFunctionParam(SEF_TIANXUANHALF, 0);
	return btLevel; // 返回技能默认召唤宝宝等级
}

DWORD CHumanPlayer::GetTianXuannDamage()
{
	auto* pSpecialEquipMgr = CSpecialEquipmentManager::GetInstance();
	if (this->IsSpecialEquipmentFunctionOn(SEF_TIANXUANALL))
		return pSpecialEquipMgr->GetFunctionParam(SEF_TIANXUANALL, 1);
	else if (this->IsSpecialEquipmentFunctionOn(SEF_TIANXUANHALF))
		return pSpecialEquipMgr->GetFunctionParam(SEF_TIANXUANHALF, 1);
	return 0; // 默认值
}

int CHumanPlayer::CalcLucky()
{
	int maxmc = 0;
	int minmc = 0;
	BYTE bJob = GetPro(); // 职业 0战士、1法师、2道士
	if (bJob == JOB_WAR)
	{
		maxmc = GetPropValue(PI_MAXDC);
		minmc = GetPropValue(PI_MINDC);
	}
	if (bJob == JOB_MAG)
	{
		maxmc = GetPropValue(PI_MAXMC);
		minmc = GetPropValue(PI_MINMC);
	}
	if (bJob == JOB_TAO)
	{
		maxmc = GetPropValue(PI_MAXSC);
		minmc = GetPropValue(PI_MINSC);
	}
	int nlucky = GetPropValue(PI_LUCKY) - GetPropValue(PI_DAWN);
	if (nlucky > 0 && Getrand(100) < g_pLucky[nlucky])
		minmc = maxmc - (maxmc - minmc) * max(10 - nlucky, 0) / 10;
	else if (nlucky < 0)
		maxmc = minmc + (maxmc - minmc) * max(nlucky + 10, 0) / 10;
	return GetRangeRand(minmc, maxmc);
}

BOOL CHumanPlayer::SpellCast(int x, int y, UINT nTarget, WORD wMagicId)
{
	// 检查是否学过该技能
	USERMAGIC* pMagic = GetMagic(wMagicId);
	if (pMagic == nullptr) return FALSE;
	const char* szName = pMagic->pClass->szName;
	CLogicMap* pMap = GetMap();
	std::vector<std::string> dwExtraParams;
	// 地图检查：禁止使用技能
	if (pMap && pMap->IsFlagSeted(MF_NOSKILL, g_dwInterFlag, dwExtraParams))
	{
		for (int i = 0; i < dwExtraParams.size(); i++)
		{
			if (szName == dwExtraParams[i])
			{
				SaySystem("此地图禁止使用 %s 技能!", szName);
				return FALSE;
			}
		}
	}
	// 检查技能使用时间间隔
	if (!pMagic->useTimer.IsTimeOut(pMagic->pClass->wDelay)) return FALSE;
	// 检查技能是是开关式,如果不是, 就去判断是否可以释放技能
	if ((pMagic->pClass->dwFlag & MAGICFLAG_ACTIVED) == 0)
		if (!CanDoAction(AT_SPELLSKILL)) return FALSE;
	
	// 缓存常用属性和Magic数据
	const Magic& magicSkill = CMagicManager::GetInstance()->GetMagic(wMagicId);
	const BYTE btLevel = pMagic->magic.btLevel;
	const auto& skillData = magicSkill.skills[btLevel];
	const DWORD dwClassFlag = pMagic->pClass->dwFlag;
	
	BOOL bShowEffect = ((dwClassFlag & MAGICFLAG_NOEFFECT) == 0); // 是否显示技能效果
	BOOL bTrain = ((dwClassFlag & MAGICFLAG_ACTIVED) == 0 && (dwClassFlag & MAGICFLAG_FORCED) == 0); // 是否计算技能经验

	// 魔法值检查
	DWORD dwSpellMpCost = 0;
	const BOOL bCostMp = ((dwClassFlag & MAGICFLAG_ACTIVED) == 0 || (dwClassFlag & MAGICFLAG_FORCED) == 0);
	if (bCostMp)
	{
		dwSpellMpCost = (DWORD)pMagic->pClass->GetSpellPoint(btLevel);
		if ((DWORD)GetPropValue(PI_CURMP) < dwSpellMpCost)
		{
			SaySystem("魔法值不足,无法施放 %s 技能!", szName);
			bShowEffect = FALSE;
			bTrain = FALSE;
			wMagicId = 0; // 设置一个无效的技能id
		}
	}
	
	// 缓存常用属性
	BOOL bSuccess = TRUE; // 是否使用技能成功
	const BYTE bJob = GetPro(); // 职业 0战士、1法师、2道士
	const int nSrcX = getX();
	const int nSrcY = getY();
	const int nLevel = GetPropValue(PI_LEVEL);
	int dir = GetDirection();
	// 道士物品检查
	int needcount = 0;
	e_skill_need_item needitem = SNI_CHARM;
	if (bJob == JOB_TAO) // 道士需要物品判断
	{
		int checkcount = 0;
		const int nCharmCount = pMagic->pClass->wCharmCount;
		if (dwClassFlag & MAGICFLAG_USECHARM) // 道符
		{
			needcount++;
			if (CheckMaterial(ISM_CHARM, 5, 0, nCharmCount))
			{
				needitem = SNI_CHARM;
				checkcount++;
			}
		}
		// 毒药/木偶检查函数
		auto CheckDualItem = [&](DWORD dwFlag1, e_skill_need_item item1, DWORD dwFlag2, e_skill_need_item item2, BYTE special)
			{
				bool hasItem1 = (dwClassFlag & dwFlag1) && CheckMaterial(ISM_POISON, 1, special, nCharmCount);
				bool hasItem2 = (dwClassFlag & dwFlag2) && CheckMaterial(ISM_POISON, 2, special, nCharmCount);
				if (dwClassFlag & dwFlag1) needcount++;
				if (dwClassFlag & dwFlag2) needcount++;
				if (hasItem1 && !hasItem2)
				{
					needitem = item1;
					checkcount++;
				}
				else if (!hasItem1 && hasItem2)
				{
					needitem = item2;
					checkcount++;
				}
				else if (hasItem1 && hasItem2)
				{
					needitem = boPoison ? item1 : item2;
					checkcount++;
				}
			};
		if (wMagicId == 6) // 施毒术
			CheckDualItem(MAGICFLAG_USEGREENPOISON, SNI_GREENPOISON, MAGICFLAG_USEREDPOISON, SNI_REDPOISON, 0);
		else if (wMagicId == 45) // 诅咒术
			CheckDualItem(MAGICFLAG_USESTRAWMAN, SNI_STRAWMAN, MAGICFLAG_USESTRAWWOMAN, SNI_STRAWWOMAN, 1);

		if (needcount > 0 && checkcount == 0)
		{
			SaySystem("没有足够的物品, 无法施展 %s 技能!", szName);
			bShowEffect = FALSE;
			wMagicId = 0; // 设置一个无效的技能id
		}
	}

	CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(nTarget); // 攻击目标
	if (pObject == nullptr) pObject = this;
	if (bJob == JOB_WAR) // 战士技能
	{
		switch (wMagicId)
		{
		case 26: // 烈火剑法
		{
			// 如果已经激活, 就无法再激活
			if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED) return FALSE;
			// 如果间隔时间小于五秒中, 就无法激活
			if (!pMagic->useTimer.IsTimeOut(6000)) return FALSE;
			m_pTimeOutDeActiveMagic = pMagic;
			pMagic->useTimer.SetTimeOut(11500);
			pMagic->dwFlag |= USERMAGICFLAG_ACTIVED;
			pMagic->useTimer.Savetime();
			SaySystemAttrib(CC_MAGICTIPS, "你的武器因精神火球而炙热");
			if (m_pClientObj)m_pClientObj->PostMsg("#+FIR!", 6);
			return TRUE;
		}
		break;
		case 27: // 野蛮冲撞
		{
			const int nDamage1 = skillData.value1;
			const int nDamage2 = skillData.value2;
			const int nGrid = MAKEWORD(skillData.value3, pMagic->magic.btLevel);
			const int nDamage3 = skillData.value4;
			const int nGridDelay = skillData.value5;
			const int nDamage = Getrand(5);
			bTrain = AddProcess(EP_RUSH, MAKEDWORD2W(nGrid, nGridDelay), nDamage1 + nDamage, nDamage2 + nDamage, MAKEDWORD2W(dir, nDamage3), 1);
		}
		break;
		case 42: // 护身真气
		{
			if (IsStatusSet(SI_HUSHENZHENQI))
				bSuccess = FALSE;
			else
			{
				if (IsSystemFlagSeted(SF_STRONGSHIELD)) // 如果金刚护体还在, 清理掉
					SetSystemFlag(SF_STRONGSHIELD, FALSE);
				const int index = btLevel % 4;
				this->m_xAbilityShellRef.pShell = &g_xSkillAbilityShell42[index];
				this->m_xAbilityShellRef.pShell->wMaxHp = magicSkill.skills[index].value3 * 10;
				this->m_xAbilityShellRef.pShell->wAddHp = magicSkill.skills[index].value1;
				this->m_xAbilityShellRef.pShell->wAddAc = magicSkill.skills[index].value2;
				this->m_xAbilityShellRef.dwCurExp = 0;
				this->m_xAbilityShellRef.wCurHp = magicSkill.skills[index].value3 * 10;
				this->m_xAbilityShellRef.wLevel = index;
				this->m_xAbilityShellRef.wSkillId = wMagicId;
				hushenleve = index;
				SendAroundMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[index]);
				SendMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[index]);
				UpdateProp(); // 更新人物数据
				bTrain = SetStatus(SI_HUSHENZHENQI, 0, 0xffffffff);
			}
		}
		break;
		case 44: // 雷霆剑
		{
			//	如果已经激活, 就无法再激活
			if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED) return FALSE;
			//	如果间隔时间小于五秒中, 就无法激活
			if (!pMagic->useTimer.IsTimeOut(6000)) return FALSE;
			m_pTimeOutDeActiveMagic = pMagic;
			pMagic->useTimer.SetTimeOut(11500);
			pMagic->dwFlag |= USERMAGICFLAG_ACTIVED;
			pMagic->useTimer.Savetime();
			SaySystemAttrib(CC_MAGICTIPS, "你被赋予雷电的力量");
			if (m_pClientObj)m_pClientObj->PostMsg("#+THU!", 6);
			return TRUE;
		}
		break;
		case 61: // 金刚护体
		{
			if (IsSystemFlagSeted(SF_STRONGSHIELD))
				bSuccess = FALSE;
			else
			{
				if (IsStatusSet(SI_HUSHENZHENQI)) // 护身真气状态, 清除掉
					ClrStatus(SI_HUSHENZHENQI);
				const int index = btLevel % 4;
				this->m_xAbilityShellRef.pShell = &g_xSkillAbilityShell61[index];
				this->m_xAbilityShellRef.dwCurExp = 0;
				this->m_xAbilityShellRef.pShell->wAddAc = magicSkill.skills[index].value3;
				this->m_xAbilityShellRef.pShell->wAddMac = magicSkill.skills[index].value4;
				this->m_xAbilityShellRef.pShell->wAddHp = magicSkill.skills[index].value1;
				this->m_xAbilityShellRef.pShell->wAddMp = magicSkill.skills[index].value2;
				this->m_xAbilityShellRef.pShell->wMaxHp = magicSkill.skills[index].value5 * 10;
				this->m_xAbilityShellRef.wCurHp = magicSkill.skills[index].value5 * 10;
				this->m_xAbilityShellRef.wLevel = index;
				this->m_xAbilityShellRef.wSkillId = wMagicId;
				hushenleve = index;
				JingganNoDamage = magicSkill.skills[index].value6;
				SendAroundMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[index]);
				SendMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[index]);
				SetSystemFlag(SF_STRONGSHIELD, TRUE, btLevel, 0xffffffff);
				UpdateProp();
				UpdateSubProp();
				bTrain = TRUE;
			}
		}
		break;
		case 62: // 突斩
		{
			const int nDamage1 = skillData.value1;
			const int nDamage2 = skillData.value2;
			const int nGrid = MAKEWORD(skillData.value3, pMagic->magic.btLevel);
			const int nDamage3 = skillData.value4;
			const int nGridDelay = skillData.value5;
			const int nDamage = Getrand(5);
			bTrain = AddProcess(EP_RUSHHIT, MAKEDWORD2W(nGrid, nGridDelay), nDamage1 + nDamage, nDamage2 + nDamage, MAKEDWORD2W(dir, nDamage3), 1);
		}
		break;
		case 74: // 擒龙手 
		{
			bTrain = FALSE;
			if (!IsProperTarget(pObject)) 
				break;
			if (Getrand(100 - pMagic->magic.btLevel * 25) < 20)
			{
				SaySystem("没有足够的抓取能力");
				break;
			}
			if (pObject != this || pObject->GetType() != OBJ_MONSTER || pObject->IsDeath() ||
				pObject->GetPropValue(PI_LEVEL) >= GetPropValue(PI_LEVEL))
			{
				SaySystem("没有足够的抓取能力");
				break;
			}
			int x = getX();
			int y = getY();
			GETNEXTPOS(x, y, dir);
			bTrain = pObject->FlyTo(GetMap(), x, y);
			if (pObject->GetType() == OBJ_MONSTER)
				pObject->SetSystemFlag(SF_SLEEP, TRUE, 0, 1000); // 眩晕(休眠1秒)
		}
		break;
		case 77: // 移形换影
		{
			if (m_pMap == nullptr) return FALSE;
			bTrain = FALSE;
			if (m_pMap->IsFlagSeted(MF_NOFLASHMOVE))
			{
				SaySystem("当前地图无法使用移形换影!");
				break;
			}
			if (m_pMap->IsBlocked(x, y))
			{
				SaySystem("坐标无法到达, 移形换影失败!");
				break;
			}
			const int maxDis = skillData.value1; // 飞行最大距离
			if (DISTANCE(x, y, nSrcX, nSrcY) > maxDis)
			{
				SaySystem("距离太远, 移形换影使用失败!");
				break;
			}
			if (SpellFly(x, y, wMagicId))
				bTrain = TRUE;
			else
				bSuccess = FALSE;
		}
		break;
		default:
		{
			if (dwClassFlag & MAGICFLAG_ACTIVED)
			{
				pMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
				if (pMagic->pClass->szSpecial[0] != 0)
				{
					char szMsg[200];
					if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
						_snprintf(szMsg, sizeof(szMsg) - 1, "#+%s!", pMagic->pClass->szSpecial);
					else
						_snprintf(szMsg, sizeof(szMsg) - 1, "#+U%s!", pMagic->pClass->szSpecial);
					if (m_pClientObj)m_pClientObj->PostMsg(szMsg, (int)strlen(szMsg));
				}
				if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
					SaySystemAttrib(CC_GREENS, "开启%s", szName);
				else
					SaySystemAttrib(CC_GREENS, "关闭%s", szName);
				return TRUE;
			}
			else
				DPRINT(ERROR_RED, "未处理的技能 %u \n", wMagicId);
		}
		break;
		}
	}
	if (bJob == JOB_MAG) // 法师技能
	{
		switch (wMagicId)
		{
		case 1: // 小火球
		case 5: // 火炎刀
		{
			bTrain = FALSE;
			if (!IsProperTarget(pObject))
				break;
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
#ifdef _DEBUG
			if (wMagicId == 5)
				SaySystem("火炎刀 伤害值:%d", nDamage);
			if (wMagicId == 1)
				SaySystem("小火球 伤害值:%d", nDamage);
#endif
			int nDis = DISTANCE(pObject->getX(), pObject->getY(), nSrcX, nSrcY);
			bTrain = pObject->AddProcess(EP_BEATTACKED, nDamage, GetId(), DT_MAGIC, 0, 120 * nDis + 500);
		}
		break;
		case 8: // 抗拒火环
		{
			const int count = skillData.value1;
			const int probability = skillData.value2;
			bTrain = (Getrand(100) < probability) ? ~DoMagicPushAround(count) : FALSE;
		}
		break;
		case 9: // 地狱火焰
		case 10: // 疾光电影
		case 51: // 冰龙破
		{
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
			const int nCount = skillData.value3;
#ifdef _DEBUG
			if (wMagicId == 9)
				SaySystem("地狱火焰 伤害值:%d", nDamage);
			else if (wMagicId == 10)
				SaySystem("疾光电影 伤害值:%d", nDamage);
			else if (wMagicId == 51)
				SaySystem("冰龙破 伤害值:%d", nDamage);
#endif
			bTrain = DoLineMagic(nDamage, dir, nCount);
		}
		break;
		case 11: // 雷电术
		case 49: // 冰箭术
		case 71: // 狂龙紫电
		{
			bTrain = FALSE;
			if (!IsProperTarget(pObject)) 
				break;
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
#ifdef _DEBUG
			if (wMagicId == 11)
				SaySystem("雷电术 伤害值:%d", nDamage);
			else if (wMagicId == 49)
				SaySystem("冰箭术 伤害值:%d", nDamage);
			else if (wMagicId == 71)
				SaySystem("狂龙紫电 伤害值:%d", nDamage);
#endif
			bTrain = DoPointMagic(nDamage, x, y);
		}
		break;
		case 20: // 诱惑之光
		{
			if (pObject && pObject != this && pObject != m_pPet)
			{
				bTrain = TRUE;
				CMonsterEx* pMonster = (CMonsterEx*)pObject;
				const int iLevel = pMagic->magic.btLevel;
				if (pMonster->GetOwner() != this || pMonster->GetOwner() == nullptr)
				{
					BYTE pType = pMonster->GetDesc()->petset.Type;
					if (pType != APT_DEF && pType != APT_CALL) break;
					const int value3 = MIN(skillData.value3, 5); // 最多宝宝数量
					const int value2 = skillData.value2; // 允许诱惑怪物最高等级
					if (pMonster == m_pSeizedObject)
						m_iSeizedTimes++;
					else
					{
						m_pSeizedObject = pMonster;
						m_iSeizedTimes = 1;
					}
					const int value1 = skillData.value1; // 诱惑成功几率(百分比)
					const int value4 = skillData.value4; // 时间基数(分钟)
					const int value5 = skillData.value5; // 黄名几率(百分比)
					if (pMonster->HolySeizedOk(this, iLevel, m_iSeizedTimes, value1, value5))
					{
						if (m_iPetCount >= value3)
						{
							int nRand = Getrand(100);
							if (nRand < value5)
								pMonster->SetSystemFlag(SF_SLEEP, TRUE, 0, (3 * (iLevel + 1) + Getrand(5 * (iLevel + 1))) * 1000);
							break;
						}
						if (pMonster->GetDesc()->base.btLevel > value2)
						{
							int nRand = Getrand(100);
							if (nRand < value5)
								pMonster->SetSystemFlag(SF_SLEEP, TRUE, 0, (3 * (iLevel + 1) + Getrand(5 * (iLevel + 1))) * 1000);
							break; 
						}
						if (AddPet(pMonster))
						{
							m_wPetSkill = wMagicId;
							if (pMonster->GetDesc())
							{
								BYTE btLevel = iLevel;
								USERMAGIC* pCallMagic = GetMagic(69); // 兽灵术
								if (pCallMagic)
								{
									TrainMagic(pCallMagic);
									btLevel = 3 + pCallMagic->magic.btLevel / 2;
								}
								char szClassName[64];
								sprintf(szClassName, "%s%d", pMonster->GetDesc()->base.szViewName, btLevel);
								MonsterClass* pDesc = CMonsterManagerEx::GetInstance()->GetClassByName(szClassName);
								if (pDesc)
									pMonster->SetDesc(pDesc);
							}
							pMonster->SetOwner(this);
							pMonster->AddPropValue(PI_CURHP, pMonster->GetPropValue(PI_MAXHP));

							int minmc = GetPropValue(PI_MINMC);
							int maxmc = GetPropValue(PI_MAXMC);
							int nDamage = 0;
							if (GetPropValue(PI_LUCKY) >= 9)
								nDamage = GetAttackPower(maxmc, 1);
							else
								nDamage = GetAttackPower(minmc, maxmc - minmc + 1);
							pMonster->SetBetrayInfo((nDamage + value4) * 60000);

							MONSTERGEN* pGen = pMonster->GetGen();
							if (pGen)
							{
								pGen->curcount--;
								if (pGen->curcount < 0)
									pGen->curcount = 0;
							}
						}
						else
						{
							m_pSeizedObject = nullptr;
							m_iSeizedTimes = 0;
						}
					}
				}
				else
					pMonster->SetSystemFlag(SF_SLEEP, TRUE, 0, (3 * (iLevel + 1) + Getrand(5 * (iLevel + 1))) * 1000);
			}
		}
		break;
		case 21: // 瞬间移动
		{
			const int probability = skillData.value1;
			if (Getrand(100) < probability)
			{
				bSuccess = EscapeMap();
				if (!bSuccess)
					SaySystem("当前地图无法使用瞬间移动技能!");
				else
					bTrain = TRUE;
			}
			else
				bTrain = FALSE;
		}
		break;
		case 22: // 火墙
		{
			int minmc = GetPropValue(PI_MINMC);
			int maxmc = GetPropValue(PI_MAXMC);
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
			const int dwTick = skillData.value3;
			const int dwTime = skillData.value4;
#ifdef _DEBUG
			SaySystem("火墙 时间 %u 秒", dwTime);
#endif
			bTrain = AddProcess(EP_FIREBURN, MAKEDWORD2W(x, y), nDamage, dwTick, dwTime, 600);
		}
		break;
		case 23: // 爆裂火焰
		case 24: // 地狱雷光
		case 33: // 冰咆哮
		{
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
			int nDelay = 800;
			if (wMagicId == 24)
			{
				x = getX();
				y = getY();
				nDelay = 400;
			}
			const int nCount = skillData.value3;
#ifdef _DEBUG
			if (wMagicId == 23)
				SaySystem("爆裂火焰 伤害值:%d", nDamage);
			else if (wMagicId == 24)
				SaySystem("地狱雷光 伤害值:%d", nDamage);
			else if (wMagicId == 33)
				SaySystem("冰咆哮 伤害值:%d", nDamage);
#endif
			bTrain = MagicBoom(nDamage, x, y, nCount, nDelay);
		}
		break;
		case 31: // 魔法盾
		{
			ResMag_Count = skillData.value2 * skillData.value5; // 魔法盾抵抗次数
			NoDamage = skillData.value3; // 魔法盾抵抗百分比
			const int pow = skillData.value1;
			const int time = skillData.value4;
			const DWORD dwTime = static_cast<DWORD>(pow * time);
			bTrain = AddProcess(EP_SETSTATUS, SI_BUBBLEDEFENCEUP, btLevel, dwTime * 1000, 0, 200);
#ifdef _DEBUG
			SaySystem("魔法盾 时间 %u 秒", dwTime);
#endif
		}
		break;
		case 32: // 圣言术
		{
			const int value1 = skillData.value1; // 允许圣言最高等级
			const int value2 = skillData.value2; // 杀死几率
			if ((pObject->GetType() == OBJ_MONSTER || pObject->GetType() == OBJ_PET) &&
				!pObject->IsDeath() && IsProperTarget(pObject) &&
				pObject->GetPropValue(PI_LEVEL) < nLevel &&
				pObject->GetPropValue(PI_LEVEL) <= value1 &&
				((CMonsterEx*)pObject)->GetDesc() &&
				(((CMonsterEx*)pObject)->GetDesc()->sprop.pFlag & SF_HOLYWORD) != 0 &&
				Getrand(100) < value2)
			{
				pObject->ToDeath(GetId());
				bTrain = TRUE;
			}
			else
				bTrain = FALSE;
		}
		break;
		case 35: // 化身蝙蝠 
		{
			if (m_pMap == nullptr)return FALSE;
			bTrain = FALSE;
			if (m_pMap->IsFlagSeted(MF_NOFLASHMOVE))
			{
				SaySystem("当前地图无法使用化身蝙蝠!");
				break;
			}
			if (m_pMap->IsBlocked(x, y))
			{
				SaySystem("坐标无法到达, 化身蝙蝠失败!");
				break;
			}
			const int maxDis = skillData.value1; // 飞行最大距离
			if (DISTANCE(x, y, nSrcX, nSrcY) > maxDis)
			{
				SaySystem("距离太远, 化身蝙蝠使用失败!");
				break;
			}
			const int probability = skillData.value2; // 成功几率
			if (Getrand(100) >= probability)
			{
				SaySystem("化身蝙蝠使用失败!");
				break;
			}
			if (SpellFly(x, y, wMagicId))
				bTrain = TRUE;
			else
				bSuccess = FALSE;
		}
		break;
		case 50: // 风火轮
		{
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
			const int dwTime = skillData.value3;
			bTrain = SetStatus(SI_FENGHUOLUN, nDamage, dwTime * 1000);
#ifdef _DEBUG
			SaySystem("风火轮 时间 %u 秒", dwTime);
#endif
		}
		break;
		case 52: // 玄冰刃
		{
			static int effectdir[4] = { 1, 3, 5, 7 };
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
			const int nCount = skillData.value3;
#ifdef _DEBUG
			SaySystem("玄冰刃 伤害值:%d", nDamage);
#endif
			int nScu = 0;
			for (int i = 0; i < 4; i++)
			{
				if(DoLineMagic(nDamage, effectdir[i], nCount))
					nScu++;
			}
			if (nScu > 0) bTrain = TRUE;
		}
		break;
		case 53: // 五雷轰
		{
			static POINT ptEffect[4] = { { 1, 1}, { -1, 1}, { 1, -1}, { -1, -1} }; // 四个方向点
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
			const int nCount = skillData.value3;
#ifdef _DEBUG
			SaySystem("五雷轰 伤害值:%d", nDamage);
#endif
			int nScu = 0;
			if (nCount > 0 && DoPointMagic(nDamage, x, y)) // 中心点必中
				nScu++;
			for (int i = 0; i < nCount - 1; i++)
			{
				POINT nEffect = ptEffect[Getrand(4)];
				if (DoPointMagic(nDamage, x + nEffect.x, nEffect.y + y))
					nScu++;
			}
			if (nScu > 0) bTrain = TRUE;
		}
		break;
		case 54: // 冰旋风
		{
			if (m_pMap == nullptr) return FALSE;
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
			const int value3 = skillData.value3;
			const int value4 = skillData.value4;
#ifdef _DEBUG
			SaySystem("冰旋风 伤害值:%d", nDamage);
#endif
			bTrain = m_pMap->DamageAround(this, x, y, value4, nDamage, DT_MAGIC, 0, 600, Getrand(100) < value3);
		}
		break;
		case 70: // 风影盾 
		{
			if (IsSystemFlagSeted(SF_WINDSHIELD))
				bSuccess = FALSE;
			else
			{
				const int pow = skillData.value1;
				const int power = skillData.value6;
				const DWORD dwTime = pow * power;
#ifdef _DEBUG
				SaySystem("风影盾 时间 %u 秒", dwTime);
#endif
				bTrain = AddProcess(EP_SETSYSTEMFLAG, SF_WINDSHIELD, TRUE, btLevel, dwTime * 1000, 200);
			}
		}
		break;
		case 73: // 流星火雨
		{
			const int value3 = skillData.value3; // 火云最长持续时间
			const int value4 = skillData.value4; // 火雨固定浮动伤害
			const int value5 = skillData.value5; // 火雨伤害范围
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
			MagicBoom(nDamage + value4, x, y, value5, 1600); // 火雨
#ifdef _DEBUG
			SaySystem("流星火雨 时间 %u 秒", value3);
#endif
			bTrain = AddProcess(EP_FIRERAIN, MAKEDWORD2W(x,y), value5, nDamage, value3, 1600); // 火云
		}
		break;
		default:
		{
			if (pMagic->pClass->dwFlag & MAGICFLAG_ACTIVED)
			{
				pMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
				if (pMagic->pClass->szSpecial[0] != 0)
				{
					char szMsg[200];
					if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
						_snprintf(szMsg, sizeof(szMsg) - 1, "#+%s!", pMagic->pClass->szSpecial);
					else
						_snprintf(szMsg, sizeof(szMsg) - 1, "#+U%s!", pMagic->pClass->szSpecial);
					if (m_pClientObj)m_pClientObj->PostMsg(szMsg, (int)strlen(szMsg));
				}
				if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
					SaySystemAttrib(CC_GREENS, "开启%s", szName);
				else
					SaySystemAttrib(CC_GREENS, "关闭%s", szName);
				return TRUE;
			}
		}
		break;
		}
	}
	if (bJob == JOB_TAO) // 道士技能
	{
		switch (wMagicId)
		{
		case 2: // 治疗术
		{
			bTrain = FALSE;
			if (!IsProperFriend(pObject))
				break;
			int nDamage = getskillpower(wMagicId); // 计算回回血总量
			const int value3 = skillData.value3; // 每次加血的数量
#ifdef _DEBUG
			SaySystem("治疗术 加血值:%d", nDamage);
#endif
			bTrain = pObject->AddProcess(EP_MAGHEALING, nDamage, value3, 0, 0, 600);
		}
		break;
		case 6: // 施毒术
		{
			bTrain = FALSE;
			if (!IsProperTarget(pObject)) 
				break;
			int nPower = GetRangeRand(GetPropValue(PI_MINSC), GetPropValue(PI_MAXSC));
			const int hongdu = skillData.value1; // 红毒降低防御点数
			const int lvdu = skillData.value2 + GetTianXuannDamage(); // 绿毒威力
			const int value3 = skillData.value3; // 中毒最长时间
			nPower += value3;
#ifdef _DEBUG
			SaySystem("施毒术 时间 %d 秒", nPower);
#endif
			bTrain = AddProcess(EP_POISON, nTarget, needitem, MAKEDWORD2W(hongdu, lvdu), nPower, 800);
		}
		break;
		case 13: // 灵魂道符
		case 67: // 幽冥火咒
		{
			bTrain = FALSE;
			if (!IsProperTarget(pObject))
				break;
			int nDamage = getskillpower(wMagicId);
			nDamage += CalcLucky();
			nDamage = MAX(0, nDamage);
#ifdef _DEBUG
			if (wMagicId == 13)
			{
				SaySystem("灵魂道符 伤害值:%d", nDamage);
			}
			if (wMagicId == 67)
			{
				SaySystem("幽冥火咒 伤害值:%d", nDamage);
			}
#endif
			int nDis = DISTANCE(pObject->getX(), pObject->getY(), nSrcX, nSrcY);
			bTrain = pObject->AddProcess(EP_BEATTACKED, nDamage, GetId(), DT_MAGIC, 0, 120 * nDis + 500);
		}
		break;
		case 14: // 幽灵盾
		{
			const int pow = skillData.value1;
			const int value2 = skillData.value2;
			int nPower = CalcLucky();
#ifdef _DEBUG
			SaySystem("幽灵盾 时间 %d 秒", nPower + pow);
#endif
			int nDis = DISTANCE(pObject->getX(), pObject->getY(), nSrcX, nSrcY);
			bTrain = AddProcess(EP_DEFENCEUP, MAKEDWORD2W(x, y), value2, nPower + pow, SI_DEFENCEUP, 120 * nDis + 500);
		}
		break;
		case 15: // 神圣战甲术
		{
			const int pow = skillData.value1;
			const int value2 = skillData.value2;
			int nPower = CalcLucky();
#ifdef _DEBUG
			SaySystem("神圣战甲术 时间 %d 秒", nPower + pow);
#endif
			int nDis = DISTANCE(pObject->getX(), pObject->getY(), nSrcX, nSrcY);
			bTrain = AddProcess(EP_DEFENCEUP, MAKEDWORD2W(x, y), value2, nPower + pow, SI_MAGDEFENCEUP, 120 * nDis + 500);
		}
		break;
		case 16: // 困魔咒
		{
			if (!IsProperTarget(pObject))
			{
				bShowEffect = FALSE;
				bTrain = FALSE;
				break;
			}
			int nTime = CalcLucky();
			const int nPow = skillData.value1;
			nTime += nPow;
#ifdef _DEBUG
			SaySystem("困魔咒 时间 %d 秒", nTime);
#endif
			CMonsterTrapper::Create(this, x, y, nLevel, pMagic->dwColor, nTime * 1000);
			bTrain = TRUE;
		}
		break;
		case 17: // 召唤骷髅
		{
			bTrain = FALSE;
			if (m_iPetCount > 0) 
			{
				bShowEffect = FALSE;
				break;
			}
			char szClassName[100];
			DWORD nLevel = GetTianXuanCallLevel(btLevel);
			sprintf(szClassName, "%s%u", pMagic->pClass->szSpecial, nLevel);
			bTrain = SummonPet(szClassName);
			if (bTrain) this->m_wPetSkill = wMagicId;
		}
		break;
		case 18: // 隐身术
		{
			bTrain = FALSE;
			if (!IsProperFriend(pObject))
				break;
			int nTime = CalcLucky();
			const int nPow = skillData.value1;
			nTime += nPow;
#ifdef _DEBUG
			SaySystem("隐身时间 %u 秒!", nTime);
#endif
			DWORD dwParam = (pObject->getX() << 16) | pObject->getY();
			if (IsStatusSet(SI_CLOAK))
				break;
			else
				bTrain = pObject->AddProcess(EP_SETSTATUS, SI_CLOAK, dwParam, nTime * 1000);
		}
		break;
		case 19: // 群体隐身术
		{
			int nTime = CalcLucky();
			const int nPow = skillData.value1;
			nTime += nPow;
#ifdef _DEBUG
			SaySystem("群隐身时间 %u 秒!", nTime);
#endif
			int nDis = DISTANCE(x, y, nSrcX, nSrcY);
			bTrain = AddProcess(EP_GROUPCLOAK, MAKEDWORD2W(x, y), nTime, 0, 0, 500);
		}
		break;
		case 29: // 群体治愈术
		{
			int nDamage = getskillpower(wMagicId); // 回血总量
			const int value3 = skillData.value3; // 每次加血的数量
			const int value4 = skillData.value4; // 范围
#ifdef _DEBUG
			SaySystem("群体治愈术 加血值:%d", nDamage);
#endif
			bTrain = AddProcess(EP_GROUPMAGHEALING, MAKEDWORD2W(x, y), value4, nDamage, value3, 500);
		}
		break;
		case 30: // 召唤神兽
		{
			bTrain = FALSE;
			if (m_iPetCount > 0) 
			{
				bShowEffect = FALSE;
				break;
			}
			char szClassName[100];
			DWORD nLevel = GetTianXuanCallLevel(btLevel);
			sprintf(szClassName, "%s%u", pMagic->pClass->szSpecial, nLevel);
			bTrain = SummonPet(szClassName);
			if (bTrain) this->m_wPetSkill = wMagicId;
		}
		break;
		case 37: // 狮子吼
		{
			int tx = nSrcX;
			int ty = nSrcY;
			CAliveObject* pFrontObject = nullptr;
			if (pObject)
			{
				int nDis = DISTANCE(pObject->getX(), pObject->getY(), tx, ty);
				if (nDis == 1)
				{
					dir = GetDirectionTo(getX(), getY(), pObject->getX(), pObject->getY());
					SetDirection((e_direction)dir);
					pFrontObject = pObject;
				}
			}
			else
			{
				GETNEXTPOS(tx, ty, dir);
				pFrontObject = (CAliveObject*)m_pMap->FindTarget(this, tx, ty);
			}
			CAliveObject* pFrontObject2 = nullptr;
			if (btLevel == 3)
			{
				GETNEXTPOS(tx, ty, dir);
				pFrontObject2 = (CAliveObject*)m_pMap->FindTarget(this, tx, ty);
				if (pFrontObject2 == nullptr || (pFrontObject2 && pFrontObject2->IsDeath()) || (pFrontObject2 && !pFrontObject2->CanBePushed(this)))
					pFrontObject2 = nullptr;
			}
			if (pFrontObject && !pFrontObject->IsDeath() && pFrontObject->CanBePushed(this))
			{
				const int nCount = skillData.value4; // 推人距离
				int i = 0;
				for (i = 0; i < nCount; i++)
				{
					if (!pFrontObject->DoLionPush(dir))break;
					if (pFrontObject2 && !pFrontObject2->DoPushed(dir))break;
				}
				const int nGridDelay = skillData.value3; // 卡位时间
				if (pFrontObject->GetType() == OBJ_PLAYER)
					pFrontObject->SetAction(AT_BEATTACK, pFrontObject->GetDirection(), pFrontObject->getX(), pFrontObject->getY(), nGridDelay* i);
				if (pFrontObject2 && pFrontObject2->GetType() == OBJ_PLAYER)
					pFrontObject2->SetAction(AT_BEATTACK, pFrontObject2->GetDirection(), pFrontObject2->getX(), pFrontObject2->getY(), nGridDelay* i);
				
				stLionBack lionback = { 0 };
				lionback.dwFeature = pFrontObject->GetFeather();
				lionback.dwStatus = pFrontObject->GetStatus();
				lionback.wEndFlag = 1;
				int nBackDir = (dir + 4) % 8;
				tx = pFrontObject->getX(), ty = pFrontObject->getY();
				pFrontObject->SendAroundMsg(pFrontObject->GetId(), 5, tx, ty, nBackDir, &lionback, sizeof(lionback));
				pFrontObject->SendMsg(pFrontObject->GetId(), 5, tx, ty, nBackDir, &lionback, sizeof(lionback));
				pFrontObject->SetDirection((e_direction)nBackDir);
				if (pFrontObject2) pFrontObject2->SetDirection((e_direction)nBackDir);
				const int nDamage1 = skillData.value1; // 第一个伤害
				const int nDamage2 = skillData.value2; // 第二个伤害
				const int nDamage = Getrand(5);
				if (i != nCount)
				{
					pFrontObject->BeAttack(this, nDamage1 + nDamage, DT_DIRECT);
					if (pFrontObject2)
						pFrontObject2->BeAttack(this, nDamage2 + nDamage, DT_DIRECT);
				}
			}
			else
			{
				bTrain = FALSE;
				bShowEffect = FALSE;
			}
		}
		break;
		case 45: // 诅咒术
		{
			bTrain = FALSE;
			if (!IsProperTarget(pObject))
				break;
			int nPower = GetRangeRand(GetPropValue(PI_MINSC), GetPropValue(PI_MAXSC));
			const int value1 = skillData.value1; // 诅咒术(女)蓝咒-降魔防
			const int value2 = skillData.value2 + GetTianXuannDamage(); // 诅咒术(男)红咒-减魔法
			const int value3 = skillData.value3; // 中毒最长时间
			nPower += value3;
#ifdef _DEBUG
			SaySystem("诅咒术 时间 %d 秒", nPower);
#endif
			bTrain = AddProcess(EP_STRAW, nTarget, needitem, MAKEDWORD2W(value2, value1), nPower, 800);
		}
		break;
		case 46: // 赶尸
		{
			bTrain = FALSE;
			if (pObject != this && pObject->IsDeath() && pObject->GetType() == OBJ_MONSTER)
			{
				if (m_iPetCount > 0)
				{
					bShowEffect = FALSE;
					break;
				}
				char szClassName[100];
				DWORD nLevel = GetTianXuanCallLevel(btLevel);
				sprintf(szClassName, "%s%u", pMagic->pClass->szSpecial, nLevel);
				bTrain = SummonPet(szClassName, 1, pObject->getX(), pObject->getY());
				if (bTrain)
				{
					this->m_wPetSkill = wMagicId;
					CMonsterEx* pMonster = (CMonsterEx*)pObject;
					CGameWorld::GetInstance()->RemoveMapObject(pObject);
					CMonsterManagerEx::GetInstance()->DeleteMonster(pMonster);
				}
			}
			else
			{
				bShowEffect = FALSE;
				break;
			}
		}
		break;
		case 47: // 替身法符
		{
			bTrain = FALSE;
			if (IsSystemFlagSeted(SF_FAKETARGET))
			{
				bShowEffect = FALSE;
				break;
			}
			CMonsterEx* pReplacer = SummonMonster(pMagic->pClass->szSpecial, 0, x, y);
			if (pReplacer == nullptr)
			{
				bShowEffect = FALSE;
				break;
			}
			else
			{
				const int value1 = skillData.value1; // 替身持续时间
				const int value2 = skillData.value2; // 最大可吸引怪物数量

				DWORD dwTime = CalcLucky();
				dwTime += value1;
				pReplacer->SetSystemFlag(SF_FAKETARGET, TRUE, 0, dwTime * 1000);
				SetSystemFlag(SF_FAKETARGET, TRUE, 0, dwTime * 1000);
				UINT nCount = 0;
				xListHost<VISIBLE_OBJECT>::xListNode* pNode = nullptr;
				if (m_xVisibleObjectList.getCount())
				{
					pNode = m_xVisibleObjectList.getHead();
					while (pNode)
					{
						if (pNode->getObject()->pObject->GetType() == OBJ_MONSTER &&
							((CMonsterEx*)pNode->getObject()->pObject)->GetTarget() == this)
						{
							((CMonsterEx*)pNode->getObject()->pObject)->SetTarget(pReplacer);
							nCount++;
						}
						if (nCount > value2) break;
						pNode = pNode->getNext();
					}
				}
				if (nCount > 0) bTrain = TRUE;				
			}
		}
		break;
		case 48: // 灵魂墙
		{
			if (IsProperTarget(pObject) &&
				(pObject->GetType() == OBJ_MONSTER || pObject->GetType() == OBJ_PET) &&
				!pObject->IsSystemFlagSeted(SF_SOULWALL) &&
				pObject->GetPropValue(PI_LEVEL) <= nLevel)
			{
				const int value1 = skillData.value1; // 强度
				const int value2 = skillData.value2; // 技能成功几率

				MonsterClass* pDesc = ((CMonsterEx*)pObject)->GetDesc();
				if ((pDesc->sprop.pFlag & SF_ANTSOULWALL) != 0 &&
					Getrand(value2) < pDesc->sprop.AntSoulWall)
				{
					SaySystem("目标表示不想理踩了你的咒语,并且默默地走开了");
					bSuccess = FALSE;
				}
				else
				{
					int nTime = CalcLucky();
					nTime += value1;
#ifdef _DEBUG
					SaySystem("灵魂墙 时间 %d 秒", nTime);
#endif
					pObject->SetSystemFlag(SF_SOULWALL, TRUE, pMagic->dwColor, nTime * 1000);
				}
			}
			else
				bSuccess = FALSE;
		}
		break;
		case 64: // 神光术
		{
			bTrain = FALSE;
			if (m_pMap == nullptr)
				break;
			const int value1 = skillData.value1; // 解除几率
			if (Getrand(100) < value1)
			{
				const int value2 = skillData.value2; // 解诅咒范围
				const int value3 = skillData.value3; // 最多可解数量
				const int value4 = skillData.value4; // 免诅咒时间(秒)
				const int value5 = skillData.value5; // 免麻痹时间(秒)
				std::vector<BYTE> btCure = { SI_PALSY, SI_STRAWMAN, SI_STRAWWOMAN }; // 解诅咒类型
				std::vector<CAliveObject*> retTargets;  // 存储解诅咒的目标列表
				bTrain = m_pMap->CureBagStatusAround(this, x, y, value2, btCure, value3, &retTargets);
				if (bTrain) SaySystem("解除诅咒成功!");
				for (const auto& target : retTargets)
				{
					target->SetSkillTime(wMagicId, value4 * 1000);
					target->SetStatusTime(SI_PALSY, value5 * 1000);
				}
			}
		}
		break;
		case 65: // 解毒术
		{
			bTrain = FALSE;
			if (m_pMap == nullptr)
				break;
			const int value1 = skillData.value1; // 解除几率
			if (Getrand(100) < value1)
			{
				const int value2 = skillData.value2; // 解毒范围
				const int value3 = skillData.value3; // 最多可解数量
				const int value4 = skillData.value4; // 免中毒时间(秒)
				std::vector<BYTE> btCure = { SI_REDPOISON, SI_GREENPOISON }; // 解毒类型
				std::vector<CAliveObject*> retTargets;  // 存储解毒的目标列表
				bTrain = m_pMap->CureBagStatusAround(this, x, y, value2, btCure, value3, &retTargets);
				if (bTrain) SaySystem("解毒成功!");
				for (const auto& target : retTargets)
				{
					target->SetSkillTime(wMagicId, value4 * 1000);
				}
			}
		}
		break;
		case 66: // 强化骷髅
		{
			bTrain = FALSE;
			char szClassName[100];
			DWORD nLevel = GetTianXuanCallLevel(btLevel);
			sprintf(szClassName, "%s%u", pMagic->pClass->szSpecial, nLevel);
			if (m_iPetCount > 1)
			{
				bShowEffect = FALSE;
				break;
			}
			int count = 2;
			if (m_iPetCount == 1)
			{
				if (m_wPetSkill != wMagicId)
				{
					bShowEffect = FALSE;
					break;
				}
				else
					count = 1;
			}
			m_wPetSkill = wMagicId;
			for (int i = 0; i < count; i++)
			{
				bTrain = SummonPet(szClassName);
			}
		}
		break;
		case 72: // 心灵召唤
		{
			if (m_iPetCount == 0) 
			{
				bTrain = FALSE;
				break;
			}
			bTrain = TRUE;
			const int value1 = skillData.value1; // 宝宝血量加到百分比
			pMagic->pClass->wDelay = skillData.value2 * 1000; // 使用间隔时间(秒)
			for (int i = 0; i < m_iPetCount; i++)
			{
				if (m_pPets[i]->IsDeath()) continue;
				POINT pt;
				if (m_pMap->GetValidPoint(getX(), getY(), &pt, 1) > 0)
				{
					m_pPets[i]->SetTarget(nullptr);
					m_pPets[i]->FlyTo(GetMap(), pt.x, pt.y); // 宝宝传送
					WORD wMaxHp = m_pPets[i]->GetPropValue(PI_MAXHP);
					WORD wCurHp = m_pPets[i]->GetPropValue(PI_CURHP);
					wMaxHp = ROUND(wMaxHp * value1 / 100);
					if (wCurHp < wMaxHp)
					{
						m_pPets[i]->AddPropValue(PI_CURHP, (wMaxHp - wCurHp));
						m_pPets[i]->SendHpMpChanged(-(wMaxHp - wCurHp)); // 宝宝加血
					}
				}
				if (pObject != this)
				{
					if (IsProperTarget(pObject) || pObject->GetType() != OBJ_NPC) // 设定宝宝攻击目标
					{
						m_pPets[i]->SetTarget(pObject);
					}
				}
			}
		}
		break;
		case 75: // 遁地
		{
			if (m_pMap == nullptr)return FALSE;
			bTrain = FALSE;
			if (m_pMap->IsFlagSeted(MF_NOFLASHMOVE))
			{
				SaySystem("当前地图无法使用遁地!");
				break;
			}
			if (m_pMap->IsBlocked(x, y))
			{
				SaySystem("坐标无法到达, 遁地失败!");
				break;
			}
			const int maxDis = skillData.value1; // 飞行最大距离
			if (DISTANCE(x, y, nSrcX, nSrcY) > maxDis)
			{
				SaySystem("距离太远, 遁地使用失败!");
				break;
			}
			const int probability = skillData.value2; // 成功几率
			if (Getrand(100) >= probability)
			{
				SaySystem("遁地使用失败!");
				break;
			}
			if (SpellFly(x, y, wMagicId))
				bTrain = TRUE;
			else
				bSuccess = FALSE;
		}
		break;
		default:
		{
			if (pMagic->pClass->dwFlag & MAGICFLAG_ACTIVED)
			{
				pMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
				if (pMagic->pClass->szSpecial[0] != 0)
				{
					char szMsg[200];
					if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
						_snprintf(szMsg, sizeof(szMsg) - 1, "#+%s!", pMagic->pClass->szSpecial);
					else
						_snprintf(szMsg, sizeof(szMsg) - 1, "#+U%s!", pMagic->pClass->szSpecial);
					if (m_pClientObj)m_pClientObj->PostMsg(szMsg, (int)strlen(szMsg));
				}
				if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
					SaySystemAttrib(CC_GREENS, "开启%s", szName);
				else
					SaySystemAttrib(CC_GREENS, "关闭%s", szName);
				return TRUE;
			}
		}
		break;
		}
	}

	if (bSuccess)
	{
		SetCurAttackType(DT_MAGIC);
		SetPreActionType(AT_SPELLSKILL);
		pMagic->useTimer.Savetime();
		if (bCostMp)// 当有魔法值消耗时, 处理魔法值
		{
			CostMp(dwSpellMpCost);
			SendHpMpChanged();
		}
		if (needcount > 0)
		{
			switch (needitem)
			{
			case SNI_CHARM: // 道符
				TakeMaterial(ISM_CHARM, 5, 0, pMagic->pClass->wCharmCount);
				break;
			case SNI_GREENPOISON: // 绿毒
				TakeMaterial(ISM_POISON, 1, 0, pMagic->pClass->wGreenPoisonCount);
				boPoison = FALSE;
				break;
			case SNI_REDPOISON: // 红毒
				TakeMaterial(ISM_POISON, 2, 0, pMagic->pClass->wRedPoisonCount);
				boPoison = TRUE;
				break;
			case SNI_STRAWMAN: // 诅咒木偶男
				TakeMaterial(ISM_POISON, 1, 1, pMagic->pClass->wStrawManCount);
				boPoison = FALSE;
				break;
			case SNI_STRAWWOMAN: // 诅咒木偶女
				TakeMaterial(ISM_POISON, 2, 1, pMagic->pClass->wStrawWomanCount);
				boPoison = TRUE;
				break;
			}
		}
		if (bShowEffect)
		{
			char szmagicid[20];
			sprintf(szmagicid, "%u", wMagicId);
			SendAroundMsg(GetId(), SM_SPELLSKILL, 0, 0, wMagicId, szmagicid);
			DWORD dwArray[2] = { nTarget, pMagic->dwColor };
			WORD wEffect = (((int)pMagic->pClass->btEffectType) << 8) | pMagic->pClass->btEffectValue;
			SendAroundMsg(GetId(), SM_PLAYSKILLEFFECT, x, y, wEffect, (LPVOID)dwArray, sizeof(dwArray));
			SendMsg(GetId(), SM_PLAYSKILLEFFECT, x, y, wEffect, (LPVOID)dwArray, sizeof(dwArray));
		}
		if (bTrain) TrainMagic(pMagic);
		return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::SpellFly(int x, int y, WORD wMagicId)
{
	constexpr BYTE bt1Value = 0x0c;
	constexpr BYTE btColorValue = 0xff;
	constexpr BYTE btSm2Value = 0;

	const int nSrcX = getX();
	const int nSrcY = getY();
	const DWORD dwFeature = GetFeather();
	const DWORD dwStatus = GetStatus();
	const WORD wCurHp = static_cast<WORD>(GetPropValue(PI_CURHP));
	const WORD wMaxHp = static_cast<WORD>(GetPropValue(PI_MAXHP));
	const WORD w3 = GetSex() << 8 | GetDirection();
	const char* pszName = GetName();
	const size_t nNameLen = strlen(pszName);
	const size_t nMsgSize = sizeof(tag_batfly_header) + nNameLen;

	tag_batfly batfly;
	batfly.header.wX = nSrcX;
	batfly.header.wY = nSrcY;
	batfly.header.bt1 = bt1Value;
	batfly.header.btColor = btColorValue;
	batfly.header.btSm = wMagicId;
	batfly.header.btSm2 = btSm2Value;
	batfly.header.dwFeature = dwFeature;
	batfly.header.dwStatus = dwStatus;
	batfly.header.wCurHp = wCurHp;
	batfly.header.wMaxHp = wMaxHp;
	strcpy(batfly.szName, pszName);
	
	int nDis = DISTANCE(x, y, nSrcX, nSrcY);
	int nBaseTime = 60;
	if (wMagicId == 35) nBaseTime = 6;
	AddProcess(EP_SKILLFLY, x, y, 0, 0, nBaseTime * nDis + 500);
	SendAroundMsg(GetId(), 0xaa, x, y, w3, &batfly, nMsgSize);
	SendMsg(GetId(), 0xaa, x, y, w3, &batfly, nMsgSize);
	return TRUE;
}

BOOL CHumanPlayer::IsDefenceEffectiveObject(CAliveObject* pObject)
{
	return TRUE;
}

BOOL CHumanPlayer::SpecialHit(int dir, WORD wSkillId)
{
	// 动作检查（烈火和雷霆剑不受限制）
	if (wSkillId != 44 && wSkillId != 26)
	{
		if (!CanDoAction(AT_SPECIALHIT)) return FALSE;
	}
	// 获取技能信息
	USERMAGIC* pMagic = this->GetMagic(wSkillId);
	if (pMagic == nullptr) return FALSE;
	const char* szName = pMagic->pClass->szName;
	CLogicMap* pMap = GetMap();
	std::vector<std::string> dwExtraParams;
	// 地图检查：禁止使用技能
	if (pMap && pMap->IsFlagSeted(MF_NOSKILL, g_dwInterFlag, dwExtraParams))
	{
		for (int i = 0; i < dwExtraParams.size(); i++)
		{
			if (szName == dwExtraParams[i])
			{
				SaySystem("此地图禁止使用 %s 技能!", szName);
				return FALSE;
			}
		}
	}
	// 状态检查：变身、骑马、摆摊、麻痹、冰冻
	if (this->IsSystemFlagSeted(SF_TRANSFORMED) || m_bRideHorse || GetActionType() == AT_PRIVATESHOP ||
		this->IsStatusSet(SI_PALSY) || this->IsSystemFlagSeted(SF_FROZEN))
		return FALSE;
	// 检查武器
	if (GetEquipment(_U_WEAPON) == nullptr)
	{
		SaySystem("本次 %s 技能必须佩武器使用!", szName);
		return FALSE;
	}
	// 魔法值检查
	const int nMpDamage = MAX(0, pMagic->pClass->GetSpellPoint(pMagic->magic.btLevel));
	if (nMpDamage > GetPropValue(PI_CURMP))
	{
		SaySystem("你的魔法值不足施放 %s 技能!", szName);
		return FALSE;
	}
	// 技能激活检查（刺杀剑术除外）
	if ((pMagic->dwFlag & USERMAGICFLAG_ACTIVED) == 0 && wSkillId != 12)
	{
		SaySystem("您的 %s 技能尚未激活!", szName);
		return FALSE;
	}
	// 获取攻击目标
	int tx = getX(), ty = getY();
	GETNEXTPOS(tx, ty, dir);
	CAliveObject* pObject = m_pMap->FindTarget(this, tx, ty, FALSE);
	// 目标检查
	BOOL SendBUF = FALSE;
	if (pObject)
		SendBUF = IsProperTarget(pObject);
	// 初始化攻击参数
	WORD wMsg = 0x0e;
	DWORD dwFlag = 0;
	DWORD dwType = DT_PHYSICS;
	BOOL bSaveSkillTime = TRUE;
	// 计算基础伤害
	int nPower = getskillpower(wSkillId);
	nPower += CalcLucky();
	nPower = MAX(0, nPower);
	// 统一计算百分比伤害的辅助函数
	auto CalculateBonusDamage = [this](int basePower, WORD skillId) -> int {
		const Magic& magicskill = CMagicManager::GetInstance()->GetMagic(skillId);
		const int pow = magicskill.skills[this->GetMagic(skillId)->magic.btLevel].value3;
		const double powerMultiplier = pow / 100.0;
		return static_cast<int>(basePower * powerMultiplier);
	};
	switch (wSkillId)
	{
	case 7: // 攻杀剑法
	{
		wMsg = 0x12;
		if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
			pMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
		if (SendBUF)
			pObject->AddProcess(EP_BEATTACKED, nPower, GetId(), dwType, dwFlag, 200);
	}
	break;
	case 12: // 刺杀剑术
	{
		wMsg = 0x13;
		int px = getX(), py = getY();
		GETNEXTPOS(px, py, dir);
		CAliveObject* pTarget1 = m_pMap->FindTarget(this, px, py);
		GETNEXTPOS(px, py, dir);
		CAliveObject* pTarget2 = m_pMap->FindTarget(this, px, py);
		int nSecPower = CalculateBonusDamage(nPower, wSkillId);
		SendBUF = TRUE;
		if (pTarget1)
			pTarget1->AddProcess(EP_BEATTACKED, nPower, GetId(), dwType, 0, 100);
		if (pTarget2)
			pTarget2->AddProcess(EP_BEATTACKED, nSecPower, GetId(), dwType, 0, 200);
	}
	break;
	case 25: // 半月弯刀
	{
		wMsg = 0x18;
		constexpr int widtattack[4] = { 2, 1, 0, 7 };
		const int nSecPower = CalculateBonusDamage(nPower, wSkillId);
		int nCalcPower = 0;
		SendBUF = TRUE;
		const int nX = getX();
		const int nY = getY();
		for (int i = 0; i < 4; i++)
		{
			const int nAttackDir = (dir + widtattack[i]) % 8;
			int px = nX, py = nY;
			GETNEXTPOS(px, py, nAttackDir);
			CAliveObject* pTarget = m_pMap->FindTarget(this, px, py);
			if (pTarget && !pTarget->IsDeath())
			{
				if ( i == 2)
					nCalcPower = nPower;
				else
					nCalcPower = nSecPower * (i + 1) / 4;
				pTarget->AddProcess(EP_BEATTACKED, nCalcPower, GetId(), dwType, 0, 200);
			}
		}
	}
	break;
	case 26: // 烈火剑法
	{
		wMsg = 0x8;
		if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
			pMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
		if (m_pClientObj)
			m_pClientObj->PostMsg("#+UFIR!", 7);
		nPower += CalculateBonusDamage(nPower, wSkillId);
		dwFlag = DF_TARGETEFFECT | TE_FIRE;
		bSaveSkillTime = FALSE;
		if (SendBUF)
			pObject->AddProcess(EP_BEATTACKED, nPower, GetId(), dwType, dwFlag, 200);
	}
	break;
	case 40: // 残影刀法
	{
		wMsg = 0x19;
		if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
			pMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
		if (SendBUF)
			pObject->AddProcess(EP_BEATTACKED, nPower, GetId(), dwType, dwFlag, 200);
	}
	break;
	case 41: // 血影刀法
	{
		wMsg = 0x19;
		if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
			pMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
		if (SendBUF)
			pObject->AddProcess(EP_BEATTACKED, nPower, GetId(), dwType, dwFlag, 200);
	}
	break;
	case 43: // 抱月刀
	{
		wMsg = 0x18;
		constexpr int widtattack[8] = { 5, 6, 7, 0, 1, 2, 3, 4 };
		const int nSecPower = CalculateBonusDamage(nPower, wSkillId);
		int nCalcPower = 0;
		SendBUF = TRUE;
		const int nX = getX();
		const int nY = getY();
		for (int i = 0; i < 8; i++)
		{
			int px = nX, py = nY;
			const int nAttackDir = (dir + widtattack[i]) % 8;
			GETNEXTPOS(px, py, nAttackDir);
			CAliveObject* pTarget = m_pMap->FindTarget(this, px, py);
			if (pTarget && !pTarget->IsDeath())
			{
				if (i < 3)
					nCalcPower = nSecPower * (i + 1) / 8;
				else if (i == 3 || i == 4)
					nCalcPower = nPower;
				else if (i > 4)
					nCalcPower = nSecPower * (8 - i) / 8;
				pTarget->AddProcess(EP_BEATTACKED, nCalcPower, GetId(), dwType, 0, 200);
			}
		}
	}
	break;
	case 44: // 雷霆剑
	{
		wMsg = 0x19;
		if (pMagic->dwFlag & USERMAGICFLAG_ACTIVED)
			pMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
		if (m_pClientObj)
			m_pClientObj->PostMsg("#+UTHU!", 7);
		nPower += CalculateBonusDamage(nPower, wSkillId);
		dwFlag = DF_TARGETEFFECT | 2;
		dwType = DT_MAGIC;
		bSaveSkillTime = FALSE;
		if (SendBUF)
			pObject->AddProcess(EP_BEATTACKED, nPower, GetId(), dwType, dwFlag, 200);
	}
	break;
	case 59: // 破击剑法
	{
		wMsg = 0x19;
		dwFlag = DF_TARGETEFFECT | TE_POJI;
		const Magic& magicskill = CMagicManager::GetInstance()->GetMagic(wSkillId);
		const int probability = magicskill.skills[pMagic->magic.btLevel].value3;
		if (pObject && pObject->GetType() == OBJ_PLAYER && Getrand(100) < probability)
		{
			if (pObject->IsStatusSet(SI_DEFENCEUP))
				pObject->ClrStatus(SI_DEFENCEUP);
			if (pObject->IsStatusSet(SI_MAGDEFENCEUP))
				pObject->ClrStatus(SI_MAGDEFENCEUP);
			if (pObject->IsStatusSet(SI_HUSHENZHENQI))
				pObject->ClrStatus(SI_HUSHENZHENQI);
			dwType = MAKEWORD(dwType, DRT_POJI);
		}
		if (SendBUF)
			pObject->AddProcess(EP_BEATTACKED, nPower, GetId(), dwType, dwFlag, 200);
	}
	break;
	case 60: // 破盾斩
	{
		wMsg = 0x19;
		dwFlag = DF_TARGETEFFECT | TE_PODUN;
		const Magic& magicskill = CMagicManager::GetInstance()->GetMagic(wSkillId);
		const int probability = magicskill.skills[pMagic->magic.btLevel].value3;
		if (pObject && pObject->GetType() == OBJ_PLAYER && Getrand(100) < probability)
		{
			if (pObject->IsStatusSet(SI_BUBBLEDEFENCEUP))
				pObject->ClrStatus(SI_BUBBLEDEFENCEUP);
			if (pObject->IsSystemFlagSeted(SF_WINDSHIELD))
				pObject->SetSystemFlag(SF_WINDSHIELD, FALSE);
			if (pObject->IsSystemFlagSeted(SF_STRONGSHIELD))
				pObject->SetSystemFlag(SF_STRONGSHIELD, FALSE);
			dwType = MAKEWORD(dwType, DRT_PODUN);
		}
		if (SendBUF)
			pObject->AddProcess(EP_BEATTACKED, nPower, GetId(), dwType, dwFlag, 200);
	}
	break;
	}
	// 消耗魔法值
	if (nMpDamage > 0)
	{
		CostMp(nMpDamage);
		SendHpMpChanged();
	}
	SetPreActionType(AT_SPECIALHIT);
	// 发送技能消息
	DWORD dwColor = pMagic->dwColor;
	WORD w3 = MAKEWORD(dir, pMagic->magic.btLevel);
	if (wMsg == 0x13)//刺杀
		SendMsg(GetId(), wMsg, getX(), getY(), w3, &dwColor, sizeof(DWORD));
	if (wMsg == 0x19)
	{
		WORD wView[3] = { wSkillId, static_cast<WORD>(dwColor & 0xffff), static_cast<WORD>((dwColor & 0xffff0000) >> 16)};
		SendAroundMsg(GetId(), wMsg, getX(), getY(), w3, wView, sizeof(WORD) * 3);
	}
	else
	{
		SendAroundMsg(GetId(), wMsg, getX(), getY(), w3, &dwColor, sizeof(DWORD));
	}
	// 武器耐久损耗
	const int nWeaponDamage = Getrand(5) + 2 - GetPropValue(PI_HARD);
	if (nWeaponDamage > 0)
		DamageDura(_U_WEAPON, nWeaponDamage);
	// 保存技能时间
	if (bSaveSkillTime)
	{
		m_tmrSpecialAttackSkill.Savetime();
		m_tmrAttack.Savetime();
	}
	if (SendBUF)
	{
		TrainMagic(pMagic);
		UpdateAutoMagic();
	}
	SetCurAttackType(DT_MAGIC);
	return TRUE;
}

VOID CHumanPlayer::OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type)
{
	int nDuraDamage = nDamage;
	HushenBuffdamage += nDamage;
	int dressrate = CGameWorld::GetInstance()->GetVar(EVI_DRESSDAMAGERATE);
	int defencerate = CGameWorld::GetInstance()->GetVar(EVI_DEFENCEDAMAGERATE);
	int jewrate = CGameWorld::GetInstance()->GetVar(EVI_JEWELRYDAMAGERATE);

	if (dressrate > 0)
		DamageDura(_U_DRESS, (nDuraDamage + dressrate - 1) / dressrate);

	int duradamage = 0;
	if (defencerate > 0)
	{
		duradamage = (nDuraDamage + defencerate - 1) / defencerate;
		DamageDura(_U_SHOE, duradamage);
		DamageDura(_U_BELT, duradamage);
		DamageDura(_U_HELMET, duradamage);
	}

	if (jewrate > 0)
	{
		duradamage = (nDuraDamage + jewrate - 1) / jewrate;
		DamageDura(_U_RINGL, duradamage);
		DamageDura(_U_RINGR, duradamage);
		ITEM* pArmRingL = GetEquipment(_U_ARMRINGL);
		if (pArmRingL && (pArmRingL->baseitem.btStdMode == 24 || pArmRingL->baseitem.btStdMode == 26))
			DamageDura(_U_ARMRINGL, duradamage);
		DamageDura(_U_ARMRINGR, duradamage);
		DamageDura(_U_NECKLACE, duradamage);
		ITEM* pCharm = GetEquipment(_U_CHARM);
		if (pCharm && pCharm->baseitem.btStdMode == 30)
			DamageDura(_U_CHARM, duradamage);
	}

	if (pAttacker)
	{
		SetPetTarget(pAttacker);
		if (IsStatusSet(SI_FENGHUOLUN))
		{
			DWORD dwDamage = GetStatusParam(SI_FENGHUOLUN);
			pAttacker->BeAttack(this, dwDamage, DT_MAGIC, DF_TARGETEFFECT | TE_FIREWHEEL);
			ClrStatus(SI_FENGHUOLUN);
		}
	}
	//计算护身真气, 金刚护体
	if (this->m_xAbilityShellRef.pShell)
	{
		//这个变量用来确定发送29
		BOOL  ISsendBuf = FALSE;
		if (HushenBuffdamage > 50)
		{
			HushenBuffdamage -= 50;
			ISsendBuf = TRUE;
			this->m_xAbilityShellRef.wCurHp--;
		}
		if (ISsendBuf)
		{
			SendAroundMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[hushenleve]);
			SendMsg(GetId(), 0x1d, this->m_xAbilityShellRef.wCurHp, this->m_xAbilityShellRef.pShell->wMaxHp, hushenbuff[hushenleve]);
		}
		if (this->m_xAbilityShellRef.wCurHp == 0)
		{
			this->m_xAbilityShellRef.pShell = nullptr;
			//去掉SHELL影响
			HushenBuffdamage = 0;
			if (CAliveObject::IsStatusSet(SI_HUSHENZHENQI))
			{
				CAliveObject::ClrStatus(SI_HUSHENZHENQI);
				UpdateProp();
				//UpdateSubProp();
			}
			if (CAliveObject::IsSystemFlagSeted(SF_STRONGSHIELD))
			{
				CAliveObject::SetSystemFlag(SF_STRONGSHIELD, FALSE);
				UpdateProp();
				UpdateSubProp();
			}
		}
	}
	CAliveObject::OnDamage(pAttacker, nDamage, type);
}

BOOL CHumanPlayer::CanBePushed(CAliveObject* pAttacker)
{
	if (GetActionType() == AT_PRIVATESHOP)return FALSE;
	if (CAliveObject::CanBePushed(pAttacker))return TRUE;
	if (pAttacker->GetType() == OBJ_PLAYER)
		return (((CHumanPlayer*)pAttacker)->m_iCurrentTitleIndex > m_iCurrentTitleIndex);
	return FALSE;
}

DWORD CHumanPlayer::GetLevelupExp()
{
	if (this->m_pHumanDataDesc)
		return this->m_pHumanDataDesc->dwLevelupExp;
	return 0;
}

VOID CHumanPlayer::OnKillTarget(CAliveObject* pTarget)
{
	if (pTarget && pTarget->GetType() == OBJ_MONSTER && GetPro() == JOB_MAG)
	{
		USERMAGIC* pMagic = GetMagic(76);
		if (pMagic && Getrand(100) < (20 + pMagic->magic.btLevel * 20))
		{
			int level = pTarget->GetPropValue(PI_LEVEL);
			AddMp(level);
			SaySystem( "怪物的魂魄化作魔力注入你的体内, 吸收到%u点魔法值", level );
			TrainMagic(pMagic);
		}
	}
}

BOOL CHumanPlayer::RemoveMagic(const char* pszMagic)
{
	USERMAGIC* pMagic = m_pMagic;
	USERMAGIC* p = m_pMagic;
	MAGICCLASS* pClass = CMagicManager::GetInstance()->GetClassByName(pszMagic);
	if (pClass == nullptr)return FALSE;
	return RemoveMagic(pClass->id);
}

BOOL CHumanPlayer::RemoveMagic(UINT nMagicId)
{
	USERMAGIC* pMagic = m_pMagic;
	USERMAGIC* p = m_pMagic;
	while (pMagic)
	{
		if (pMagic->pClass->id == nMagicId)
		{
			if (pMagic == m_pMagic)
				m_pMagic = pMagic->pNext;
			else
				p->pNext = pMagic->pNext;
			CGameWorld::GetInstance()->FreeUserMagic(pMagic);
			SendMsg(nMagicId, 0xd4, 0, 0, 0);
			CMagicManager::GetInstance()->DeleteMagicFromDB(GetDBId(), nMagicId);
			return TRUE;
		}
		p = pMagic;
		pMagic = pMagic->pNext;
	}
	return FALSE;
}
