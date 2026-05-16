#include "StdAfx.h"
#include "monsterex.h"
#include "MonsterAttack.h"
#include "logicmapmgr.h"
#include "logicmap.h"
#include "MonsterManagerEx.h"
#include "MonsterGenManager.h"
#include "FireNimoBurnEvent.h"
#include "ZombiHoleEvent.h"
#include "SculpEiceEvent.h"
#include "server.h"
#include "gameworld.h"
#include "monitemsmgr.h"
#include "humanplayer.h"
#include "DownItemMgr.h"
#include "itemmanager.h"
#include "guildex.h"
#include "eventmanager.h"
#include "visibleevent.h"
#include "systemscript.h"
#include "autoscriptmanager.h"

xObjectPool<StringCacheNode> CMonsterEx::m_xStringCachePool;
CMonsterEx::CMonsterEx(void) : m_xUpdateNode(this)
{
	m_pGen = nullptr;
	m_pScriptPage = nullptr;
	m_xQProcess.create(CGameWorld::GetInstance()->GetMonsterQueue());
	Clean();
}

CMonsterEx::~CMonsterEx(void)
{
}

BOOL CMonsterEx::Init(MonsterClass* pDesc, int mapid, int x, int y, MONSTERGEN* pGen, BOOL bGotoPoint, int gotox, int gotoy)
{
	if (pDesc == nullptr) return FALSE;
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
	if (pMap == nullptr) return FALSE;

	int mapWidth = pMap->GetWidth();
	int mapHeight = pMap->GetHeight();
	x = (x < 0) ? 0 : (x >= mapWidth) ? mapWidth - 1 : x;
	y = (y < 0) ? 0 : (y >= mapHeight) ? mapHeight - 1 : y;

	if (pMap->IsPhysicsBlocked(x, y)) return FALSE;

	m_pGen = pGen;
	setXY(x, y);
	SetMapId(mapid);
	SetSType(0x13);

	if (!SetDesc(pDesc)) return FALSE;
	m_bFirstEnterMap = TRUE;

	m_wCurHp = m_pDesc->prop.hp;
	m_wCurMp = m_pDesc->prop.mp;

	SetGotoTarget(bGotoPoint, gotox, gotoy);

	if (pGen && pGen->pszScriptPage)
	{
		m_pScriptPage = m_xStringCachePool.newObject();
		o_strncpy(m_pScriptPage->szString, pGen->pszScriptPage, 250);
	}
	if (pGen && pGen->range > 1000)
		m_boSpecialGen = TRUE;
	return CAiObjectEx::Init(x, y);
}

VOID CMonsterEx::Clean()
{
	// 清理动态分配的资源
	if (m_pScriptPage != nullptr)
	{
		m_xStringCachePool.deleteObject(m_pScriptPage);
		m_pScriptPage = nullptr;
	}
	
	m_xAttackObj.Clear();
	
	ClearGen();
	m_pDesc = nullptr;
	
	// 清理其他成员变量
	m_pCurFocusItem = nullptr;
	m_IdleTimer.Savetime();
	m_betrayTimer.Savetime();
	m_bSetOwner = FALSE;
	m_dwExp = 0;
	m_wCurHp = 0;
	m_wCurMp = 0;
	m_dwKillCount = 0;
	m_btRevivalCount = 0;
	m_fCuted = FALSE;
	m_bGotoPoint = FALSE;
	m_bFirstEnterMap = FALSE;
	m_boSpecialGen = FALSE;
	m_wGotoX = 0;
	m_wGotoY = 0;
	strcpy(m_szOriginalName, "");
	CAiObjectEx::Clean();
}

const char* CMonsterEx::GetViewName()
{
	return m_szLongName;
}

VOID CMonsterEx::OnDeath(DWORD dwKiller)
{
	BOOL bDropItem = TRUE;
	CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(dwKiller);

	//	删除和主人的联系
	CAliveObject* pOwner = nullptr;
	CHumanPlayer* pKiller = nullptr;
	if ((pOwner = GetOwner()))
	{
		pOwner->OnPetDie(this, pObject);
		pOwner->DelPet(this);
		//	通知主人被杀了
		if (pOwner->GetType() == OBJ_PLAYER && pOwner != pObject)
		{
			if (pObject)
				pOwner->SaySystemAttrib(CC_PETDIE, "您的 %s 被 %s 杀了!", GetName(), pObject->GetName());
			else
				pOwner->SaySystemAttrib(CC_PETDIE, "您的 %s 被杀了!", GetName());
		}
		bDropItem = FALSE;
	}
	if (pObject)
	{
		if (pObject->GetType() == OBJ_PLAYER)
			pKiller = ((CHumanPlayer*)pObject);
		else if (pObject->GetType() == OBJ_PET)
		{
			if (pObject->GetOwner() && pObject->GetOwner()->GetType() == OBJ_PLAYER)
				pKiller = ((CHumanPlayer*)pObject->GetOwner());
		}
		if (GetType() == OBJ_MONSTER || GetType() == OBJ_PET)
			((CMonsterEx*)pObject)->m_dwKillCount++;
		pObject->AddExp(GetPropValue(PI_EXP), GetPropValue(PI_LEVEL), GetId());
		if (pKiller)
		{
			pKiller->OnKillTarget(this);
			if (m_pScriptPage) //执行怪物脚本
				DoScript(pKiller, m_pScriptPage->szString);
		}
		else pObject->OnKillTarget(this);
	}
	if (pKiller == nullptr) pKiller = CAutoScriptManager::GetInstance()->GetScriptTarget();
	//死亡脚本
	if (m_pDesc == nullptr) return;
	if (m_pDesc->pszDeathScript) DoScript(pKiller, m_pDesc->pszDeathScript);
	//所有怪都会触发
	DoScript(pKiller, "MonsterEnv.OnDeath");
	//掉落物品
	if (bDropItem)
	{
		if (m_pDesc->sprop.pFlag & SF_BODYITEM) bDropItem = FALSE;
		if (m_btRevivalCount > 0) bDropItem = FALSE; // 被复活的怪不掉东西
	}
	MonsterDeathEvent subDeathEvent(this, pKiller, bDropItem);
	PUBLISH_EVENT(subDeathEvent);
	//检查变身
	if (GetType() == OBJ_MONSTER || GetType() == OBJ_PET) CheckChangeInto();
	m_bodytimer.Savetime();
}

VOID CMonsterEx::DoProcess(OBJECTPROCESS* pProcess)
{
	if (pProcess == nullptr) return;
	switch (pProcess->ident)
	{
	case EM_RELIVE:
	{
		AddPropValue(PI_CURHP, pProcess->dwParam[0]);
		SetDeath(FALSE);
		AddRevivalCount(1);
		RefreshViewmsg(2);
		CGameWorld::GetInstance()->AddUpdateMonster(this);
	}
	break;
	default:
	{
		CAliveObject::DoProcess(pProcess);
	}
	break;
	}
}

VOID CMonsterEx::Update()
{
	CAliveObject::Update();
	if (m_pDesc == nullptr)
	{
		CGameWorld::GetInstance()->RemoveMapObject(this);
		CMonsterManagerEx::GetInstance()->DeleteMonster(this);
		return;
	}
	CAliveObject* pTarget = GetTarget();
	if (pTarget && !m_bIsShow && (m_pDesc->sprop.pFlag & SF_SHOW) != 0)
	{
		if (DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY()) <= m_pDesc->aiset.ViewDistance)
		{
			SetSystemFlag(SF_HIDED, FALSE); // 不管之前是否隐身, 播出现动画就必须显身.
			if (m_pDesc->sprop.pFlag & SF_SCULPEICE)
				SculpEiceEvent::Create(this, getX(), getY(), 0, 1000, 60000);
			if (m_pDesc->sprop.pFlag & SF_ZOMBIHOLE)
				ZombiHoleEvent::Create(this, getX(), getY(), 0, 1000, 60000);
			DWORD dwView[4] = { GetFeather(), 0, 0, 0 }; //出现动画
			SendAroundMsg(GetId(), 0x14, getX(), getY(), (WORD)GetDirection(), dwView, sizeof(dwView));
			RefreshViewmsg();
			m_bIsShow = TRUE;
			SetAction(AT_SHOW, GetDirection(), getX(), getY(), 1000);
		}
	}
	//与目标距离只有1格了
	if (m_bNoAiDelayAttack && pTarget && DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY()) <= 1)
	{
		if (GetType() == OBJ_MONSTER && GetActionType() != AT_SHOW)//没有延时攻击, 实现玩家经过怪时会及时攻击.
		{
			m_bNoAiDelayAttack = FALSE;
			DWORD dwActionTime = 0;
			if (GetActionType() == AT_WALK)	dwActionTime = 112;
			if (GetActionType() == AT_RUN)	dwActionTime = 42;
			if (dwActionTime != 0)
				SetAction(AT_AIACTION, GetDirection(), getX(), getY(), dwActionTime);
		}
	}
	// 检查宠物叛变
	if (m_bSetOwner && m_betrayTimer.IsTimeOut(m_dwBetray)) 
	{
		MonsterClass* pDesc = CMonsterManagerEx::GetInstance()->GetClassByName(m_szOriginalName);
		SetOwner(nullptr);
		m_bSetOwner = FALSE;
		SetDesc(pDesc);// 重置怪类属性
	}
	//死亡方式
	if (IsDeath())
	{
		switch (m_pDesc->aiset.DieStyle)
		{
		case ADS_NORMAL:
		{
		}
		break;
		case ADS_BOOM:
		{
		}
		break;
		}

		if ((m_pDesc->sprop.pFlag & SF_NOBODY) != 0)
			CMonsterManagerEx::GetInstance()->DeleteMonster(this);

		if (m_pDesc && (m_pDesc->sprop.pFlag & SF_BODYNOTDISAPPEAR) != 0) return;
		if (m_bodytimer.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_BODYTIME) * 1000))
			CMonsterManagerEx::GetInstance()->DeleteMonster(this);
		return;
	}
	if (IsSystemFlagSeted(SF_SOULWALL)) //灵魂墙状态
	{
		Attack((int)GetDirection(), m_pDesc->attackdesc.Delay);
		return;
	}
	if (!m_bPosLocked)//这个里判断是否遇到卡地图,如果遇到则随机走向一个点
	{
		for (int i = 0; i < 8; i++)
		{
			if (AiWalk(i, FALSE)) return;
		}
	}
	if (CheckChangeInto()) return;//检测变身等行为
	if (CAliveObject* pOwner = GetOwner())//获取主人
	{
		if (!pOwner->IsPetsActive())//如果设置宠物休息命令, 攻击目标设置为空
		{
			if (pTarget)
			{
				m_xAttackObj.Clear();
				SetTarget(nullptr);
				pTarget = nullptr;
			}
			return;
		}
		else if (m_pDesc && m_pDesc->petset.Type == APT_RIDE)//如果当前对象是骑乘类
		{
			if (pOwner->GetType() == OBJ_PLAYER && ((CHumanPlayer*)pOwner)->IsHorseRest()) return;
		}
	}
	//如果目标为空,且怪物的Sprop.FLAG 为自动攻击SF_AUTOATTACK
	if (pTarget == nullptr)
	{
		if (m_pDesc->sprop.pFlag & SF_AUTOATTACK)
		{   //搜索视野内范围的怪物做攻击目标
			pTarget = SearchTarget(m_pDesc->aiset.TargetSelect, m_pDesc->aiset.ViewDistance);
			if (pTarget != nullptr) 
			{
				if ((m_pDesc->sprop.pFlag & SF_GROUPSHARETARGET) != 0) // 同组共享目标
					ShareTarget(pTarget);
				else
					SetTarget(pTarget);
			}
		}
	}
	if (pTarget)
	{
		int nDis = DISTANCE(pTarget->getX(), pTarget->getY(), getX(), getY());
		//如果与目标距离超过攻击距离, 就又开始无延时AI攻击
		if (!m_bNoAiDelayAttack && nDis > 1) m_bNoAiDelayAttack = TRUE;
		if ((m_pDesc->sprop.pFlag & SF_LOCKTARGET) == 0)
		{
			int dropDis = m_pDesc->aiset.ViewDistance + CGameWorld::GetInstance()->GetVar(EVI_DROPTARGETDISTANCE); //视觉距离+配置的数量
			//没有合适的目标时和目标超过12格时放弃
			if (!IsProperTarget(pTarget) || !m_refObjTarget.IsValid() || nDis >= dropDis)
			{
				SetTarget(nullptr);
				pTarget = nullptr;
			}
		}
		if (pTarget && m_pMap != pTarget->GetMap())
		{
			m_xAttackObj.Clear();
			SetTarget(nullptr);
			pTarget = nullptr;
		}
	}
	if (!CanDoAction(AT_AIACTION))return;//判断攻击等是否可以执行
	m_IdleTimer.Savetime();
	if (m_pDesc->base.btImage == 0xea) // 黑袍辅助功能
	{
		CMonsterEx* pFriend = (CMonsterEx*)SearchFriendDistance(m_pDesc->attackdesc.AttackDistance);
		if (pFriend)
		{
			CMonsterAttack monAttack(this);
			monAttack.BlackRobeHelper(pFriend);
		}
	}
	//如果目标为空,判断是不是拾取类宠物
	if (pTarget == nullptr && m_pDesc && (m_pDesc->sprop.pFlag & SF_PICKUPITEM))
	{
		//拾取逻辑
		if (m_pCurFocusItem)//宠物的焦点物品
		{
			WORD ix = m_pCurFocusItem->getX();
			WORD iy = m_pCurFocusItem->getY();
			if (m_pCurFocusItem->GetMap() == nullptr || m_pCurFocusItem->GetMap() != m_pMap)
				m_pCurFocusItem = nullptr;
			else if (m_pMap == nullptr)
				m_pCurFocusItem = nullptr;
			else
			{
				if (getX() == ix && getY() == iy) //当坐标和目标做一致时,拾取物品
				{
					CAliveObject* pOwner = GetOwner();
					CDownItemObject* pDownItem = (CDownItemObject*)m_pCurFocusItem;
					ITEM item = pDownItem->GetItem();
					if ((item.dwMakeIndex & 0x80000000) && item.baseitem.btStdMode == 255 && item.baseitem.btShape == 0)	//金钱
					{
						if (CDownItemMgr::GetInstance()->DeleteGroundItem(m_pMap, (CDownItemObject*)m_pCurFocusItem, FALSE))
						{
							m_pCurFocusItem = nullptr;
							DWORD dwGold = item.wCurDura | (item.wMaxDura << 16);
							if (pOwner != nullptr) pOwner->PetAddgold(dwGold);
						}
					}
					else
					{
						if (CDownItemMgr::GetInstance()->DeleteGroundItem(m_pMap, (CDownItemObject*)m_pCurFocusItem, FALSE))
						{
							m_pCurFocusItem = nullptr;
							if (pOwner != nullptr)
								pOwner->PetAddBagItem(item, FALSE, TRUE);
						}
					}
					PetgotoPOwner = FALSE;
				}
				else//走到目标点
				{
					if (m_pMap->IsBlocked(ix, iy)) m_pCurFocusItem = nullptr;
					else if (DISTANCE(getX(), getY(), ix, iy) >= 15) m_pCurFocusItem = nullptr;
					else GotoPosition(ix, iy);
				}
			}
		}
	}
	//隐藏状态, 不移动
	if (IsSystemFlagSeted(SF_HIDED)) return;
	//设定怪物脱离目标时, 返回自己出生点
	if (pTarget == nullptr && GetType() != OBJ_PET)
		if (m_bGotoPoint) GotoPosition(m_wGotoX, m_wGotoY);
	//移动方式
	switch (m_pDesc->aiset.MoveStyle)
	{
	case AMS_FOLLOW://跟踪目标
		Ai_Follow(m_pDesc->attackdesc.AttackDistance);
		break;
	case AMS_ESCAPE://逃跑
		Ai_Escape(m_pDesc->aiset.EscapeDistance);
		break;
	case AMS_KEEPLINE://保持直线
		Ai_KeepLine(m_pDesc->attackdesc.AttackDistance);
		break;
	case AMS_KEEPDISTANCE://保持距离
		Ai_KeepDistance(m_pDesc->attackdesc.AttackDistance, m_pDesc->aiset.EscapeDistance);
		break;
	case AMS_STATIC://静止不动
		Ai_Static(m_pDesc->attackdesc.AttackDistance, (m_pDesc->sprop.pFlag & SF_AUTOHIDE) != 0);
		break;
	case AMS_BIANZOUBIANDA://边走边打
		Ai_BianZouBianDa(m_pDesc->attackdesc.AttackDistance);
		break;
	case AMS_BACKHOME://归位移动
		Ai_BackHome(m_pDesc->attackdesc.AttackDistance, m_pDesc->aiset.ViewDistance, m_pGen);
		break;
	default:
		Ai_StupidMove();//傻瓜乱跑
		break;
	}
}

int	CMonsterEx::GetPropValue(PROP_INDEX index)
{
	if (m_pDesc == nullptr) return 0;
	int value = 0;
	switch (index)
	{
	case PI_LEVEL:
		value = m_pDesc->base.btLevel;
		break;
	case PI_HITRATE:
		value = m_pDesc->prop.hit;
		break;
	case PI_ESCAPE:
		value = m_pDesc->prop.speed;
		break;
	case PI_CURMP:
		value = m_wCurMp;
		break;
	case PI_MINMC:
		value = m_pDesc->prop.mc1;
		break;
	case PI_MAXMC:
		value = m_pDesc->prop.mc2;
		break;
	case PI_MINMAC:
		value = m_pDesc->prop.mac1;
		break;
	case PI_MAXMAC:
		value = m_pDesc->prop.mac2;
		break;
	case PI_CURHP:
		value = m_wCurHp;
		break;
	case PI_MINDC:
		value = m_pDesc->prop.dc1;
		break;
	case PI_MAXDC:
		value = m_pDesc->prop.dc2;
		break;
	case PI_MINAC:
		value = m_pDesc->prop.ac1;
		break;
	case PI_MAXAC:
		value = m_pDesc->prop.ac2;
		break;
	case PI_EXP:
		value = m_pDesc->prop.exp;
		break;
	case PI_MAXHP:
		value = m_pDesc->prop.hp;
		break;
	case PI_MAXMP:
		value = m_pDesc->prop.mp;
		break;
	}
	value += CAliveObject::GetPropValue(index);
	if (value < 0) value = 0;
	return value;
}

void CMonsterEx::DecPropValue(PROP_INDEX index, int value)
{
	switch (index)
	{
	case PI_CURHP:
	{
		WORD wDecHp = static_cast<WORD>(value);
		if (wDecHp > m_wCurHp)
			m_wCurHp = 0;
		else
			m_wCurHp -= wDecHp;
		if (IsStatusSet(SI_GREENPOISON))//绿毒状态
		{
			if (m_wCurHp <= 0) ToDeath(GetSetter(SI_GREENPOISON)); // 如果血量被减到0了, 就判断死亡
		}
	}
	break;
	case PI_CURMP:
	{
		WORD wDecMp = static_cast<WORD>(value);
		if (wDecMp > m_wCurMp)
			m_wCurMp = 0;
		else
			m_wCurMp -= wDecMp;
	}
	break;
	}
}

void CMonsterEx::AddPropValue(PROP_INDEX index, int value)
{
	switch (index)
	{
	case PI_CURMP:
	{
		WORD wAddMp = static_cast<WORD>(value);
		if (m_pDesc->prop.mp <= m_wCurMp)return;
		if (wAddMp < (m_pDesc->prop.mp - m_wCurMp))
			m_wCurMp += wAddMp;
		else
			m_wCurMp = m_pDesc->prop.mp;
	}
	break;
	case PI_CURHP:
	{
		WORD wAddHp = static_cast<WORD>(value);
		if (m_pDesc->prop.hp <= m_wCurHp)return;
		if (wAddHp < (m_pDesc->prop.hp - m_wCurHp))
			m_wCurHp += wAddHp;
		else
			m_wCurHp = m_pDesc->prop.hp;
	}
	break;
	}
}

BOOL CMonsterEx::AiWalk(int dir, BOOL bCheckRun)
{
	if (m_pDesc == nullptr) return FALSE;
	DWORD dwDelay = m_pDesc->prop.walkdelay;
	if (GetTarget() == nullptr)
	{
		if (GetType() == OBJ_MONSTER && (!m_pGen || m_pGen->range <= 1000))
			dwDelay += GetRangeRand(60000, 120000);
	}
	if (bCheckRun && (m_pDesc->sprop.pFlag & SF_ALLOWRUN) != 0)
		return Run(dir, dwDelay) || Walk(dir, dwDelay);
	return Walk(dir, dwDelay);
}

VOID CMonsterEx::OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type)
{
	if (IsSystemFlagSeted(SF_SLEEP)) SetSystemFlag(SF_SLEEP, FALSE);
	if (type == DT_PHYSICS && IsSystemFlagSeted(SF_SOULWALL)) SetSystemFlag(SF_SOULWALL, FALSE);
	if (pAttacker)
	{
		if (m_pDesc == nullptr) return;
		//受伤时, 随机生小怪
		if ((m_pDesc->sprop.pFlag & SF_MINION) != 0 && Getrand(100) < m_pDesc->sprop.MonsterRate)//随机生效
		{
			// 定义8个方向的偏移量
			const int dirs[8][2] = {
				{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}
			};
			//从名字列表中随机一个
			xStringsExtracter<3> monsters(m_pDesc->sprop.MonsterName, ",");
			int nMonster = monsters.getCount();
			int monsterIndex = rand() % nMonster;
			for (int i = 0; i < 6; i++)
			{
				int newX = getX() + dirs[i][0];
				int newY = getY() + dirs[i][1];
				CMonsterEx* pNewMonster = CMonsterManagerEx::GetInstance()->CreateMonster(monsters[monsterIndex], GetMapId(), newX, newY);
				if (pNewMonster == nullptr)	break;
				if (!CGameWorld::GetInstance()->AddMapObject(pNewMonster))
					CMonsterManagerEx::GetInstance()->DeleteMonsterImm(pNewMonster);
			}
			return;
		}
		//	受伤脚本
		if (m_pDesc->pszHurtScript) DoScript(nullptr, m_pDesc->pszHurtScript);

		if ((m_pDesc->sprop.pFlag & SF_NOREATTACK) == 0)
		{
			if (IsProperTarget(pAttacker) && (GetTarget() == nullptr || Getrand(3) == 0))
				SetTarget(pAttacker);
		}
		if (m_pDesc->sprop.pFlag & SF_HURTESCAPE)
			GoAwayPosition(pAttacker->getX(), pAttacker->getY());
	}
	CAliveObject::OnDamage(pAttacker, nDamage, type);
}

BOOL CMonsterEx::AttackTarget(e_direction dir)
{
	//获取目标
	CAliveObject* pTarget = GetTarget();
	if (pTarget == nullptr) return FALSE;
	// 获取目标的坐标
	WORD wTargetX = pTarget->getX(), wTargetY = pTarget->getY();
	int dx = (int)getX() - wTargetX;
	int dy = (int)getY() - wTargetY;
	if (dx != 0 && dy != 0 && abs(dx) != abs(dy)) return FALSE; // 不在8个方向的线上
	if (dir == ED_MAX)
		dir = (e_direction)GetDirectionTo(getX(), getY(), wTargetX, wTargetY);
	if ((m_pDesc->sprop.pFlag & SF_LOCKDIRECTION) == 0) // 固定朝向
		SetDirection(dir);
	//设置AI延时时间
	SetAction(AT_ATTACK, dir, getX(), getY(), m_pDesc->attackdesc.Delay);
	Stop(); // 停止上一次动作
	//攻击方式
	damage_type damagetype = (damage_type)m_pDesc->attackdesc.DamageType;
	WORD wFlag = (m_pDesc->attackdesc.Action << 8) | (WORD)GetDirection();
	const int nDamage = GetPropPower();
	switch (m_pDesc->attackdesc.AttackStyle)
	{
	case AAS_DIRECTATTACK://直接攻击
	{
		//拿走目标的
		pTarget->TakeDamage(GetId(), nDamage, damagetype, 0, DISTANCE(getX(), getY(), wTargetX, wTargetY) * 50 + 300);
		DWORD dwView[2] = { GetFeather(), GetStatus() };
		SendAroundMsg(GetId(), SM_ATTACK, getX(), getY(), wFlag, dwView, sizeof(dwView));
	}
	break;
	case AAS_THROWATTACK://投郑攻击
	{
		pTarget->TakeDamage(GetId(), nDamage, damagetype, 0, DISTANCE(getX(), getY(), wTargetX, wTargetY) * 50 + 300);
		ThrowDesc desc{};
		desc.x = wTargetX;
		desc.y = wTargetY;
		desc.dwTargetId = pTarget->GetId();
		SendAroundMsg(GetId(), 0x16, getX(), getY(), GetDirection(), &desc, sizeof(desc));
	}
	break;
	case AAS_MAGICATTACK://施法攻击
	{
		DWORD dwView[2] = { GetFeather(), GetStatus() };
		SendAroundMsg(GetId(), SM_ATTACK, getX(), getY(), wFlag, dwView, sizeof(dwView));
		switch (m_pDesc->attackdesc.DamageStyle)
		{
		case ADS_RANGE:
			CAliveObject::MagicBoom(nDamage, getX(), getY(), m_pDesc->attackdesc.DamageRange);
			break;
		case ADS_LINE:
			CAliveObject::DoLineMagic(nDamage, GetDirection(), m_pDesc->attackdesc.AttackDistance);
			break;
		case ADS_CROSS:
			CAliveObject::DoLineMagic(nDamage, 0, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 2, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 4, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 6, m_pDesc->attackdesc.DamageRange);
			break;
		case ADS_XSHAPE:
			CAliveObject::DoLineMagic(nDamage, 1, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 3, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 5, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 7, m_pDesc->attackdesc.DamageRange);
			break;
		case ADS_ALLDIR:
			CAliveObject::DoLineMagic(nDamage, 0, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 2, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 4, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 6, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 1, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 3, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 5, m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, 7, m_pDesc->attackdesc.DamageRange);
			break;
		case ADS_SECTOR:
			CAliveObject::DoLineMagic(nDamage, GetValidDirection(GetDirection()), m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, GetValidDirection(GetDirection() + 1), m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, GetValidDirection(GetDirection() + 2), m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, GetValidDirection(GetDirection() - 1), m_pDesc->attackdesc.DamageRange);
			CAliveObject::DoLineMagic(nDamage, GetValidDirection(GetDirection() - 2), m_pDesc->attackdesc.DamageRange);
			break;
		}
	}
	break;
	case AAS_MOYAN://魔眼攻击
	{
		CMonsterAttack monAttack(this);
		monAttack.MoYan(nDamage, damagetype);
	}
	break;
	case AAS_BLOODKING: // 铁血魔王
	{
		CMonsterAttack monAttack(this);
		monAttack.BloodKing(nDamage);
	}
	break;
	}
	return TRUE;
}

BOOL CMonsterEx::CanDoAction(actiontype action)
{
	if (m_ActionType != AT_STAND) return FALSE;
	if (IsSystemFlagSeted(SF_SLEEP)) return FALSE;//休眠状态
	return CAliveObject::CanDoAction(action);
}

VOID CMonsterEx::SelectAttack(ai_attack_style attackStyle)
{
	if( m_pDesc == nullptr )return;
	m_pDesc->attackdesc.AttackStyle = attackStyle;
}

BOOL CMonsterEx::EnterTargetList(CAliveObject* pTarget, UINT nDamage)
{
	BOOL bFlag = FALSE;
	m_xAttackObj.UpdateValid();
	if (m_xAttackObj.getObject() == pTarget)
	{
		m_xAttackObj.AddAttackInfo(pTarget, nDamage);
		bFlag = TRUE;
	}
	else if (m_xAttackObj.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_DROPTARGETTIME) * 1000))
	{
		m_xAttackObj.Clear();
		SetTarget(pTarget);
		m_xAttackObj.AddAttackInfo(pTarget, nDamage);
		bFlag = TRUE;
	}
	return bFlag;
}

CAliveObject* CMonsterEx::GetProperTarget()
{
	if (m_xAttackObj.getObject()) return m_xAttackObj.getObject();
	return nullptr;
}

BOOL CMonsterEx::ChangeInto(const char* pszClassName)
{
	MonsterClass* pDesc = CMonsterManagerEx::GetInstance()->GetClassByName(pszClassName);
	if (pDesc == nullptr) return FALSE;
	RefreshViewmsg(1);
	SetDesc(pDesc);
	return TRUE;
}

BOOL CMonsterEx::CheckChangeInto()
{
	for (int i = 0; i < 3; i++)
	{
		if (m_pDesc->changeinto[i].bEnabled)
		{
			if (!CheckSituation(m_pDesc->changeinto[i].situation1) || !CheckSituation(m_pDesc->changeinto[i].situation2))
				continue;
			DPRINT(SUCCESS_GREEN, "%s 变身为 %s\n", m_pDesc->base.szClassName, m_pDesc->changeinto[i].szChangeInto);
			// 变身前属性值
			int appendEffect = m_pDesc->changeinto[i].AppendEffect;
			if (ChangeInto(m_pDesc->changeinto[i].szChangeInto))
			{
				if (appendEffect > 0)
				{
					if (appendEffect & ACA_CLEANKILLCOUNT) m_dwKillCount = 0;
					if (appendEffect & ACA_FULLHPMP)
					{
						AddPropValue(PI_CURHP, GetPropValue(PI_MAXHP));
						AddPropValue(PI_CURMP, GetPropValue(PI_MAXMP));
					}
					if (appendEffect & ACA_CLEANEXP) m_dwExp = 0;
					if (appendEffect & ACA_CANCLETARGET) SetTarget(nullptr);
				}
				if (IsDeath())
				{
					m_bIsShow = FALSE;
					SetDeath(FALSE);
				}
				RefreshViewmsg();
				SendChangeName();
				return TRUE;
			}
			return FALSE;
		}
	}
	return FALSE;
}

BOOL CMonsterEx::IsProperTarget(CAliveObject* pObject)
{
	//目标为空
	if (pObject == nullptr)return FALSE;
	//目标死亡
	if (pObject->IsDeath())return FALSE;
	//跟目标不是一个地图 
	if (pObject->GetMap() != GetMap())return FALSE;
	//目标隐藏
	if (pObject->IsSystemFlagSeted(SF_HIDED))return FALSE;
	//抗变身、目标变身
	if (m_pDesc && (m_pDesc->sprop.pFlag & SF_ANTTRANSFORM) == 0 && pObject->IsSystemFlagSeted(SF_TRANSFORMED))return FALSE;
	//如果目标是自己, 或者目标是自己的主人, 或者目标和自己的主人一样.
	CAliveObject* pOwner = GetOwner();
	if (pObject == this || pObject == pOwner || (pObject->GetOwner() != nullptr && pObject->GetOwner() == pOwner))return FALSE;
	if (GetType() == OBJ_PET)
	{
		//	目标如果是站在安全区的人, 就放弃攻击
		if (pObject->GetType() == OBJ_PLAYER && ((CHumanPlayer*)pObject)->InSafeArea()) return FALSE;
	}
	if (pOwner == nullptr)	//	如果主人是空, 并且不是疯狂状态, 那么就不攻击同类(怪物)
	{
		if (m_pDesc && m_pDesc->base.btRace == MR_HUMAN)return TRUE;
		if (pObject->GetType() == OBJ_MONSTER)return FALSE;
	}
	return TRUE;
}

VOID CMonsterEx::OnChangeOwner(CAliveObject* pOld, CAliveObject* pNew)
{
	if (m_pMap)
	{
		if (pOld == nullptr && pNew)
		{
			m_pMap->DecObjectCount(OBJ_MONSTER);
			m_pMap->AddObjectCount(OBJ_PET);
			AddVisibleObjectType(OBJ_MONSTER);
			//SearchViewRange();
			AddRef();
			CGameWorld::GetInstance()->AddUpdateMonster(this);
		}
		if (pOld && pNew == nullptr)
		{
			m_pMap->DecObjectCount(OBJ_PET);
			m_pMap->AddObjectCount(OBJ_MONSTER);
			RemoveVisibleObjectType(OBJ_MONSTER);
			//SearchViewRange();
			DecRef();
		}
	}
	if (pOld != nullptr) pOld->DelPet(this);
	if (pNew != nullptr)
	{
		if (!pNew->AddPet(this))
		{
			SetOwner(nullptr);
			return;
		}
		m_bSetOwner = TRUE;
		if (CheckChangeInto())
		{
			//m_bSetOwner = FALSE;
			//SearchViewRange();
			return;
		}
		m_bSetOwner = FALSE;
		sprintf(m_szLongName, "%s(%s)", GetName(), pNew->GetName());
	}
	else if (!IsDeath())
		strcpy(m_szLongName, GetName());
	SetTarget(nullptr);
	SendChangeName();
	RefreshViewList();
}

VOID CMonsterEx::WinExp(DWORD dwExp)
{
	m_dwExp += dwExp;
}

VOID CMonsterEx::AddExp(DWORD dwExp, int level, DWORD dwId)
{
	if (GetOwner())
	{
		GetOwner()->AddExp(dwExp, level, dwId);
		dwExp /= 3;
		if (dwExp == 0)dwExp = 1;
		WinExp(dwExp);
	}
	else
		WinExp(dwExp);
}

BOOL CMonsterEx::SetDesc(MonsterClass* pClass)
{
	if (pClass == m_pDesc) return FALSE;
	if (pClass == nullptr) return FALSE;
	if (m_pDesc != nullptr)
		strcpy(m_szOriginalName, m_pDesc->base.szViewName);
	else
		strcpy(m_szOriginalName, "");
	m_pDesc = pClass;
	SetMpRecoverTick(m_pDesc->prop.recovermptime * 1000);
	SetHpRecoverTick(m_pDesc->prop.recoverhptime * 1000);
	CAliveObject::m_nVisibleObjectFlag = 0;
	AddVisibleObjectType(OBJ_PLAYER);
	if ((pClass->aiset.TargetFlag & ATF_ANIMAL) != 0 ||
		(pClass->aiset.TargetFlag & ATF_MONSTER) != 0 ||
		(pClass->aiset.TargetFlag & ATF_ATTACKSANDCITY) != 0)
		AddVisibleObjectType(OBJ_MONSTER);
	if ((pClass->aiset.TargetFlag & ATF_PETS) != 0)
		AddVisibleObjectType(OBJ_PET);
	if (m_pDesc->sprop.pFlag & SF_PICKUPITEM)
		CAliveObject::AddVisibleObjectType(OBJ_DOWNITEM);
	if (m_pDesc->aiset.LockDir == 0)
		SetDirection((e_direction)Getrand(ED_MAX));
	else
		SetDirection((e_direction)((m_pDesc->aiset.LockDir - 1) % 8));
	CAliveObject* pOwner = GetOwner();
	if (pOwner && m_refObjOwner.IsValid())
		sprintf(m_szLongName, "%s(%s)", m_pDesc->base.szViewName, pOwner->GetName());
	else
		strcpy(m_szLongName, m_pDesc->base.szViewName);
	if (m_pDesc->sprop.pFlag & (SF_AUTOHIDE | SF_SHOW))
		SetSystemFlag(SF_HIDED, TRUE);
	m_pCurFocusItem = nullptr;
	if (m_pMap != nullptr)
		RefreshViewList();
	return TRUE;
}

BOOL CMonsterEx::HolySeizedOk(CHumanPlayer* pPlayer, int level, int times, int probability1, int probability2)
{
	if ((m_pDesc->sprop.pFlag & SF_CANBECALLED) == 0)
	{
		int iRand = Getrand(20);
		if (GetPropValue(PI_LEVEL) < pPlayer->GetPropValue(PI_LEVEL) + 2 && m_pDesc->petset.Type == APT_CALL)
		{
			int extimes = 1;
			if (m_pDesc->sprop.CallRate != 0)
			{
				extimes = GetRangeRand(ROUND(m_pDesc->sprop.CallRate * 0.8), ROUND(m_pDesc->sprop.CallRate * 1.3));
				extimes = ROUND(extimes * 0.75f);
			}
			int nRand = Getrand(100);
#ifdef _DEBUG
			pPlayer->SaySystem("诱惑之光 随机次数=%d 当前次数 = %d", extimes, times);
#endif
			if ((m_pDesc->sprop.CallRate != 0 && times >= extimes) || nRand < probability1)
			{
				SetSystemFlag(SF_SLEEP, FALSE);
				SetSystemFlag(SF_CRAZY, FALSE);
				if (m_pDesc->petset.Type == APT_CALL) return TRUE;
			}
			else if (m_pDesc->base.btRace == MR_UNDEAD && iRand == 0 && !IsSystemFlagSeted(SF_CRAZY))
				SetSystemFlag(SF_CRAZY, TRUE, 0, (iRand + 10) * 1000);
			else if (m_pDesc->base.btRace != MR_UNDEAD && iRand == 0)
				ToDeath(pPlayer->GetId());
		}
		else if (m_pDesc->base.btRace == MR_UNDEAD && Getrand(3) == 0 && !IsSystemFlagSeted(SF_CRAZY))
			SetSystemFlag(SF_CRAZY, TRUE, 0, (iRand + 10) * 1000);
	}
	if (!IsSystemFlagSeted(SF_SLEEP))
	{
		int monsterlevel = GetPropValue(PI_LEVEL);
		monsterlevel = ROUND(monsterlevel * 0.75f);
		if (monsterlevel <= 0)monsterlevel = 1;
		int nRand = Getrand(100);
#ifdef _DEBUG
		pPlayer->SaySystem("诱惑之光 黄名 对比次数=%d 当前次数 = %d", monsterlevel, times);
#endif
		if ((times % monsterlevel) == 0 || nRand < probability2)
			SetSystemFlag(SF_SLEEP, TRUE, 0, (3 * (level + 1) + Getrand(5 * (level + 1))) * 1000);
	}
	return FALSE;
}

BOOL CMonsterEx::CheckSituation(AttackSituation& situation)
{
	if (situation.Situation <= 0)return TRUE;
	int s = situation.Situation;
	int p = situation.Param;
	switch (s)
	{
	case AS_HPBEYOUND:
		if (GetPropValue(PI_CURHP) <= p) return FALSE;
		break;
	case AS_HPBELOW:
		if (GetPropValue(PI_CURHP) >= p) return FALSE;
		break;
	case AS_AROUNDHUMAN:
		if (CAliveObject::m_xVisibleObjectList.getCount() <= p) return FALSE;
		break;
	case AS_MPBELOW:
		if (GetPropValue(PI_CURMP) >= p) return FALSE;
		break;
	case AS_MPBEYOUND:
		if (GetPropValue(PI_CURMP) <= p) return FALSE;
		break;
	case AS_IDLETIMEBEYOUND:
		if (GetTarget() != nullptr || !m_IdleTimer.IsTimeOut(p * 1000)) return FALSE;
		break;
	case AS_EXPBEYOUNDEQUAL:
		if (m_dwExp < (DWORD)p) return FALSE;
		break;
	case AS_CHANGEOWNER:
		if (!m_bSetOwner) return FALSE;
		m_bSetOwner = FALSE;
		break;
	case AS_KILLCOUNTABOVE:
		if (m_dwKillCount <= (DWORD)p) return FALSE;
		break;
	case AS_GOTTARGET:
		if (GetTarget() == nullptr) return FALSE;
		break;
	case AS_DEATH:
		if (!IsDeath()) return FALSE;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

//ATF_HUMAN= 1,
//ATF_MONSTER = 2,
//ATF_ANIMAL = 4,
//ATF_CRIMER = 8,
//ATF_BOSS = 16,
//ATF_ATTACKSANDCITY = 32,	//	攻城者
BOOL CMonsterEx::IsTargetSelectable(CAliveObject* pTarget)
{
	if (m_pDesc == nullptr || pTarget == nullptr)return FALSE;
	//if( pTarget->IsSystemFlagSeted(SF_TRANSFORMED ) )return FALSE;
	if (m_pDesc && (m_pDesc->sprop.pFlag & SF_ANTTRANSFORM) == 0 && pTarget->IsSystemFlagSeted(SF_TRANSFORMED))return FALSE;

	//	隐身cooleyes
	if (pTarget->IsStatusSet(SI_CLOAK))
	{
		UINT nDist = DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY());

		if ( /*nDist > 1 && */m_pDesc->aiset.CoolEyes < nDist)
		{
			return FALSE;
		}
	}
	BYTE btTargetFlag = m_pDesc->aiset.TargetFlag;
	switch (pTarget->GetType())
	{
	case OBJ_PLAYER:
	{
		if (((CHumanPlayer*)pTarget)->GetPkValue() >= CGameWorld::GetInstance()->GetVar(EVI_REDPKPOINT))
		{
			if ((btTargetFlag & ATF_CRIMER))
				return TRUE;
		}
		else if (btTargetFlag & ATF_HUMAN)
			return TRUE;
		//	todo: 攻城者
		if ((btTargetFlag & ATF_ATTACKSANDCITY))
		{
			CGuildEx* pGuild = ((CHumanPlayer*)pTarget)->GetGuild();
			if (pGuild && pGuild->IsAttackSabuk())
				return TRUE;
		}
	}
	break;
	case OBJ_PET:
	{
		if ((btTargetFlag & ATF_PETS) != 0)
		{
			MonsterClass* pDesc = ((CMonsterEx*)pTarget)->GetDesc();
			if (pDesc != nullptr && pDesc->petset.Type == APT_CALL)return TRUE;
		}
		if (pTarget->GetOwner() && (btTargetFlag & ATF_ATTACKSANDCITY) != 0 && pTarget->GetOwner()->GetType() == OBJ_PLAYER)
		{
			CHumanPlayer* pPlayer = (CHumanPlayer*)pTarget->GetOwner();
			CGuildEx* pGuild = pPlayer->GetGuild();
			if (pGuild && pGuild->IsAttackSabuk())
				return TRUE;
		}
		return FALSE;
	}
	break;
	case OBJ_MONSTER:
	{
		CMonsterEx* pTargetM = (CMonsterEx*)pTarget;
		MonsterClass* pMDesc = pTargetM->GetDesc();
		if (pMDesc == nullptr)return FALSE;
		//	不打人类怪物
		if (pMDesc->base.btRace == MR_HUMAN)
			return FALSE;
		//	怪物攻城, 大刀不管
		if (pTargetM->m_bGotoPoint && m_pDesc && m_pDesc->base.btRace == MR_HUMAN)
			return FALSE;
		if ((pMDesc->sprop.pFlag & SF_AUTOATTACK))
		{
			if (btTargetFlag & ATF_MONSTER)
				return TRUE;
		}
		else if (btTargetFlag & ATF_ANIMAL)
			return TRUE;
	}
	break;
	}
	return FALSE;
}

BOOL CMonsterEx::TrainOk(CHumanPlayer* pPlayer, int times)
{
	int extimes = GetRangeRand(ROUND(m_pDesc->sprop.CallRate * 0.8), ROUND(m_pDesc->sprop.CallRate * 1.3));
	if (times >= extimes)
		return TRUE;
	return FALSE;
}

BOOL CMonsterEx::CanEnterMap(CLogicMap* pMap)
{
	if (!pMap->IsFlagSeted(MF_RIDEHORSE))
	{
		if (m_pDesc != nullptr && m_pDesc->petset.Type == APT_RIDE)return FALSE;
	}
	return TRUE;
}

VOID CMonsterEx::OnEnterMap(CLogicMap* pMap)
{
	CAliveObject::OnEnterMap(pMap);
	if (m_bFirstEnterMap)
	{
		m_bFirstEnterMap = FALSE;
		if (m_pDesc && m_pDesc->pszBornScript) // 出生脚本
			DoScript(nullptr, m_pDesc->pszBornScript);
	}
}

VOID CMonsterEx::OnSystemFlagSeted(int index, DWORD dwParam)
{
	switch (index)
	{
	case SF_SLEEP:
	{
		SendChangeName();
	}
	break;
	case SF_SOULWALL:
	{
		SendAroundMsg(GetId(), 0x323, 0, 0, 0, &dwParam, sizeof(DWORD));
	}
	break;
	}
}

VOID CMonsterEx::OnSystemFlagCleared(int index, DWORD dwParam)
{
	switch (index)
	{
	case SF_SLEEP:
	case SF_CRAZY:
	{
		SendChangeName();
	}
	break;
	case SF_SOULWALL:
	{
		SendAroundMsg(GetId(), 0x323, 1, 0, 0, &dwParam, sizeof(DWORD));
	}
	break;
	case SF_FAKETARGET:
	{
		CGameWorld::GetInstance()->RemoveMapObject(this);
		CMonsterManagerEx::GetInstance()->DeleteMonster(this);
	}
	break;
	}
}

BYTE CMonsterEx::GetNameColor(CMapObject* pViewer)
{
	if (m_pDesc == nullptr)return 255;
	if (IsSystemFlagSeted(SF_SLEEP))
		return 125; // 黄名-休眠模式
	if (IsSystemFlagSeted(SF_CRAZY))
		return 249; // 红名-狂魔模式
	return m_pDesc->base.btNameColor;
}

VOID CMonsterEx::OnChangeTarget(CAliveObject* pOld, CAliveObject* pNew)
{
	CAliveObject::OnChangeTarget(pOld, pNew);
	if (pNew != nullptr && m_pDesc)
	{
		if (m_pDesc->pszGotTargetScript)
			DoScript((pNew->GetType() == OBJ_PLAYER ? (CHumanPlayer*)pNew : nullptr), m_pDesc->pszGotTargetScript);
		if (GetType() == OBJ_PET && m_pDesc->petset.Type == APT_CALL)
		{
			CHumanPlayer* pPlayer = (CHumanPlayer*)GetOwner();
			if (pPlayer) pPlayer->SetPetTarget(pNew);
		}
		if (!pOld != !pNew) SetAction(AT_AIACTION, GetDirection(), getX(), getY(), 0);
		if (DISTANCE(pNew->getX(), pNew->getY(), getX(), getY()) > m_pDesc->aiset.ViewDistance) return;
		if (m_pMap && (m_pDesc->sprop.pFlag & SF_SHARETARGET) != 0) // 同类共享目标
		{
			static std::vector<CMonsterEx*> s_monsterCache; // 使用静态缓存避免频繁分配
			s_monsterCache.clear();
			int nStartX = getX() - m_pDesc->aiset.ViewDistance;
			int nStartY = getY() - m_pDesc->aiset.ViewDistance;
			int nEndX = getX() + m_pDesc->aiset.ViewDistance;
			int nEndY = getY() + m_pDesc->aiset.ViewDistance;
			nStartX = max(0, nStartX);
			nStartY = max(0, nStartY);
			nEndX = min(m_pMap->GetWidth() - 1, nEndX);
			nEndY = min(m_pMap->GetHeight() - 1, nEndY);
			for (int x = nStartX; x <= nEndX; x++)
			{
				for (int y = nStartY; y <= nEndY; y++)
				{
					CMapCellInfo* pInfo = m_pMap->GetMapCellInfo(x, y);
					if (!pInfo) continue;
					xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
					while (pNode)
					{
						CMapObject* pObject = pNode->getObject();
						if (pObject && pObject->GetType() == GetType())
						{
							CMonsterEx* pMonster = (CMonsterEx*)pObject;
							if (!pMonster->IsDeath() && pMonster->GetTarget() == nullptr)
								s_monsterCache.push_back(pMonster);
						}
						pNode = pNode->getNext();
					}
				}
			}
			for (CMonsterEx* pMonster : s_monsterCache)
			{
				pMonster->SetTarget(pNew);
			}
		}
	}
}

VOID CMonsterEx::ShareTarget(CAliveObject* pNew)
{
	if (m_pGen == nullptr) 
	{
		SetTarget(pNew);
		return;
	}
	xIndexPtrListHelper<CMonsterEx> helper(m_pGen->xMonsterList);
	CMonsterEx* pMonster = helper.first();
	while (pMonster != nullptr)
	{
		if (pMonster->GetTarget() == nullptr)
			pMonster->SetTarget(pNew);
		pMonster = helper.next();
	}
}

VOID CMonsterEx::OnCuted(CHumanPlayer* pCuter)
{
	if (m_fCuted)
	{
		if (pCuter)
			pCuter->SaySystem("没有发现任何东西");
		return;
	}
	m_fCuted = TRUE;
	// 掉落配置物品
	if (m_pDesc != nullptr && (m_pDesc->sprop.pFlag & SF_BODYITEM) != 0 && m_pDesc->pDownItems != nullptr)
	{
		ITEM item;
		DOWNITEM* pDownItem = m_pDesc->pDownItems->pItems;
		while (pDownItem)
		{
			if (pDownItem->updatecycle())
			{
				if (pDownItem->bGold)
					pCuter->AddGold(GetRangeRand(pDownItem->nCount, pDownItem->nCountMax));
				else
				{
					if (pCuter->GetBagFree() == 0) // 背包满
						CDownItemMgr::GetInstance()->DropItem(m_pMap, item, getX(), getY(), TRUE, pCuter);
					else
					{
						if (CMonItemsMgr::GetInstance()->CreateDownItem(pDownItem, item))
						{
							if (!pCuter->AddBagItem(item))
								CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
						}
					}
				}
			}
			pDownItem = pDownItem->pNext;
		}
	}
	if (m_pDesc && (m_pDesc->sprop.pFlag & SF_CANBECUT))
	{
		WORD hp = static_cast<WORD>(GetPropValue(PI_CURHP));
		WORD maxhp = static_cast<WORD>(GetPropValue(PI_MAXHP));
		DWORD dwParam[3] = { GetFeather(), GetStatus(), (hp << 16) | maxhp };
		SendAroundMsg(GetId(), 0x21, getX(), getY(), (WORD)GetDirection(), dwParam, sizeof(dwParam));
		SetSystemFlag(SF_BONE, TRUE);
	}
}

VOID CMonsterEx::OnDamageTarget(CAliveObject* pTarget, int nDamage, damage_type type)
{
	CAliveObject::OnDamageTarget(pTarget, nDamage, type);
	if (pTarget == nullptr || m_pDesc == nullptr)
		return;
	if (m_pDesc->attackdesc.AppendEffect == AAA_NONE)
		return;
	if (Getrand(100) > m_pDesc->attackdesc.AppendRate)
		return;
	DWORD dwTime = m_pDesc->attackdesc.AppendTime == 0 ? 0 : m_pDesc->attackdesc.AppendTime * 1000;
	switch (m_pDesc->attackdesc.AppendEffect)
	{
	case AAA_REDPOISON://红毒
			pTarget->SetStatus(SI_REDPOISON, 4, dwTime);//给目标设置红毒状态
		break;
	case AAA_GREENPOISON://绿毒
			pTarget->SetStatus(SI_GREENPOISON, 4, dwTime);//给目标设置绿毒状态
		break;
	case AAA_STONE://石化
		break;
	case AAA_STRAWMAN://红咒
			pTarget->SetStatus(SI_STRAWMAN, 4, dwTime);//给目标设置红咒状态
		break;
	case AAA_STRAWWOMAN://蓝咒
			pTarget->SetStatus(SI_STRAWWOMAN, 4, dwTime);//给目标设置蓝咒状态
		break;
	case AAA_FROZEN://冰冻
			pTarget->SetSystemFlag(SF_FROZEN, TRUE, 0, dwTime); // 给目标设置冰冻系统标识
		break;
	case AAA_MABI://麻痹
			pTarget->SetStatus(SI_PALSY, 0, dwTime);
		break;
	case AAA_PUSHED://野蛮冲撞
	{
		WORD wTargetX, wTargetY;
		pTarget->GetPosition(wTargetX, wTargetY);
		int dir = GetDirectionTo(getX(), getY(), wTargetX, wTargetY);
		pTarget->DoPushed(dir);
	}
	break;
	case AAA_LIONPUSHED://突斩
	{
		WORD wTargetX, wTargetY;
		pTarget->GetPosition(wTargetX, wTargetY);
		int dir = GetDirectionTo(getX(), getY(), wTargetX, wTargetY);
		pTarget->DoLionPush(dir);
	}
	break;
	case AAA_FIREWALL://火墙
	{
		nDamage = GetRangeRand(GetPropValue(PI_MINDC), GetPropValue(PI_MAXDC));
		if (m_pDesc->attackdesc.AppendType > 0)
		{
			switch (m_pDesc->attackdesc.AppendType)
			{
			case 2:
				CFireNimoBurnEvent::Create(this, getX(), getY(), 2, nDamage, 2000, dwTime);
				break;
			case 3:
				CFireNimoBurnEvent::Create(this, pTarget->getX(), pTarget->getY(), 4, nDamage, 2000, dwTime);
				break;
			case 4:
				CFireNimoBurnEvent::Create(this, pTarget->getX(), pTarget->getY(), 3, nDamage, 2000, dwTime);
				break;
			case 1:
			default:
				CFireNimoBurnEvent::Create(this, getX(), getY(), 1, nDamage, 2000, dwTime);
				break;
			}
			break;
		}

		int nDis = DISTANCE(getX(), getY(), pTarget->getX(), pTarget->getY());
		if (Getrand(20) < 1)
			CFireNimoBurnEvent::Create(this, pTarget->getX(), pTarget->getY(), 4, nDamage, 2000, dwTime);
		else
		{
			if (nDis == 1)
				CFireNimoBurnEvent::Create(this, getX(), getY(), 2, nDamage, 2000, dwTime);
			if (nDis == 2)
				CFireNimoBurnEvent::Create(this, pTarget->getX(), pTarget->getY(), 3, nDamage, 2000, dwTime);
			if (nDis == 3)
				CFireNimoBurnEvent::Create(this, getX(), getY(), 1, nDamage, 2000, dwTime);
		}
	}
	break;
	}
}

VOID CMonsterEx::RefreshViewmsg(int type)
{
	char szBuffer[1024];
	int length = 0;
	if (type == 0)
		if (GetViewmsg(szBuffer, length, this)) SendAroundMsg(szBuffer, length);
	if (type == 1)
		if (GetOutViewmsg(szBuffer, length, this)) SendAroundMsg(szBuffer, length);
	if (type == 2)
	{
		if (GetOutViewmsg(szBuffer, length, this)) SendAroundMsg(szBuffer, length);
		if (GetViewmsg(szBuffer, length, this)) SendAroundMsg(szBuffer, length);
	}
}

BOOL CMonsterEx::GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	length = 0;
	if (!IsDeath() && GetType() == OBJ_PET && m_pDesc && m_pDesc->petset.Type == APT_CALL)
	{
		DWORD dwParams[4] = { GetFeather(), 0, 0, 0 };
		int tempSize = 0;
		SmartEncodeMessage(pszMsg, tempSize, GetId(), 0x14, getX(), getY(), (WORD)GetDirection(), (LPVOID)dwParams, sizeof(dwParams));
		length = tempSize;
	}
	int alength = 0;
	BOOL bRet = CAliveObject::GetViewmsg(pszMsg + length, alength, pViewer);
	if (!bRet)return FALSE;
	length += alength;
	if (IsSystemFlagSeted(SF_SOULWALL))
	{
		DWORD dwParam = GetSystemFlagParam(SF_SOULWALL);
		char szTempBuffer[1024];
		int tempSize = 0;
		SmartEncodeMessage(szTempBuffer, tempSize, GetId(), 0x323, 0, 0, 0, &dwParam, sizeof(DWORD));
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}
	return TRUE;
}

VOID CMonsterEx::ClearGen()
{
	if (m_pGen == nullptr) return;
	m_pGen->xMonsterList.delObject(this);
	m_pGen->curcount--;
	if (m_pGen->curcount < 0)m_pGen->curcount = 0;
	if (m_pGen->curcount == 0 && m_pGen->xMonsterList.getCount() == 0)
		m_pGen->xMonsterList.destroy();
	m_pGen = nullptr;
	CGameWorld::GetInstance()->RemoveMapObject(this);
}

VOID CMonsterEx::DoScript(CHumanPlayer* pTarget, const char* pszPage)
{
	CMonsterManagerEx::GetInstance()->SetCurrentActiveMonster(this);
	if (pTarget == nullptr)
		pTarget = CAutoScriptManager::GetInstance()->GetScriptTarget();
	CSystemScript::GetInstance()->Execute(pTarget->GetScriptTarget(), pszPage, 0);
	CMonsterManagerEx::GetInstance()->SetCurrentActiveMonster(nullptr);
}

VOID CMonsterEx::OnKillTarget(CAliveObject* pTarget)
{
	if (pTarget)
	{
		if (m_pDesc && m_pDesc->pszKillTargetScript)
		{
			if (pTarget->GetType() == OBJ_PLAYER)
				DoScript((CHumanPlayer*)pTarget, m_pDesc->pszKillTargetScript);
			else
				DoScript(nullptr, m_pDesc->pszKillTargetScript);
		}
	}
}

VOID CMonsterEx::SendSkill(DWORD tId, WORD x, WORD y, WORD wMagicID)
{
	MONSKILL monSkill{};
	monSkill.tId = tId;
	monSkill.wX = x;
	monSkill.wY = y;
	monSkill.wMagicID = wMagicID;
	SendAroundMsg(GetId(), 0x271, 0, 0, GetDirection(), &monSkill, sizeof(monSkill));
}