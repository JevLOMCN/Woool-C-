#include "StdAfx.h"
#include <vector>
#include "aliveobject.h"
#include "vmap.h"
#include "server.h"
#include "logicmap.h"
#include "gameworld.h"
#include "DownItemObject.h"
#include "DownItemMgr.h"
#include "ItemManager.h"
#include "logicMapMgr.h"
#include "visibleevent.h"
#include "sandcity.h"
#include "monsterex.h"
#include "FireRainEvent.h"
#include "specialequipmentmanager.h"

DWORD g_dwActionDelay[AT_MAX] =
{
	0, 600, 600, 0, 0, 0, 850, 0, 300, 600, 0, 100, 0
};

//线程安全的消息缓冲区65k大小
static thread_local CHAR s_threadMessageBuffer[65536];

BOOL CRefObject::IsValid()
{
	if (m_pObject == nullptr)return FALSE;
	if (m_pObject->GetInstanceKey() != m_dwInstanceKey)return FALSE;
	return TRUE;
}

VOID CRefObject::SetObject(CAliveObject* pObject)
{
	m_pObject = pObject;
	m_dwInstanceKey = pObject == nullptr ? 0 : pObject->GetInstanceKey();
}

xObjectPool<VISIBLE_OBJECT>	CAliveObject::m_xVisibleObjectPool;
CAliveObject::CAliveObject(void)
{
	Clean();
}

CAliveObject::~CAliveObject(void)
{
}

VOID CAliveObject::Clean()
{
	OBJECTPROCESS* pProcess = nullptr;
	while ((pProcess = m_xQProcess.pop()))
	{
		CGameWorld::GetInstance()->FreeProcess(pProcess);
	}
	m_dwProcessFlags = 0;
	m_Status.Clean();
	m_SystemFlag.Clean();
	CleanVisibleList();
	CMapObject::Clean();
	m_Direction = ED_DN;
	m_btWalkSpeed = 1;
	m_btRunSpeed = 2;
	m_wDeInvisibleLevel = 0;
	m_ActionType = AT_STAND;
	m_PreActionType = AT_STAND;
	m_CurAttackType = DT_PHYSICS;
	m_wActionX = 0;
	m_wActionY = 0;
	m_ActionDirection = ED_DN;
	m_dwActionCompleteTime = 0;
	m_Id = 0;
	m_Mapid = 0;
	m_bPosLocked = FALSE;
	m_nVisibleObjectFlag = 0;
	AddVisibleObjectType(OBJ_PLAYER);
	m_bDead = FALSE;
	m_dwInstanceKey = (timeGetTime() ^ static_cast<DWORD>(reinterpret_cast<uintptr_t>(this)));
	SetPetsActive(TRUE);
	memset(m_AddProp, 0, sizeof(m_AddProp));
	SetNoDead(FALSE);
	SetNoDamage(FALSE);
	SetSuperHit(FALSE);
	SetHpRecoverTick(800);
	SetMpRecoverTick(800);
	m_AddHpTimer.Savetime();
	m_AddMpTimer.Savetime();
	SetAddHp(0, 0);
	SetAddMp(0, 0);
	//m_dwSystemFlag = 0;
	SetOwner(nullptr);
	SetHitter(nullptr);
	SetTarget(nullptr);
	m_AttackObject.Clear();
	m_szLongName[0] = 0;
	m_dwSkill6 = 0;
	m_dwSkill45 = 0;
	m_dwStatus26 = 0;
}

VOID CAliveObject::DropGold(DWORD dwCount, int x, int y, DWORD dwOwner)
{
	ITEM item;
	if (dwCount == 0)return;
	//if( !CItemManager::GetInstance()->CreateTempItem( CGameWorld::GetInstance()->GetName( ENI_GOLD ), item ) )
	//	return;
	memset(&item, 0, sizeof(item));
	o_strncpy(item.baseitem.szName, CGameWorld::GetInstance()->GetName(ENI_GOLD), 12);
	item.baseitem.btNameLength = (BYTE)strlen(item.baseitem.szName);
	item.baseitem.btStdMode = 255;
	CItemManager::GetInstance()->IdentItem(item);
	item.baseitem.wImageIndex = 0xe1;
	if (dwCount > 1000)
		item.baseitem.wImageIndex = 0xe5;
	else if (dwCount > 500)
		item.baseitem.wImageIndex = 0xe4;
	else if (dwCount > 300)
		item.baseitem.wImageIndex = 0xe3;
	else if (dwCount > 100)
		item.baseitem.wImageIndex = 0xe2;
	item.wCurDura = dwCount & 0xffff;
	item.wMaxDura = (dwCount & 0xffff0000) >> 16;
	CDownItemMgr::GetInstance()->DropItem(m_pMap, item, x, y, dwOwner);
}

BOOL CAliveObject::Walk(int dir, DWORD dwDelay)
{
	if (!CanDoAction(AT_WALK))
		return FALSE;
	int x = m_wX, y = m_wY;
	int nSpeed = GetWalkSpeed();
	for (int i = 0; i < nSpeed; i++)
	{
		GETNEXTPOS(x, y, dir);
		if (m_pMap->IsBlocked(x, y)) return FALSE;
	}
	DWORD dwTime = dwDelay == 0 ? g_dwActionDelay[AT_WALK] : dwDelay;
	if (!SetAction(AT_WALK, (e_direction)dir, x, y, dwTime)) return FALSE;

	DWORD dwView[2] = { GetFeather(), GetStatus() };
	SendAroundMsg(GetId(), SM_WALK, x, y, dir, (LPVOID)dwView, sizeof(dwView));
	return TRUE;
}

BOOL CAliveObject::WalkXY(int x, int y, int dir, DWORD dwDelay)
{
	if (!CanDoAction(AT_WALK))return FALSE;
	if (!CheckIsNextPos(getX(), getY(), x, y, GetWalkSpeed()))return FALSE;
	if (m_pMap->IsBlocked(x, y))return FALSE;
	DWORD dwTime = dwDelay == 0 ? g_dwActionDelay[AT_WALK] : dwDelay;
	if (!SetAction(AT_WALK, (e_direction)dir, x, y, dwTime))return FALSE;

	DWORD dwView[2] = { GetFeather(), GetStatus() };
	SendAroundMsg(GetId(), SM_WALK, x, y, dir, (LPVOID)dwView, sizeof(dwView));
	return TRUE;
}

BOOL CAliveObject::Run(int dir, DWORD dwDelay)
{
	if (!CanDoAction(AT_RUN)) return FALSE;
	int x = m_wX, y = m_wY;
	int nSpeed = GetRunSpeed();
	for (int i = 0; i < nSpeed; i++)
	{
		GETNEXTPOS(x, y, dir);
		if (m_pMap->IsBlocked(x, y)) return FALSE;
	}
	DWORD dwTime = dwDelay == 0 ? g_dwActionDelay[AT_RUN] : dwDelay;
	if (!SetAction(AT_RUN, (e_direction)dir, x, y, dwTime)) return FALSE;

	DWORD dwView[2] = { GetFeather(), GetStatus() };
	SendAroundMsg(GetId(), SM_RUN, x, y, dir, (LPVOID)dwView, sizeof(dwView));
	return TRUE;
}

BOOL CAliveObject::RunXY(int x, int y, int dir, DWORD dwDelay)
{
	if (!CanDoAction(AT_RUN))return FALSE;
	int nSpeed = GetRunSpeed();
	if (!CheckIsNextPos(m_wX, m_wY, x, y, nSpeed))return FALSE;
	int mx = m_wX, my = m_wY;
	for (int i = 0; i < nSpeed; i++)
	{
		GETNEXTPOS(mx, my, dir);
		if (m_pMap->IsBlocked(mx, my))return FALSE;
	}
	DWORD dwTime = dwDelay == 0 ? g_dwActionDelay[AT_RUN] : dwDelay;
	if (!SetAction(AT_RUN, (e_direction)dir, x, y, dwTime))return FALSE;

	DWORD dwView[2] = { GetFeather(), GetStatus() };
	SendAroundMsg(GetId(), SM_RUN, mx, my, dir, (LPVOID)dwView, sizeof(dwView));
	return TRUE;
}

VOID CAliveObject::Stop()
{
	if (m_ActionType != AT_STAND)
	{
		SendAroundMsg(GetId(), SM_STOP, 0, 0, 0);
		if (CanRecvMsg())
			SendMsg(GetId(), SM_STOP, 0, 0, 0);
	}
}

BOOL CAliveObject::Turn(int dir)
{
	if (m_ActionType == AT_PRIVATESHOP || IsStatusSet(SI_PALSY) || IsSystemFlagSeted(SF_FROZEN))
		return FALSE;
	SetDirection((e_direction)dir);
	int dwHp = GetPropValue(PI_CURHP); // 当前生命值
	int dwMaxHp = GetPropValue(PI_MAXHP); // 最大生命值
	WORD w1 = 100;
	WORD wd = (dwMaxHp > 0) ? (dwHp * 100 / dwMaxHp) : 0; // 百分比取整
	DWORD dwView[3] = { GetFeather(), GetStatus(), static_cast<DWORD>(MAKELONG(w1, wd)) };
	SendAroundMsg(GetId(), SM_APPEAR, m_wX, m_wY, dir, (LPVOID)dwView, sizeof(dwView));
	return TRUE;
}

BOOL CAliveObject::Attack(int dir, DWORD dwDelay, e_humanattackmode mode)
{
	if (!CanDoAction(AT_ATTACK)) return FALSE;
	DWORD dwView[] = { GetFeather(), GetStatus() };
	SendAroundMsg(GetId(), SM_ATTACK, m_wX, m_wY, dir, (LPVOID)dwView, 8); // 普通攻击

	int x = m_wX, y = m_wY;
	GETNEXTPOS(x, y, dir);

	CAliveObject* pObj = (CAliveObject*)m_pMap->FindTarget(this, x, y);
	if (pObj == nullptr)
	{
		m_tmrAttack.Savetime();
		return TRUE;//不管有没有找到目标, 让玩家都可以发出攻击动作
	}
	if (mode == HAM_PEACE && (pObj->GetType() == OBJ_PLAYER || pObj->GetType() == OBJ_PET))
	{
		m_tmrAttack.Savetime();
		return TRUE;
	}
	if (!pObj->IsDeath() && IsProperTarget(pObj))//如果目标没死
	{
		int damage = GetPropPower();
		OnAttackTarget(pObj, damage);//虚函数调用字HUMANPLay的子类, 最终调用是否攻杀剑法等等
		pObj->AddProcess(EP_BEATTACKED, damage, GetId(), 0, DT_PHYSICS, 200);
		SetCurAttackType(DT_PHYSICS);
	}
	m_tmrAttack.Savetime();
	return TRUE;
}

BOOL CAliveObject::BeAttack(CAliveObject* pAttacker, int nDamage, damage_type damagetype, DWORD dwFlag, damage_ReType damageReType)
{
	if (GetType() == OBJ_MONSTER)
	{
		CMonsterEx* pMon = (CMonsterEx*)this;
		if (pMon->GetApprType() == APPR_TREE && pAttacker->GetCurAttackType() != DT_CUTTREE)
			return FALSE;
	}
	if (IsNoDamage()) nDamage = 0; // 无敌模式
	if (damagetype == DT_PHYSICS)
		nDamage -= GetPropAC(); // 减掉防御
	if (damagetype == DT_MAGIC)
		nDamage -= GetPropMAC(); // 减掉魔防
	if (pAttacker && pAttacker->GetType() == OBJ_PLAYER)
	{
		if (pAttacker->IsGodBlessEffectivable(SG_IMMDIE, this))
		{
			dwFlag |= DF_SUPERHIT;
			nDamage = GetPropValue(PI_CURHP);
			pAttacker->AddProcess(EP_GODBLESS, GetId(), 1);
		}
		else if (pAttacker->IsGodBlessEffectivable(SG_BIGHIT, this))
		{
			damagetype = DT_DIRECT;
			nDamage = ROUND(GetPropValue(PI_MAXHP) / 2);
			pAttacker->AddProcess(EP_GODBLESS, GetId(), 2);
		}
	}
	if (pAttacker && GetType() == OBJ_PLAYER && IsGodBlessEffectivable(SG_SHIELD, pAttacker))
	{
		AddProcess(EP_GODBLESS, pAttacker->GetId(), 4);
		return FALSE;
	}

	//	攻击者检测
	//	神御在此生效
	//	判断状态是不是无敌
	if (IsSystemFlagSeted(SF_NODAMAGE)) return FALSE;

	if (pAttacker)
	{
		if (pAttacker->IsSuperHit()) // 暴击
		{
			nDamage = GetPropValue(PI_CURHP);
			dwFlag |= DF_NOSAFE | DF_SUPERHIT;
		}
	}
	//	安全区检测
	if (dwFlag & DF_NOSAFE)
	{
		if (GetType() != OBJ_MONSTER && GetType() != OBJ_PET)
		{
			CMapCellInfo* pCell = nullptr;
			if (pAttacker)
			{
				if (pAttacker->GetType() != OBJ_MONSTER)
				{
					pCell = m_pMap->GetMapCellInfo(pAttacker->getX(), pAttacker->getY());
					if (pCell && (pCell->wFlag & MAPCELLFLAG_NOPK))
						return FALSE;
				}
			}
			pCell = m_pMap->GetMapCellInfo(getX(), getY());
			if (pCell && (pCell->wFlag & MAPCELLFLAG_NOPK))
				return FALSE;
		}
	}
	// 超级命中
	if ((dwFlag & DF_SUPERHIT) == 0)
	{
		if (pAttacker != nullptr)
		{
			if (damagetype == DT_PHYSICS || damagetype == DT_DIRECT) // 计算普通攻击或者投掷攻击伤害
			{
				int escape = GetPropValue(PI_ESCAPE); // 计算躲避
				if (escape > 0)
				{
					int hitrate = pAttacker->GetPropValue(PI_HITRATE); // 计算命中
					if (hitrate < GetRangeRand(escape / 15, escape))
						return FALSE;
					if (pAttacker && pAttacker->GetType() == OBJ_PLAYER && GetType() == OBJ_PLAYER) // 只对玩家计算
						nDamage = nDamage - escape + hitrate;
				}
			}
			else if (damagetype == DT_MAGIC) // 计算魔法躲避
			{
				int escape = GetPropValue(PI_MAGESCAPE); // 计算魔法躲避
				if (escape > 0)
				{
					if (escape > 100) escape = 100;
					int hitrate = pAttacker->GetPropValue(PI_MAGHITRATE); // 计算魔法命中
					if (escape - hitrate > Getrand(100))
						return FALSE;
					if (pAttacker && pAttacker->GetType() == OBJ_PLAYER && GetType() == OBJ_PLAYER) // 只对玩家计算
						nDamage = nDamage * (100 - escape + hitrate)/100;
				}
			}
		}
		// 在这里写了魔法盾免伤害
		if (IsStatusSet(SI_BUBBLEDEFENCEUP))
		{
			if (damagetype == DT_PHYSICS) // 这里写了魔法盾物理减免伤害
				nDamage = GetHitDamage(nDamage);
			if (damagetype == DT_MAGIC) // 这里写了魔法盾魔法减免伤害
				nDamage = GetMagDamage(nDamage);
		}
		// 在这里写了金刚护体免伤害;
		if (IsSystemFlagSeted(SF_STRONGSHIELD))
		{
			int nodamage = GetNoDamage();
			double nodamagePercentage = nodamage / 100.0; // 将无伤害值除以 100, 得到百分比值
			// 将整数类型的 nDamage 转换为浮点数类型, 执行乘法运算, 然后再转换回整数类型
			int temp = static_cast<int>(static_cast<double>(nDamage) * nodamagePercentage);
			nDamage -= temp;
		}
	}
	if (nDamage < 0) nDamage = 0;
	//此处用计算特殊攻击
	if (pAttacker != nullptr)
	{
		SetHitter(pAttacker);
		pAttacker->OnDamageTarget(this, nDamage, damagetype);
	}
	if (!Damage((pAttacker == nullptr ? 0 : pAttacker->GetId()), nDamage))
		dwFlag |= DF_NOSTRUCKACTION;
	WORD wMsg = (dwFlag & DF_TARGETEFFECT) != 0 ? 0x2f : 0x1f; // 0x2f 破盾 破击 风火轮等新技能的击中后魔法效果封包. 0x1f 被攻击, 后仰一下效果封包.
	DWORD wid = 0;
	DWORD dwEffectType = 0; // 破击, 破盾效果类型
	if (wMsg == 0x2f) // 设计破击, 破盾的技能标识
	{
		DWORD w = dwFlag - DF_TARGETEFFECT;
		dwEffectType = w;
		WORD w1 = 0;
		WORD wd = (WORD)w;
		wid = static_cast<DWORD>(MAKELONG(w1, wd));
	}
	if ((dwFlag & DF_NOSTRUCKACTION) == 0)
	{
		int dwHp = GetPropValue(PI_CURHP); // 当前生命值
		int dwMaxHp = GetPropValue(PI_MAXHP); // 最大生命值
		DWORD dwView[5]{};
		if (wMsg == 0x2f)
		{
			dwView[0] = GetFeather();
			dwView[1] = GetStatus();
			dwView[2] = timeGetTime();
			dwView[3] = pAttacker == nullptr ? 0 : pAttacker->GetId();
			dwView[4] = wid;
		}
		else
		{
			dwView[0] = GetFeather();
			dwView[1] = GetStatus();
			dwView[2] = 0;
			dwView[3] = pAttacker == nullptr ? 0 : pAttacker->GetId();
			dwView[4] = timeGetTime();
		}
		WORD heathHP = 100; // 血条长度按百分之显示
		WORD WbiliHP = (dwMaxHp > 0) ? (dwHp * 100 / dwMaxHp) : 0; // 百分比取整
		SendAroundMsg(GetId(), wMsg, WbiliHP, heathHP, nDamage, (LPVOID)dwView, sizeof(dwView));
		SendMsg(GetId(), wMsg, dwHp, dwMaxHp, nDamage, (LPVOID)dwView, sizeof(dwView));
	}
	//发送破甲、破盾音效封包
	if (damageReType == DRT_PODUN || damageReType == DRT_POJI)
	{
		if (pAttacker)
		{
			SendAroundMsg(pAttacker->GetId(), 0x9cbf, pAttacker->getX(), pAttacker->getY(), dwEffectType - 5);
			SendMsg(pAttacker->GetId(), 0x9cbf, pAttacker->getX(), pAttacker->getY(), dwEffectType - 5);
		}
	}
	if (nDamage > 0)
	{
		SendHpMpChanged(nDamage);
		OnDamage(pAttacker, nDamage, damagetype);
	}
	return TRUE;
}

BOOL CAliveObject::GetMeal(int dir)
{
	if (!CanDoAction(AT_GETMEAL))return FALSE;
	if (!SetAction(AT_GETMEAL, (e_direction)dir, getX(), getY(), 800))return FALSE;
	return TRUE;
}

BOOL CAliveObject::Backward(int dir)
{
	return TRUE;
}

VOID CAliveObject::Say(const char* pszMsg, ...)
{
	char szBuff[248];
	sprintf(szBuff, "%s: ", GetName());
	va_list	vl;
	va_start(vl, pszMsg);
	size_t nLen = strlen(szBuff);
	(void)_vsnprintf(szBuff + nLen, 247 - nLen, pszMsg, vl);
	va_end(vl);
	szBuff[120] = '\0';
	CHAR* pBuffer = s_threadMessageBuffer;
	int size = 0;
	SmartEncodeMessage(pBuffer, size, GetId(), SM_CHAT, 0xff00, 0, 0, (LPVOID)szBuff);
	if (CanRecvMsg()) OnAroundMsg(this, pBuffer, size);
	SendAroundMsg(pBuffer, size);
}

VOID CAliveObject::SaySystem(const char* pszMsg, ...)//系统消息
{
	if (!CanRecvMsg())return;
	char szBuff[248];
	va_list	vl;
	va_start(vl, pszMsg);
	_vsnprintf(szBuff, sizeof(szBuff) - 1, pszMsg, vl);
	va_end(vl);
	szBuff[120] = '\0';
	SendMsg(GetId(), SM_SYSCHAT, 0x38ff, 0, 0, szBuff);
}

VOID CAliveObject::SaySystem10(const char* pszMsg, ...)//系统消息
{
	if (!CanRecvMsg())return;
	char szBuff[248];
	va_list	vl;
	va_start(vl, pszMsg);
	_vsnprintf(szBuff, sizeof(szBuff) - 1, pszMsg, vl);
	va_end(vl);
	szBuff[120] = '\0';
	SendMsg(GetId(), SM_SYSCHAT, 0, 0, 10, szBuff);
}

VOID CAliveObject::SaySystemAttrib(DWORD dwAttrib, const char* pszMsg, ...)
{
	if (!CanRecvMsg()) return;
	char szBuff[248];
	va_list	vl;
	va_start(vl, pszMsg);
	_vsnprintf(szBuff, sizeof(szBuff) - 1, pszMsg, vl);
	va_end(vl);
	szBuff[120] = '\0';
	SendMsg(GetId(), SM_SYSCHAT, LOWORD(dwAttrib), HIWORD(dwAttrib), 0, szBuff);
}

VOID CAliveObject::SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	if (!CanRecvMsg())return;
	CHAR* pBuffer = s_threadMessageBuffer;
	int size = 0;
	SmartEncodeMessage(pBuffer, size, dwFlag, wCmd, w1, w2, w3, lpdata, datasize);
	OnAroundMsg(this, pBuffer, size);
}

VOID CAliveObject::SendMsgToObject(CAliveObject* pReceiver, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	if (!pReceiver->CanRecvMsg())return;
	CHAR* pBuffer = s_threadMessageBuffer;
	int size = 0;
	SmartEncodeMessage(pBuffer, size, dwFlag, wCmd, w1, w2, w3, lpdata, datasize);
	pReceiver->OnAroundMsg(this, pBuffer, size);
}

VOID CAliveObject::SendMapMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	if (m_pMap == nullptr)return;
	CHAR* pBuffer = s_threadMessageBuffer;
	int size = 0;
	SmartEncodeMessage(pBuffer, size, dwFlag, wCmd, w1, w2, w3, lpdata, datasize);
	m_pMap->SendMsg(this, pBuffer, size);
}

// 设置要去到的坐标
BOOL CAliveObject::SetAction(actiontype action, e_direction dir, WORD x, WORD y, DWORD dwActionTime)
{
	if (this->m_pMap == nullptr) return FALSE;
	WORD wOldX = getX();
	WORD wOldY = getY();
	if (wOldX != x || wOldY != y)
	{
		if (!m_pMap->MoveObject(this, x, y)) return FALSE;
	}
	SetDirection(dir);
	m_ActionType = action;
	if (action != AT_STAND) SetPreActionType(action);
	m_wActionX = x;
	m_wActionY = y;
	m_dwActionCompleteTime = dwActionTime;
	m_ActionTimer.Savetime();
	OnDoAction(action);
	return TRUE;
}

BOOL CAliveObject::CompleteAction()
{
	if (m_pMap == nullptr)return FALSE;
	if (m_ActionType == AT_STAND)return TRUE;
	Stop(); //停止其他动作
	m_ActionType = AT_STAND;
	SetDirection(m_ActionDirection);
	if (GetType() == OBJ_PLAYER)
	{
		m_HpRecoverTimer.Savetime();
		m_MpRecoverTimer.Savetime();
	}
	return TRUE;
}

//离开视野
BOOL CAliveObject::GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	if (IsSystemFlagSeted(SF_HIDED)) return FALSE;
	SmartEncodeMessage(pszMsg, length, GetId(), SM_DISAPPEAR, m_wX, m_wY, 0, nullptr);
	return TRUE;
}

//获取视野范围内
BOOL CAliveObject::GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	if (IsSystemFlagSeted(SF_HIDED)) return FALSE;
	char szData[200] = { 0 };
	DWORD* pdwData = (DWORD*)szData;
	pdwData[0] = GetFeather();
	pdwData[1] = GetStatus();
	int dwHp = GetPropValue(PI_CURHP); // 当前生命值
	int dwMaxHp = GetPropValue(PI_MAXHP); // 最大生命值

	WORD wCmd = SM_APPEAR;
	if (m_bDead)
	{
		if (IsSystemFlagSeted(SF_BONE))
			wCmd = 0x21;
		else
			wCmd = SM_DEATH;
	}
	WORD w1 = 100;
	WORD wd = (dwMaxHp > 0) ? (dwHp * 100 / dwMaxHp) : 0; // 百分比取整
	pdwData[2] = static_cast<DWORD>(MAKELONG(wd, w1));
	int len = sprintf(szData + 12, "%s/%u", GetViewName(), GetNameColor(pViewer));
	// 在数据最后面插入15个空字节, 去过滤玩家、怪、宠物的一些独特封号
	memcpy(szData + 12, szData + 12, static_cast<size_t>(len) + 15);
	int totalLen = len + 15;
	SmartEncodeMessage(pszMsg, length, GetId(), wCmd, m_wX, m_wY, (GetSex() << 8) | m_Direction, (LPVOID)szData, 12 + totalLen);
	if (pViewer && pViewer->GetType() == OBJ_PLAYER)
		((CAliveObject*)pViewer)->SendFeatureChanged();
	return TRUE;
}

//42名字改变
VOID CAliveObject::SendChangeName()
{
	if (CanRecvMsg())
		SendMsg(GetId(), SM_SETPLAYERNAME, GetNameColor(this), 0, GetFenghaoType23(), (LPVOID)GetViewName());
	if (CSandCity::GetInstance()->IsWarStarted())
	{
		if (m_pMap == nullptr)return;
		xListHost<VISIBLE_OBJECT>::xListNode* pNode = nullptr;
		if (m_xVisibleObjectList.getCount())
		{
			pNode = m_xVisibleObjectList.getHead();
			while (pNode)
			{
				if (pNode->getObject()->pObject->GetType() == OBJ_PLAYER)
				{
					CAliveObject* p = (CAliveObject*)pNode->getObject()->pObject;
					p->SendMsg(GetId(), SM_SETPLAYERNAME, GetNameColor(p), 0, p->GetFenghaoType23(), (LPVOID)GetViewName());
				}
				pNode = pNode->getNext();
			}
		}
	}
	else
		SendAroundMsg(GetId(), SM_SETPLAYERNAME, GetNameColor(this), 0, GetFenghaoType23(), (LPVOID)GetViewName());
}

BOOL CAliveObject::CanDoAction(actiontype action)
{
	if (m_pMap == nullptr) return FALSE;
	if (IsStatusSet(SI_PALSY)) return FALSE;
	if (IsSystemFlagSeted(SF_FROZEN)) return FALSE;
	if (GetType() == OBJ_PLAYER && action == AT_ATTACK) // 玩家的判断
		return m_tmrAttack.IsTimeOut(g_dwActionDelay[AT_ATTACK] - 80 * GetPropValue(PI_ATTACKSPEED));
	return TRUE;
}

VOID CAliveObject::Update()
{
	//分帧更新
	const DWORD dwUpdateKey = GetUpdateKey();
	DWORD dwkey = (dwUpdateKey % 2);
	switch (dwkey)
	{
	case 0: // 系统标志、状态标志时间计算
	{
		BOOL fStatusChanged = FALSE;
		for (int i = 0; i < 32; i++)
		{
			if (m_Status.IsTimeOut(i)) // 状态的时间计算
			{
				DWORD dwParam = m_Status.GetParam(i);
				if (m_Status.ClrStatus(i))
				{
					OnStatusClr(i, dwParam);
					fStatusChanged = TRUE;
				}
			}
			if (m_SystemFlag.IsTimeOut(i))//系统标识时间计算
			{
				DWORD dwParam = m_SystemFlag.GetParam(i);
				m_SystemFlag.ClrStatus(i);
				OnSystemFlagCleared(i, dwParam);
			}
		}
		if (fStatusChanged) SendStatusChanged(); // 发送状态改变

		CAliveObject* pHitter = GetHitter(); // 攻击自己的对象
		if (pHitter != nullptr)
		{
			if (pHitter->IsDeath())
				SetHitter(nullptr);
			else if (pHitter->GetOwner() == this)
				SetHitter(nullptr);
			else if (!m_refObjHitter.IsValid())
				SetHitter(nullptr);
			else if (pHitter->GetMap() != m_pMap)
				SetHitter(nullptr);
		}
	}
	break;
	case 1: // 生命值、魔法值更新
	{
		if (!IsDeath())
		{
			BOOL bSendHpChanged = FALSE;
			BOOL bSendMpChanged = FALSE;
			int nHp = 0;
			if (GetAutoRecoverHptime() > 0 && CanRecover()) // 自动恢复血量
			{
				nHp = GetAutoRecoverHp();
				if (GetPropValue(PI_CURHP) < GetPropValue(PI_MAXHP) || nHp < 0)
				{
					if (m_HpRecoverTimer.IsTimeOut(GetAutoRecoverHptime()))
					{
						if (nHp != 0)
						{
							if (nHp > 0)
								AddPropValue(PI_CURHP, nHp);
							else
								DecPropValue(PI_CURHP, -nHp);
							bSendHpChanged = TRUE;
							m_HpRecoverTimer.Savetime();
						}
					}
				}
			}
			if (GetAutoRecoverMptime() > 0 && CanRecover()) // 自动恢复蓝量
			{
				int nMp = GetAutoRecoverMp();
				if (GetPropValue(PI_CURMP) < GetPropValue(PI_MAXMP) || nMp < 0)
				{
					if (m_MpRecoverTimer.IsTimeOut(GetAutoRecoverMptime()))
					{
						if (nMp != 0)
						{
							if (nMp > 0)
								AddPropValue(PI_CURMP, nMp);
							else
								DecPropValue(PI_CURMP, -nMp);
							bSendMpChanged = TRUE;
							m_MpRecoverTimer.Savetime();
						}
					}
				}
			}
			if (m_AddHpTimer.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_HPRECOVERTIME))) // 吃药增加HP
			{
				if (m_dwAddHp > 0)
				{
					if (GetPropValue(PI_CURHP) < GetPropValue(PI_MAXHP))
					{
						bSendHpChanged = TRUE;
						nHp = m_dwAddHp > m_dwAddHpSpeed ? m_dwAddHpSpeed : m_dwAddHp;
						AddPropValue(PI_CURHP, nHp);
						m_dwAddHp -= nHp;
					}
					else
						m_dwAddHp = 0;
				}
				m_AddHpTimer.Savetime();
			}
			if (m_AddMpTimer.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_MPRECOVERTIME))) // 吃药增加MP
			{
				if (m_dwAddMp > 0)
				{
					if (GetPropValue(PI_CURMP) < GetPropValue(PI_MAXMP))
					{
						bSendMpChanged = TRUE;
						DWORD dwAddMp = m_dwAddMp > m_dwAddMpSpeed ? m_dwAddMpSpeed : m_dwAddMp;
						AddPropValue(PI_CURMP, dwAddMp);
						m_dwAddMp -= dwAddMp;
					}
					else
						m_dwAddMp = 0;
				}
				m_AddMpTimer.Savetime();
			}
			if (bSendHpChanged)SendHpMpChanged(-nHp); // 发53封包
			if (bSendMpChanged)SendHpMpChanged();
		}
	}
	break;
	}

	if (m_ActionType != AT_STAND && m_dwActionCompleteTime != 0xffffffff)
	{
		if (m_ActionTimer.IsTimeOut(m_dwActionCompleteTime))
			CompleteAction();
	}
	// 队列对象线程处理
	OBJECTPROCESS* p = nullptr;
	DWORD dwCurTime = timeGetTime();
	int count = m_xQProcess.getcount();
	// 预分配空间, 避免动态扩容
	std::vector<OBJECTPROCESS*> tempVec;
	tempVec.reserve(count);
	for (int i = 0; i < count; i++)
	{
		p = m_xQProcess.pop();
		if (p == nullptr) continue;

		DWORD dwTimeDiff = GetTimeToTime(p->dwDeliverTime, dwCurTime);
		if (p->btFirstTime || (dwTimeDiff >= p->dwDelayTime))
		{
			p->btFirstTime = FALSE;
			DoProcess(p);
			if (p->repeattimes >= 0)
			{
				p->repeattimes--;
				if (p->repeattimes <= 0)
				{
					m_dwProcessFlags &= ~(1ULL << p->ident); // 清除标识
					CGameWorld::GetInstance()->FreeProcess(p);
					continue;
				}
			}
			p->dwDeliverTime = dwCurTime;
		}
		tempVec.push_back(p);
	}
	for (auto item : tempVec)
	{
		m_xQProcess.push(item);
	}
	CMapObject::Update();
}

VOID CAliveObject::OnLeaveMap(CLogicMap* pMap)
{
	if (pMap != nullptr)
	{
		if (m_bPosLocked)
		{
			pMap->UnLockPos(m_wX, m_wY);
			m_bPosLocked = FALSE;
		}
	}
	CleanVisibleList();
	CMapObject::OnLeaveMap(pMap);
}

VOID CAliveObject::CleanVisibleList()
{
	//	断开自己的可视连接
	if (m_xVisibleObjectList.getCount() > 0)
	{
		xListHost<VISIBLE_OBJECT>::xListNode* pNode = nullptr;
		while ((pNode = m_xVisibleObjectList.getHead()) != nullptr)
		{
			VISIBLE_OBJECT* pVisibleObj = pNode->getObject();
			if (pVisibleObj && pVisibleObj->pObject)
			{
				CAliveObject* pAliveObj = static_cast<CAliveObject*>(pVisibleObj->pObject);
				if (pAliveObj)
					pAliveObj->RemoveVisibleObject(this);
			}
			m_xVisibleObjectList.removeNode(pNode);
			DeleteVisibleObject(pVisibleObj);
		}
	}
	if (m_xVisibleItemsList.getCount() > 0)
	{
		xListHost<VISIBLE_OBJECT>::xListNode* pNode = nullptr;
		while ((pNode = m_xVisibleItemsList.getHead()) != nullptr)
		{
			VISIBLE_OBJECT* pVisibleObj = pNode->getObject();
			m_xVisibleItemsList.removeNode(pNode);
			DeleteVisibleObject(pVisibleObj);
		}
	}
	//	断开别人对自己的单向连接
	if (m_pMap == nullptr) return;
	int mw = m_pMap->GetWidth();
	int mh = m_pMap->GetHeight();
	int dx = getX(), dy = getY();
	int nRange = VIEW_RANGE + 2;
	int startx = MAX(0, dx - nRange);
	int endx = MIN(mw - 1, dx + nRange);
	int starty = MAX(0, dy - nRange);
	int endy = MIN(mh - 1, dy + nRange);

	CMapCellInfo** pCellInfoBase = m_pMap->GetCellInfoBase();
	if (pCellInfoBase == nullptr)return;
	// 双指针：行指针遍历x轴, 列指针遍历y轴
	for (int tx = startx; tx <= endx; tx++)
	{
		CMapCellInfo** pCol = pCellInfoBase + tx + starty * mw;
		for (int ty = starty; ty <= endy; ty++)
		{
			CMapCellInfo* pInfo = *pCol;
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CMapObject* pObject = pNode->getObject();
					//	寻找周围能看到自己的ALIVEOBJECT
					if (pObject && pObject != this && (pObject->GetClassType() == CLS_ALIVEOBJECT || pObject->GetClassType() == CLS_NPC))
					{
						if ((((CAliveObject*)pObject)->GetVisibleObjectFlag() & (1 << GetType())) != 0)
						{
							//	从它们身上的列表中删除自己
							((CAliveObject*)pObject)->RemoveVisibleObject(this);
						}
					}
					pNode = pNode->getNext();
				}
			}
			pCol += mw;  // 移动到下一行的同一列
		}
	}
}

VOID CAliveObject::OnEnterMap(CLogicMap* pMap)
{
	SendMsg(GetId(), SM_SETPLAYERNAME, GetNameColor(this), 0, GetFenghaoType23(), (LPVOID)GetViewName());
	SendMsg(pMap->IsFlagSeted(MF_FIGHTMAP), 0x2c4, 0, 0, 0);//708 设置地图的属性 dwflag = 1 战斗,  dwflag = 0 非战斗
	SendMsg(0, SM_SETMAPNAME, pMap->GetMusicId(), 0, 0, (LPVOID)pMap->GetTitle());//地图编号
	CMapObject::OnEnterMap(pMap);
	if (!m_bPosLocked)
		m_bPosLocked = pMap->LockPos(getX(), getY());
	m_Mapid = pMap->GetIndex();
	// 进入地图的时候重新搜索下周围的环境
	SearchViewRange();
}

VOID CAliveObject::ResetPos(WORD x, WORD y)
{
	DWORD dwdata[3] = { GetFeather(), GetStatus(), 0 };
	CHAR* pBuffer = s_threadMessageBuffer;
	int size = 0;
	SmartEncodeMessage(pBuffer, size, GetId(), SM_BACK, x, y, (WORD)GetDirection(), (LPVOID)dwdata, sizeof(DWORD) * 2 + sizeof(WORD));
	if (CanRecvMsg())
		OnAroundMsg(this, pBuffer, size);
	SetAction(AT_BACK, m_Direction, x, y, 1);
}

// 物品数据转换成客户端结构
VOID CAliveObject::ItemToClient(ITEM& item)
{
	if (item.baseitem.btStdMode == 49 && item.baseitem.btShape == 51) // 豹魔石
	{
		*(BYTE*)&item.btItemExt[16] = 0; // 资质
		*(BYTE*)&item.btItemExt[17] = 255; // 职业
		*(WORD*)&item.btItemExt[18] = item.baseitem.wMc; // 显示的当前经验值
		*(WORD*)&item.btItemExt[42] = item.baseitem.wDc; // 显示的等级
		*(DWORD*)&item.btItemExt[73] = CItemManager::GetInstance()->GetPetLevelInfo(item.baseitem.wDc, 0); // 升级经验值
		*(WORD*)&item.btItemExt[99] = item.baseitem.wDc; // 等级
		*(WORD*)&item.btItemExt[107] = item.baseitem.wMc; // 当前经验
	}
}

VOID CAliveObject::UpdateVisibleObject(CMapObject* pObject)
{
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = nullptr;
	VISIBLE_OBJECT_LIST* pList = nullptr;
	if (pObject->GetClassType() == CLS_ALIVEOBJECT || pObject->GetClassType() == CLS_NPC)
		pList = &m_xVisibleObjectList;
	else
		pList = &m_xVisibleItemsList;
	if (pList->getCount())
	{
		pNode = pList->getHead();
		while (pNode)
		{
			if (pNode->getObject()->pObject == pObject)
				return;
			pNode = pNode->getNext();
		}
	}
	AddVisibleObject(pObject);
	//	如果对方是个ALIVEOBJECT, 并且对方的可以看到自己, 就把自己加入到对方的表中
	if (pList == &m_xVisibleObjectList && (((CAliveObject*)pObject)->GetVisibleObjectFlag() & (1 << GetType())) != 0)
		((CAliveObject*)pObject)->UpdateVisibleObject(this);
}

VOID CAliveObject::AddVisibleObject(CMapObject* pObject)
{
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = nullptr;
	VISIBLE_OBJECT_LIST* pList = nullptr;
	if (pObject->GetClassType() == CLS_ALIVEOBJECT || pObject->GetClassType() == CLS_NPC)
		pList = &m_xVisibleObjectList;
	else
		pList = &m_xVisibleItemsList;
	VISIBLE_OBJECT* p = NewVisibleObject();
	if (p == nullptr)return;
	pObject->AddRef();
	p->pObject = pObject;
	if (!pList->addNode(&p->Node))
	{
		DeleteVisibleObject(p);
		return;
	}
	char szMsgBuffer[1024];
	int length = 1024;
	if (CanRecvMsg() && pObject->GetViewmsg(szMsgBuffer, length, this))
		OnAroundMsg(pObject, szMsgBuffer, length);
	if (pObject->GetClassType() != CLS_ALIVEOBJECT && pObject->GetClassType() != CLS_NPC)
		return;
	e_object_type pType = GetType();
	e_object_ApprType pApprType = GetApprType();
	e_object_type pPType = pObject->GetType();
	e_object_ApprType pPApprType = pObject->GetApprType();
	BOOL IsSpecial = FALSE;
	if (pType == OBJ_MONSTER) // 判断自己是否是特殊刷怪
		IsSpecial = ((CMonsterEx*)this)->IsSpecialGen();
	if ((pType == OBJ_PLAYER || pType == OBJ_PET || pApprType == APPR_GUARD || IsSpecial) && (pPType == OBJ_PET || pPType == OBJ_MONSTER || pPApprType == APPR_GUARD))
		CGameWorld::GetInstance()->AddUpdateMonster((CMonsterEx*)pObject);
}

VOID CAliveObject::RemoveVisibleObject(CMapObject* pObject)
{
	if (pObject == nullptr) return;
	VISIBLE_OBJECT_LIST* pList = nullptr;
	if (pObject->GetClassType() == CLS_ALIVEOBJECT || pObject->GetClassType() == CLS_NPC)
		pList = &m_xVisibleObjectList;
	else
		pList = &m_xVisibleItemsList;
	if (pList->getCount() == 0) return;
	char s_szMsgBuffer[1024];
	int length = 1024;
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = pList->getHead();
	while (pNode)
	{
		if (pNode->getObject()->pObject == pObject)
		{
			pList->removeNode(pNode);
			if (CanRecvMsg() && pObject->GetOutViewmsg(s_szMsgBuffer, length, this))
				OnAroundMsg(pObject, s_szMsgBuffer, length);
			DeleteVisibleObject(pNode->getObject());
			return;
		}
		pNode = pNode->getNext();
	}
}

//	进入地图的时候重新搜索下周围的环境
VOID CAliveObject::SearchViewRange()
{
	CLogicMap* pMap = (CLogicMap*)GetMap();
	if (pMap == nullptr)return;

	int mw = pMap->GetWidth();
	int mh = pMap->GetHeight();
	int dx = getX(), dy = getY();
	int nRange = VIEW_RANGE + 2;
	int startx = MAX(0, dx - nRange);
	int endx = MIN(mw - 1, dx + nRange);
	int starty = MAX(0, dy - nRange);
	int endy = MIN(mh - 1, dy + nRange);

	CMapCellInfo** pCellInfoBase = pMap->GetCellInfoBase();
	if (pCellInfoBase == nullptr)return;

	// 双指针：行指针遍历x轴, 列指针遍历y轴
	for (int tx = startx; tx <= endx; tx++)
	{
		CMapCellInfo** pCol = pCellInfoBase + tx + starty * mw;
		for (int ty = starty; ty <= endy; ty++)
		{
			CMapCellInfo* pInfo = *pCol;
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				this->UpdateViewObjectList(&pInfo->m_xObjectList);
			}
			pCol += mw;  // 移动到下一行的同一列
		}
	}
}

//创建怪物时刷新
VOID CAliveObject::RefreshViewList()
{
	CLogicMap* pMap = (CLogicMap*)GetMap();
	if (pMap == nullptr) return;

	char s_szMsgBuffer[1024];
	int length = 1024;
	BOOL bRecvMsg = CanRecvMsg();
	DWORD dwVisibleFlag = m_nVisibleObjectFlag;
	e_object_type myType = GetType();

	// 清理自己看不到的东西
	xListHost<VISIBLE_OBJECT>::xListNode* pNode = nullptr;
	xListHost<VISIBLE_OBJECT>::xListNode* pNext = nullptr;
	CMapObject* pObject = nullptr;

	if (m_xVisibleObjectList.getCount() > 0)
	{
		pNode = m_xVisibleObjectList.getHead();
		while (pNode)
		{
			pNext = pNode->getNext();
			pObject = pNode->getObject()->pObject;
			if ((dwVisibleFlag & (1 << pObject->GetType())) == 0)
			{
				m_xVisibleObjectList.removeNode(pNode);
				if (bRecvMsg && pObject->GetOutViewmsg(s_szMsgBuffer, length, this))
					OnAroundMsg(pObject, s_szMsgBuffer, length);
				((CAliveObject*)pObject)->RemoveVisibleObject(this);
			}
			pNode = pNext;
		}
	}

	if (m_xVisibleItemsList.getCount() > 0)
	{
		pNode = m_xVisibleItemsList.getHead();
		while (pNode)
		{
			pNext = pNode->getNext();
			pObject = pNode->getObject()->pObject;
			if ((dwVisibleFlag & (1 << pObject->GetType())) == 0)
			{
				m_xVisibleItemsList.removeNode(pNode);
				if (bRecvMsg && pObject->GetOutViewmsg(s_szMsgBuffer, length, this))
					OnAroundMsg(pObject, s_szMsgBuffer, length);
			}
			pNode = pNext;
		}
	}

	// 刷新自己能看到的东西, 顺便刷新能看到自己的东西
	int mw = pMap->GetWidth();
	int mh = pMap->GetHeight();
	int dx = getX(), dy = getY();
	int nRange = VIEW_RANGE;
	int startx = MAX(0, dx - nRange);
	int endx = MIN(mw - 1, dx + nRange);
	int starty = MAX(0, dy - nRange);
	int endy = MIN(mh - 1, dy + nRange);

	CMapCellInfo** pCellInfoBase = pMap->GetCellInfoBase();
	if (pCellInfoBase == nullptr)return;
	// 双指针：行指针遍历x轴, 列指针遍历y轴
	for (int tx = startx; tx <= endx; tx++)
	{
		CMapCellInfo** pCol = pCellInfoBase + tx + starty * mw;
		for (int ty = starty; ty <= endy; ty++)
		{
			CMapCellInfo* pInfo = *pCol;
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pCellNode = pInfo->m_xObjectList.getHead();
				while (pCellNode)
				{
					pObject = pCellNode->getObject();
					if (pObject && pObject != this)
					{
						// 如果自己可以看到这个东西, 就更新下
						if ((dwVisibleFlag & (1 << pObject->GetType())) != 0)
							UpdateVisibleObject(pObject);
						// 如果这个东西可以看到自己, 就更新下
						if ((pObject->GetClassType() == CLS_ALIVEOBJECT || pObject->GetClassType() == CLS_NPC) &&
							(((CAliveObject*)pObject)->GetVisibleObjectFlag() & (1 << myType)) != 0)
						{
							((CAliveObject*)pObject)->UpdateVisibleObject(this);
						}
					}
					pCellNode = pCellNode->getNext();
				}
			}
			pCol += mw;  // 移动到下一行的同一列
		}
	}
}

inline static void ClipRects(RECT* rcs, int count, int mw, int mh)
{
	for (int i = 0; i < count; i++)
	{
		rcs[i].left = max(0, rcs[i].left);
		rcs[i].right = min(mw, rcs[i].right);
		rcs[i].top = max(0, rcs[i].top);
		rcs[i].bottom = min(mh, rcs[i].bottom);
	}
}

inline static int GetOutRect(RECT& rcMain, RECT& rcTest, RECT* prcArray)
{
	int count = 0;
	RECT intersec;
	if (!IntersectRect(&intersec, &rcMain, &rcTest))
		return 0;
	if (intersec.left == rcTest.left)
	{
		if (intersec.right != rcTest.right)
		{
			prcArray[count].top = rcTest.top;
			prcArray[count].bottom = rcTest.bottom;
			prcArray[count].left = intersec.right + 1;
			prcArray[count].right = rcTest.right;
			count++;
		}
	}
	else if (intersec.right == rcTest.right)
	{
		prcArray[count].top = rcTest.top;
		prcArray[count].bottom = rcTest.bottom;
		prcArray[count].left = rcTest.left;
		prcArray[count].right = intersec.left - 1;
		count++;
	}
	if (intersec.top == rcTest.top)
	{
		if (intersec.bottom != rcTest.bottom)
		{
			prcArray[count].left = intersec.left;
			prcArray[count].right = intersec.right;
			prcArray[count].top = intersec.bottom + 1;
			prcArray[count].bottom = rcTest.bottom;
			count++;
		}
	}
	else if (intersec.bottom == rcTest.bottom)
	{
		prcArray[count].left = intersec.left;
		prcArray[count].right = intersec.right;
		prcArray[count].top = rcTest.top;
		prcArray[count].bottom = intersec.top - 1;
		count++;
	}
	return count;
}

//跑和走的时候刷新
VOID CAliveObject::UpdateViewRange(UINT ox, UINT oy)
{
	if (m_pMap == nullptr) return;
	int nx = getX(), ny = getY();
	if (nx == ox && ny == oy) return;
	int mw = m_pMap->GetWidth() - 1, mh = m_pMap->GetHeight() - 1;
	int nRange = VIEW_RANGE + 2;
	RECT rc1 = { ox - nRange, oy - nRange, ox + nRange, oy + nRange };
	RECT rc2 = { nx - nRange, ny - nRange, nx + nRange, ny + nRange };
	RECT rcs[4];

	int rccount = GetOutRect(rc1, rc2, rcs);
	if (rccount == 0)
	{
		CleanVisibleList();
		SearchViewRange();
		return;
	}

	int mapWidth = m_pMap->GetWidth();
	CMapCellInfo** pCellInfoBase = m_pMap->GetCellInfoBase();
	// 预先裁剪所有矩形
	ClipRects(rcs, rccount, mw, mh);
	// 新进入视距的对象
	for (int i = 0; i < rccount; i++)
	{
		for (int y = rcs[i].top; y <= rcs[i].bottom; y++)
		{
			int rowBaseIndex = y * mapWidth;
			for (int x = rcs[i].left; x <= rcs[i].right; x++)
			{
				CMapCellInfo* pViewInfo = pCellInfoBase[rowBaseIndex + x];
				if (pViewInfo && pViewInfo->m_xObjectList.getCount() > 0)
					UpdateViewObjectList(&pViewInfo->m_xObjectList);
			}
		}
	}
	// 离开视距的对象
	rccount = GetOutRect(rc2, rc1, rcs);
	if (rccount > 0)
	{
		bool hasVisibleObjects = m_xVisibleItemsList.getCount() > 0 || m_xVisibleObjectList.getCount() > 0;
		if (hasVisibleObjects)
		{
			// 预先裁剪所有矩形
			ClipRects(rcs, rccount, mw, mh);
			for (int i = 0; i < rccount; i++)
			{
				for (int y = rcs[i].top; y <= rcs[i].bottom; y++)
				{
					int rowBaseIndex = y * mapWidth;
					for (int x = rcs[i].left; x <= rcs[i].right; x++)
					{
						CMapCellInfo* pViewInfo = pCellInfoBase[rowBaseIndex + x];
						if (pViewInfo && pViewInfo->m_xObjectList.getCount() > 0)
							UpdateOutViewObjectList(&pViewInfo->m_xObjectList);
					}
				}
			}
		}
	}
}

VOID CAliveObject::UpdateViewObjectList(xListHost<CMapObject>* pList)
{
	xListHost<CMapObject>::xListNode* pNode = pList->getHead();
	while (pNode)
	{
		CMapObject* pObject = pNode->getObject();
		if (pObject && pObject != this)
		{
			if ((m_nVisibleObjectFlag & (1 << pObject->GetType())))
				UpdateVisibleObject(pObject);
			if ((pObject->GetClassType() == CLS_ALIVEOBJECT ||
				pObject->GetClassType() == CLS_NPC) && (((CAliveObject*)pObject)->GetVisibleObjectFlag() & (1 << GetType())) != 0)
				((CAliveObject*)pObject)->UpdateVisibleObject(this);
		}
		pNode = pNode->getNext();
	}
}

//更新离开列表
VOID CAliveObject::UpdateOutViewObjectList(xListHost<CMapObject>* pList)
{
	xListHost<CMapObject>::xListNode* pNode = pList->getHead();
	while (pNode)
	{
		CMapObject* pObject = pNode->getObject();
		if (pObject && pObject != this)
		{
			if ((m_nVisibleObjectFlag & (1 << pObject->GetType())) != 0)
				RemoveVisibleObject(pObject);
			if ((pObject->GetClassType() == CLS_ALIVEOBJECT ||
				pObject->GetClassType() == CLS_NPC) && (((CAliveObject*)pObject)->GetVisibleObjectFlag() & (1 << GetType())) != 0)
				((CAliveObject*)pObject)->RemoveVisibleObject(this);
		}
		pNode = pNode->getNext();
	}
}

VOID CAliveObject::UpdateMonster(UINT ox, UINT oy)
{
	if (m_pMap == nullptr) return;
	int nx = getX(), ny = getY();
	if (nx == ox && ny == oy) return;
	// 只对玩家和宠物更新怪物
	if (GetType() != OBJ_PLAYER && GetType() != OBJ_PET)
		return;
	int mw = m_pMap->GetWidth();
	int mh = m_pMap->GetHeight();
	// 限制边界范围, 避免不必要的计算
	int startX = MAX(MAX(ox, nx) - 2, 0);
	int endX = MIN(MAX(ox, nx) + 2, mw - 1);
	int startY = MAX(MAX(oy, ny) - 2, 0);
	int endY = MIN(MAX(oy, ny) + 2, mh - 1);
	// 如果完全重合（位置未改变）, 无需更新
	if (startX == ox - 2 && endX == ox + 2 && startY == oy - 2 && endY == oy + 2)
		return;

	CMapCellInfo** pCellInfoBase = m_pMap->GetCellInfoBase();
	if (pCellInfoBase == nullptr) return;

	// 双指针：行指针遍历x轴, 列指针遍历y轴
	for (int x = startX; x <= endX; x++)
	{
		CMapCellInfo** pCol = pCellInfoBase + x + startY * mw;
		for (int y = startY; y <= endY; y++)
		{
			CMapCellInfo* pViewInfo = *pCol;
			if (pViewInfo)
			{
				for (auto pNode = pViewInfo->m_xObjectList.getHead(); pNode != nullptr; pNode = pNode->getNext())
				{
					CMonsterEx* pMonster = dynamic_cast<CMonsterEx*>(pNode->getObject());
					if (pMonster && !pMonster->IsDeath())
						pMonster->Update();
				}
			}
			pCol += mw;  // 移动到下一行的同一列
		}
	}
}

VOID CAliveObject::SendAroundMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	if (m_pMap == nullptr) return;
	if (IsSystemFlagSeted(SF_HIDED)) return;
	if (m_xVisibleObjectList.getCount() == 0) return;

	CHAR* pBuffer = s_threadMessageBuffer;
	int size = 0;
	SmartEncodeMessage(pBuffer, size, dwFlag, wCmd, w1, w2, w3, lpdata, datasize);
	auto* pNode = m_xVisibleObjectList.getHead();
	while (pNode) {
		CAliveObject* pTarget = static_cast<CAliveObject*>(pNode->getObject()->pObject);
		if (pTarget && pTarget->CanRecvMsg())
			pTarget->OnAroundMsg(this, pBuffer, size);
		pNode = pNode->getNext();
	}
}

VOID CAliveObject::SendAroundMsg(const char* szMsg, int length)
{
	if (m_pMap == nullptr) return;
	if (IsSystemFlagSeted(SF_HIDED)) return;
	if (!szMsg || length <= 0) return;
	if (m_xVisibleObjectList.getCount() == 0) return;
	auto* pNode = m_xVisibleObjectList.getHead();
	while (pNode) {
		CAliveObject* pTarget = static_cast<CAliveObject*>(pNode->getObject()->pObject);
		if (pTarget && pTarget->CanRecvMsg())
			pTarget->OnAroundMsg(this, szMsg, length);
		pNode = pNode->getNext();
	}
}

VOID CAliveObject::ToDeath(DWORD dwKiller)
{
	if (!m_bDead)
	{
		SetTarget(nullptr);//死亡时, 设置目标为空
		if (m_ActionType != AT_STAND)
			CompleteAction();
		if (m_bPosLocked)
			m_pMap->UnLockPos(m_wX, m_wY);
		m_bDead = TRUE;
		DWORD dwArray[2] = { GetFeather(), GetStatus() };
		SendAroundMsg(GetId(), SM_NOWDEATH, getX(), getY(), GetDirection(), (LPVOID)dwArray, sizeof(dwArray));
		if (CanRecvMsg())
			SendMsg(GetId(), SM_NOWDEATH, getX(), getY(), GetDirection(), (LPVOID)dwArray, sizeof(dwArray));
		OnDeath(dwKiller);
	}
}

BOOL CAliveObject::AddProcess(e_process ident, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3, DWORD dwParam4, DWORD dwDelay, int repeattimes, const char* pszString, BOOL firstTime)
{
	OBJECTPROCESS* p = CGameWorld::GetInstance()->AllocProcess(pszString);
	if (p == nullptr)return FALSE;
	if (!m_xQProcess.push(p))
	{
		CGameWorld::GetInstance()->FreeProcess(p);
		return FALSE;
	}
	p->dwDelayTime = dwDelay;
	p->dwDeliverTime = timeGetTime();
	p->dwParam[0] = dwParam1;
	p->dwParam[1] = dwParam2;
	p->dwParam[2] = dwParam3;
	p->dwParam[3] = dwParam4;
	p->repeattimes = repeattimes;
	p->btFirstTime = firstTime;
	p->ident = ident;
	if (ident < E_MAX) { m_dwProcessFlags |= (1ULL << ident); } // 用位计算是否有标识
	return TRUE;
}

BOOL CAliveObject::AddProcess(OBJECTPROCESS* pProcess)
{
	OBJECTPROCESS* pNewProcess = CGameWorld::GetInstance()->AllocProcess(pProcess->pszParam);
	if (pNewProcess == nullptr)return FALSE;
	if (!m_xQProcess.push(pNewProcess))
	{
		CGameWorld::GetInstance()->FreeProcess(pNewProcess);
		return FALSE;
	}
	pNewProcess->dwDelayTime = pProcess->dwDelayTime;
	pNewProcess->dwDeliverTime = pProcess->dwDeliverTime;
	pNewProcess->dwParam[0] = pProcess->dwParam[0];
	pNewProcess->dwParam[1] = pProcess->dwParam[1];
	pNewProcess->dwParam[2] = pProcess->dwParam[2];
	pNewProcess->dwParam[3] = pProcess->dwParam[3];
	pNewProcess->ident = pProcess->ident;
	pNewProcess->iStringSize = pProcess->iStringSize;
	pNewProcess->repeattimes = pProcess->repeattimes;
	pNewProcess->btFirstTime = pProcess->btFirstTime;
	e_process ident = pNewProcess->ident;
	if (ident < E_MAX) { m_dwProcessFlags |= (1ULL << ident); } // 用位计算是否有标识
	return TRUE;
}

BOOL CAliveObject::IsProcess(e_process ident) const
{
	return (m_dwProcessFlags & (1ULL << ident)) != 0;
}

VOID CAliveObject::DoProcess(OBJECTPROCESS* pProcess)
{
	switch (pProcess->ident)
	{
	case EP_NOTIFYCHANGENAME:
	{
		SendChangeName();
	}
	break;
	case EP_SETSYSTEMFLAG:
	{
		SetSystemFlag(pProcess->dwParam[0], pProcess->dwParam[1], pProcess->dwParam[2], pProcess->dwParam[3]);
	}
	break;
	case EP_ENTERMAP://确认地图
	{
		CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(pProcess->dwParam[0]);
		setXY((WORD)pProcess->dwParam[1], (WORD)pProcess->dwParam[2]);
		pMap->AddObject(this);
	}
	break;
	case EP_REFRESHFEATURE://删除对象
	{
		AddProcess(EP_ENTERMAP, GetMapId(), getX(), getY(), 0, 1000, 1);
		m_pMap->RemoveObject(this);
	}
	break;
	case EP_SETSTATUS:
	{
		SetStatus(pProcess->dwParam[0], pProcess->dwParam[1], pProcess->dwParam[2]);
	}
	break;
	case EP_CLRSTATUS:
	{
		ClrStatus(pProcess->dwParam[0]);
	}
	break;
	case EP_MAGHEALING:
	{
		DWORD dwRecoverPoint = pProcess->dwParam[1];
		if (dwRecoverPoint == 0)
			dwRecoverPoint = CGameWorld::GetInstance()->GetVar(EVI_HPRECOVERPOINT);
		SetAddHp(pProcess->dwParam[0], dwRecoverPoint);
	}
	break;
	case EP_RUSH://野蛮冲撞
	{
		int tx = getX();
		int ty = getY();
		int i = 0;

		DWORD rg = pProcess->dwParam[0];
		int nGrid = rg & 0xff;
		int nLevel = (rg & 0xff00) >> 8;
		int nRushGridDelay = (rg & 0xffff0000) >> 16;
		int nDamage1 = pProcess->dwParam[1];
		int nDamage2 = pProcess->dwParam[2];
		DWORD dd = pProcess->dwParam[3];
		int dir = dd & 0xffff;
		int nDamage3 = (dd & 0xffff0000) >> 16;

		CAliveObject* pObject = GetTarget();
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
		//如果需要多次移动, 则切换目标
		if (nLevel == 3)
		{
			GETNEXTPOS(tx, ty, dir);
			pFrontObject2 = (CAliveObject*)m_pMap->FindTarget(this, tx, ty);
			if (pFrontObject2 == nullptr || (pFrontObject2 && pFrontObject2->IsDeath()) || (pFrontObject2 && !pFrontObject2->CanBePushed(this)))
				pFrontObject2 = nullptr;
		}
		if (pFrontObject && !pFrontObject->IsDeath() && pFrontObject->CanBePushed(this))
		{
			for (i = 0; i < nGrid; i++)
			{
				if (!pFrontObject->DoPushed(dir))
				{
					if (i > 0)
						break;
					else
					{
						DoRush(dir, 0);
						SaySystemAttrib(CC_MAGICTIPS, "冲撞失败!");
						return;
					}
				}
				if (pFrontObject2 && !pFrontObject2->DoPushed(dir))
				{
					if (i > 0)
						break;
					else
					{
						DoRush(dir, 0);
						return;
					}
				}
				if (!DoRush(dir, 0))
				{
					SaySystemAttrib(CC_MAGICTIPS, "缺乏冲撞力!");
					return;
				}
			}
			if (pFrontObject->GetType() == OBJ_PLAYER)
				pFrontObject->SetAction(AT_BEATTACK, pFrontObject->GetDirection(), pFrontObject->getX(), pFrontObject->getY(), nRushGridDelay * i);
			if (pFrontObject2 && pFrontObject2->GetType() == OBJ_PLAYER)
				pFrontObject2->SetAction(AT_BEATTACK, pFrontObject2->GetDirection(), pFrontObject2->getX(), pFrontObject2->getY(), nRushGridDelay * i);
			pFrontObject->BeAttack(this, nDamage1, DT_DIRECT);
			if (pFrontObject2) pFrontObject2->BeAttack(this, nDamage2, DT_DIRECT);

			pFrontObject->m_tmrAttack.Savetime();
			if (pFrontObject2)pFrontObject2->m_tmrAttack.Savetime();
		}
		else
		{
			for (i = 0; i < nGrid; i++)
			{
				if (!DoRush(dir, 0))
				{
					SaySystemAttrib(CC_MAGICTIPS, "缺乏冲撞力!");
					break;
				}
			}
			int x, y;
			GetFrontPosition(x, y);
			if (m_pMap && m_pMap->IsPhysicsBlocked(x, y)) // 撞墙时, 自身掉血
				BeAttack(this, nDamage3, DT_DIRECT);
		}
	}
	break;
	case EP_RUSHHIT://突斩
	{
		int tx = getX();
		int ty = getY();
		int i = 0;

		DWORD rg = pProcess->dwParam[0];
		int nGrid = rg & 0xff;
		int nLevel = (rg & 0xff00) >> 8;
		int nRushGridDelay = (rg & 0xffff0000) >> 16;
		int nDamage1 = pProcess->dwParam[1];
		int nDamage2 = pProcess->dwParam[2];
		DWORD dd = pProcess->dwParam[3];
		int dir = dd & 0xffff;
		int nDamage3 = (dd & 0xffff0000) >> 16;

		CAliveObject* pObject = GetTarget();
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
		if (nLevel == 3)
		{
			GETNEXTPOS(tx, ty, dir);
			pFrontObject2 = (CAliveObject*)m_pMap->FindTarget(this, tx, ty);
			if (pFrontObject2 == nullptr || (pFrontObject2 && pFrontObject2->IsDeath()) || (pFrontObject2 && !pFrontObject2->CanBePushed(this)))
				pFrontObject2 = nullptr;
		}
		if (pFrontObject && !pFrontObject->IsDeath() && pFrontObject->CanBePushed(this))
		{
			for (i = 0; i < nGrid; i++)
			{
				if (!pFrontObject->DoPushed(dir)) 
				{
					if (i > 0)
						break;
					else
					{
						DoRush(dir, 2);
						SaySystemAttrib(CC_MAGICTIPS, "冲撞失败!");
						return;
					}
				}
				if (pFrontObject2 && !pFrontObject2->DoPushed(dir))
				{
					if (i > 0)
						break;
					else
					{
						DoRush(dir, 2);
						return;
					}
				}
				if (!DoRush(dir, 2))
				{
					SaySystemAttrib(CC_MAGICTIPS, "缺乏冲撞力!");
					return;
				}
			}
			if (pFrontObject->GetType() == OBJ_PLAYER)
				pFrontObject->SetAction(AT_BEATTACK, pFrontObject->GetDirection(), pFrontObject->getX(), pFrontObject->getY(), nRushGridDelay * i);
			if (pFrontObject2 && pFrontObject2->GetType() == OBJ_PLAYER)
				pFrontObject2->SetAction(AT_BEATTACK, pFrontObject2->GetDirection(), pFrontObject2->getX(), pFrontObject2->getY(), nRushGridDelay * i);
			pFrontObject->BeAttack(this, nDamage1, DT_DIRECT);
			if (pFrontObject2) pFrontObject2->BeAttack(this, nDamage2, DT_DIRECT);

			pFrontObject->m_tmrAttack.Savetime();
			if (pFrontObject2)pFrontObject2->m_tmrAttack.Savetime();
		}
		else
		{
			for (i = 0; i < nGrid; i++)
			{
				if (!DoRush(dir, 2))
				{
					SaySystemAttrib(CC_MAGICTIPS, "缺乏冲撞力!");
					break;
				}
			}
			int x, y;
			GetFrontPosition(x, y);
			if (m_pMap && m_pMap->IsPhysicsBlocked(x, y)) // 撞墙时, 自身掉血
				BeAttack(this, nDamage3, DT_DIRECT);
		}
	}
	break;
	case EP_FIRERAIN: // 流星火雨-火云
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		UINT nRange = pProcess->dwParam[1];
		UINT nDamage = pProcess->dwParam[2];
		UINT nKeepTime = pProcess->dwParam[3];
		FireRainEvent::Create(this, x, y, nRange, nDamage, nKeepTime * 1000, 2000);
	}
	break;
	case EP_DEAD://对象死亡
	{
		ToDeath(pProcess->dwParam[0]);
	}
	break;
	case EP_BEATTACKED://受到攻击
	{
		CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(pProcess->dwParam[1]);//获取攻击者对象
		BeAttack(pObject, (int)pProcess->dwParam[0], (damage_type)LOWORD(pProcess->dwParam[2]), pProcess->dwParam[3], (damage_ReType)HIWORD(pProcess->dwParam[2]));
	}
	break;
	case EP_CHANGEMAP://从连接点切换地图
	{
		FlyTo(pProcess->dwParam[0], pProcess->dwParam[1], pProcess->dwParam[2], FALSE);
	}
	break;
	}
}

BOOL CAliveObject::FlyTo(int mapid, int x, int y, BOOL bShowEffect)
{
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
	return FlyTo(pMap, x, y, bShowEffect);
}

BOOL CAliveObject::FlyTo(CLogicMap* pMap, int x, int y, BOOL bShowEffect)
{
	if (pMap == nullptr) return FALSE;
	if (!CanEnterMap(pMap)) return FALSE;
	// 是摆摊状态无法FLYTO
	if (this->m_ActionType == AT_PRIVATESHOP || IsDeath())
		return FALSE;
	// 如果是宠物, 与玩家飞到同一个坐标
	if (GetType() != OBJ_PET && pMap->IsBlocked(x, y)) //判断移动点是否锁定
	{
		POINT pt;
		if (pMap->GetValidPoint(x, y, &pt, 1))
			x = pt.x, y = pt.y;
	}
	CLogicMap* pOldMap = m_pMap;
	if (!pOldMap->RemoveObject(this))//如果删除对象失败
		return FALSE;
	BOOL bSendMsg = CanRecvMsg();//能否接收信息
	if (bSendMsg)
	{
		SendMsg(0, SM_CLEAROBJECTS, 0, 0, 0);
		SendMsg(GetId(), SM_CHANGEMAP, x, y, 0, (LPVOID)pMap->GetName());
	}

	int ox = getX();
	int oy = getY();
	setXY(x, y);
	if (!pMap->AddObject(this))
	{
		setXY(ox, oy);
		SendMsg(GetId(), SM_CHANGEMAP, ox, oy, 0, (LPVOID)pOldMap->GetName());
		pOldMap->AddObject(this);
		return FALSE;
	}
	SetPreActionType(AT_FLY);
	if (bSendMsg && bShowEffect)
	{
		DWORD dwView[3] = { GetFeather(), GetStatus(), GetHealth() };
		WORD w3 = (GetSex() << 8) | (BYTE)GetDirection();
		SendAroundMsg(GetId(), 0x321, getX(), getY(), w3, (LPVOID)dwView, sizeof(dwView));
		SendMsg(GetId(), 0x321, getX(), getY(), w3, (LPVOID)dwView, sizeof(dwView));
		SendFeatureChanged();
		if (GetJingang())
			resetHushenBuff(getX(), getY(), GetId(), 61);
		else if (GetHushen())
			resetHushenBuff(getX(), getY(), GetId(), 42);
	}
	OnChangeMap(pOldMap, ox, oy, pMap, x, y);
	return TRUE;
}

VOID CAliveObject::SendStatusChanged()   //改变状态
{
	DWORD dwFlag = m_Status.GetFlag();
	WORD wSpeed = static_cast<WORD>(GetPropValue(PI_ATTACKSPEED));
	WORD w1 = dwFlag & 0xffff;
	WORD w2 = (dwFlag & 0xffff0000) >> 16;
	SendAroundMsg(GetId(), 0x291, w1, w2, wSpeed);
	if (CanRecvMsg())
		SendMsg(GetId(), 0x291, w1, w2, wSpeed);
}

BOOL CAliveObject::Damage(DWORD dwHitter, int value)
{
	DecPropValue(PI_CURHP, value);
	if (GetPropValue(PI_CURHP) == 0)
	{
		if (IsNoDead() || !WillDie()) return FALSE;
		AddProcess(EP_DEAD, dwHitter, 0, 0, 0, 2);
	}
	return TRUE;
}

//开启技能状态
VOID CAliveObject::OnStatusSet(int index, DWORD dwParam)
{
	switch (index)
	{
	case 16: // 风火轮
		break;
		//case	17:	//	战士护身
		   //{
			  //UpdateProp();
		//		UpdateSubProp();
			 //}
			//break;
	case 20: // 魔法盾
		//AddProp( PI_MAXAC, (int)dwParam);
		break;
	case 21: // 幽灵盾
		AddProp(PI_MAXMAC, (int)dwParam);
		break;
	case 22: // 神圣战甲
		AddProp(PI_MAXAC, (int)dwParam);
		break;
	case 30: // 红毒
		AddProp(PI_MAXMAC, -1 * (int)dwParam);
		AddProp(PI_MAXAC, -1 * (int)dwParam);
		break;
	case 23: // 隐身
	{
		xListHost<VISIBLE_OBJECT>::xListNode* pNode = nullptr;
		if (m_xVisibleObjectList.getCount())
		{
			pNode = m_xVisibleObjectList.getHead();
			while (pNode)
			{
				if (pNode->getObject()->pObject->GetType() == OBJ_MONSTER &&
					((CMonsterEx*)pNode->getObject()->pObject)->GetTarget() == this)
				{
					if (DISTANCE(getX(), getY(), pNode->getObject()->pObject->getX(), pNode->getObject()->pObject->getY()) > 1)
					{
						((CMonsterEx*)pNode->getObject()->pObject)->SetTarget(nullptr);
					}
				}
				pNode = pNode->getNext();
			}
		}
	}
	break;
	}
}

//技能效果时间到
VOID CAliveObject::OnStatusClr(int index, DWORD dwParam)
{
	switch (index)
	{
	case 16: // 风火轮
		break;
	case 17: // 战士护身        
		SaySystemAttrib(CC_GREENS, "你的护身真气已经击碎");
		break;
	case 20: // 魔法盾
		//AddProp( PI_MAXAC, -1 * (int)dwParam);
		break;
	case 21: // 幽灵盾　
		SaySystemAttrib(CC_GREENB, "抗魔法力恢复正常");
		AddProp(PI_MAXMAC, -1 * (int)dwParam);
		break;
	case 22: // 神圣战甲
		SaySystemAttrib(CC_GREENB, "防御力恢复正常");
		AddProp(PI_MAXAC, -1 * (int)dwParam);
		break;
	case 23: // 隐身
		break;
	case 30: // 红毒
		AddProp(PI_MAXAC, (int)dwParam);
		AddProp(PI_MAXMAC, (int)dwParam);
		break;
	}
}

BOOL CAliveObject::MagicBoom(int nDamage, int x, int y, int nWide, int dwDelay)
{
	int mw = m_pMap->GetWidth();
	int mh = m_pMap->GetHeight();
	int nStartX = MAX(0, x - nWide);
	int nStartY = MAX(0, y - nWide);
	int nEndX = MIN(mw - 1, x + nWide);
	int nEndY = MIN(mh - 1, y + nWide);

	CMapCellInfo** pCellInfoBase = m_pMap->GetCellInfoBase();
	if (pCellInfoBase == nullptr) return FALSE;

	xListHost<CMapObject>::xListNode* pNode = nullptr;
	CMapObject* pObject = nullptr;
	BOOL bFlag = FALSE;

	// 双指针：行指针遍历x轴, 列指针遍历y轴
	for (int tx = nStartX; tx <= nEndX; tx++)
	{
		CMapCellInfo** pCol = pCellInfoBase + tx + nStartY * mw;
		for (int ty = nStartY; ty <= nEndY; ty++)
		{
			CMapCellInfo* pInfo = *pCol;
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					pObject = pNode->getObject();
					pNode = pNode->getNext();
					if (pObject == this) continue;
					if (pObject && pObject->GetClassType() == CLS_ALIVEOBJECT && !((CAliveObject*)pObject)->IsDeath() && IsProperTarget((CAliveObject*)pObject))
					{
						if (dwDelay == 0 && pObject->GetType() == OBJ_MONSTER && ((CMonsterEx*)pObject)->GetImage() == 0xc7) // 0xc7 表示通天教主
						{
							int nDis = DISTANCE(pObject->getX(), pObject->getY(), getX(), getY());
							dwDelay = 60 * nDis + 300;
						}
						((CAliveObject*)pObject)->AddProcess(EP_BEATTACKED, nDamage, GetId(), DT_MAGIC, 0, dwDelay);
						bFlag = TRUE;
					}
				}
			}
			pCol += mw;  // 移动到下一行的同一列
		}
	}
	return bFlag;
}

BOOL CAliveObject::DoPushed(int nDirection)
{
	int x = getX();
	int y = getY();
	GETNEXTPOS(x, y, nDirection);
	int	nBackDir = GETBACKDIR(nDirection);
	if (m_pMap->MoveObject(this, x, y))
	{
		DWORD dwView[] = { GetFeather(), GetStatus() };
		SendAroundMsg(GetId(), SM_BACK, x, y, nBackDir, dwView, sizeof(dwView));
		SendMsg(GetId(), SM_BACK, x, y, nBackDir, dwView, sizeof(dwView));
		SetDirection((e_direction)nBackDir);
		return TRUE;
	}
	return FALSE;
}

BOOL CAliveObject::DoLionPush(int nDirection)
{
	int x = getX();
	int y = getY();
	GETNEXTPOS(x, y, nDirection);
	int	nBackDir = GETBACKDIR(nDirection);
	if (m_pMap->MoveObject(this, x, y))
	{
		stLionBack lionback = { 0 };
		lionback.dwFeature = GetFeather();
		lionback.dwStatus = GetStatus();
		lionback.wEndFlag = 0;
		SendAroundMsg(GetId(), 5, x, y, nBackDir, &lionback, sizeof(lionback));
		SendMsg(GetId(), 5, x, y, nBackDir, &lionback, sizeof(lionback));
		SetDirection((e_direction)nBackDir);
		return TRUE;
	}
	return FALSE;
}

BOOL CAliveObject::DoRush(int nDirection, DWORD dwFlag)
{
	int x = getX();
	int y = getY();
	GETNEXTPOS(x, y, nDirection);
	SetDirection((e_direction)nDirection);
	DWORD dwView[2] = { GetFeather(), GetStatus() };
	dwFlag = MAKEWORD(nDirection, dwFlag);
	if (m_pMap->MoveObject(this, x, y))
	{
		SendAroundMsg(GetId(), SM_RUSH, x, y, (WORD)dwFlag, dwView, sizeof(dwView));
		SendMsg(GetId(), SM_RUSH, x, y, (WORD)dwFlag, dwView, sizeof(dwView));
		return TRUE;
	}
	else
	{
		SendAroundMsg(GetId(), SM_RUSH_FAIL, x, y, (WORD)dwFlag, dwView, sizeof(dwView));
		SendMsg(GetId(), SM_RUSH_FAIL, x, y, (WORD)dwFlag, dwView, sizeof(dwView));
		return FALSE;
	}
}

int CAliveObject::CharPushed(int nDirection, int nCount)
{
	int iCount = 0;
	int x = 0;
	int y = 0;
	int nBackDir = GETBACKDIR(nDirection);
	for (int i = 0; i < nCount; i++)
	{
		x = getX();
		y = getY();
		GETNEXTPOS(x, y, nDirection);
		if (m_pMap->MoveObject(this, x, y))
		{
			DWORD dwView[2] = { GetFeather(), GetStatus() };
			SendAroundMsg(GetId(), SM_BACK, x, y, nBackDir, dwView, sizeof(dwView));
			SendMsg(GetId(), SM_BACK, x, y, nBackDir, dwView, sizeof(dwView));
			iCount++;
		}
	}
	return iCount;
}

VOID CAliveObject::OnSetPos(WORD oldx, WORD oldy, WORD newx, WORD newy)
{
	if (m_bPosLocked)
	{
		if (m_pMap)m_pMap->UnLockPos((int)oldx, (int)oldy);
	}
	if (m_pMap)
		m_bPosLocked = m_pMap->LockPos(newx, newy);
	CheckClearCloak();
}

BOOL CAliveObject::DoMagicPushAround(int nCount)
{
	xListHost<CMapObject>::xListNode* pNode = nullptr;
	int x, y;
	CMapObject* pObject = nullptr;
	BOOL bFlag = FALSE;
	for (int i = 0; i < 8; i++)
	{
		x = getX();
		y = getY();
		GETNEXTPOS(x, y, i);
		CMapCellInfo* pInfo = m_pMap->GetMapCellInfo(x, y);
		if (pInfo)
		{
			pNode = pInfo->m_xObjectList.getHead();
			while (pNode)
			{
				pObject = pNode->getObject();
				pNode = pNode->getNext();
				if (pObject && pObject->GetClassType() == CLS_ALIVEOBJECT && !((CAliveObject*)pObject)->IsDeath() && IsProperTarget((CAliveObject*)pObject) && ((CAliveObject*)pObject)->CanBePushed(this))
				{
					((CAliveObject*)pObject)->CharPushed(i, nCount);
					bFlag = TRUE;
				}
			}
		}
	}
	return bFlag;
}

BOOL CAliveObject::DoPointMagic(int nDamage, int x, int y)
{
	BOOL bFlag = FALSE;
	xListHost<CMapObject>::xListNode* pNode = nullptr;
	CMapObject* pObject = nullptr;
	CMapCellInfo* pInfo = m_pMap->GetMapCellInfo(x, y);
	if (pInfo)
	{
		pNode = pInfo->m_xObjectList.getHead();
		while (pNode)
		{
			pObject = pNode->getObject();
			pNode = pNode->getNext();
			if (pObject == this)continue;
			if (pObject && pObject->GetClassType() == CLS_ALIVEOBJECT && !((CAliveObject*)pObject)->IsDeath() && IsProperTarget((CAliveObject*)pObject))
			{
				((CAliveObject*)pObject)->AddProcess(EP_BEATTACKED, nDamage, GetId(), DT_MAGIC, 0, 600);
				bFlag = TRUE;
			}
		}
	}
	return bFlag;
}

BOOL CAliveObject::DoLineMagic(int nDamage, int nDirection, int nCount)
{
	int x = getX();
	int y = getY();
	BOOL bFlag = FALSE;
	xListHost<CMapObject>::xListNode* pNode = nullptr;
	CMapObject* pObject = nullptr;
	for (int i = 0; i < nCount; i++)
	{
		GETNEXTPOS(x, y, nDirection);
		CMapCellInfo* pInfo = m_pMap->GetMapCellInfo(x, y);
		if (pInfo)
		{
			pNode = pInfo->m_xObjectList.getHead();
			while (pNode)
			{
				pObject = pNode->getObject();
				pNode = pNode->getNext();
				if (pObject == this)continue;
				if (pObject && pObject->GetClassType() == CLS_ALIVEOBJECT && !((CAliveObject*)pObject)->IsDeath() && IsProperTarget((CAliveObject*)pObject))
				{
					int nDis = DISTANCE(pObject->getX(), pObject->getY(), getX(), getY());
					((CAliveObject*)pObject)->AddProcess(EP_BEATTACKED, nDamage, GetId(), DT_MAGIC, 0, 120 * nDis + 500);
					bFlag = TRUE;
				}
			}
		}
	}
	return bFlag;
}

VOID CAliveObject::DefenceUp(int nState, int nSec)
{
	int nPower = 2 + GetPropValue(PI_LEVEL) / 7;
	SetStatus(nState, nPower, nSec * 1000);
}

VOID CAliveObject::ForceMove(int x, int y)
{
	if (m_pMap == nullptr)return;
	if (m_pMap->IsBlocked(x, y))return;
	if (m_pMap->MoveObject(this, x, y))
	{
		DWORD dwFeature = GetFeather();
		SendAroundMsg(GetId(), 0x0c, x, y, (WORD)GetDirection(), &dwFeature, 4);
		SendMsg(GetId(), 0x0c, x, y, (WORD)GetDirection(), &dwFeature, 4);
	}
}

VOID CAliveObject::SetTarget(CAliveObject* pObject)
{
	CAliveObject* pOld = m_refObjTarget.getObject();
	m_refObjTarget.SetObject(pObject);
	OnChangeTarget(pOld, pObject);
}

VOID CAliveObject::SetHitter(CAliveObject* pHitter)
{
	CAliveObject* pOld = m_refObjHitter.getObject();
	m_refObjHitter.SetObject(pHitter);
	OnChangeHitter(pOld, pHitter);
}

VOID CAliveObject::SetOwner(CAliveObject* pOwner)
{
	CAliveObject* pOld = m_refObjOwner.getObject();
	m_refObjOwner.SetObject(pOwner);
	OnChangeOwner(pOld, pOwner);
}

VOID CAliveObject::TakeDamage(DWORD dwAttackId, int nDamage, damage_type type, DWORD dwFlag, DWORD dwDelay, DWORD dwCount)
{
	AddProcess(EP_BEATTACKED, nDamage, dwAttackId, (DWORD)type, dwFlag, dwDelay, dwCount);
}

VOID CAliveObject::Hide()
{
	char szMsg[1024];
	int length;
	if (GetOutViewmsg(szMsg, length))
		SendAroundMsg(szMsg, length);
}

VOID CAliveObject::Show()
{
	char szMsg[1024];
	int length;
	if (GetViewmsg(szMsg, length))
		SendAroundMsg(szMsg, length);
}

BOOL CAliveObject::IsGodBlessEffective(int type)
{
	if (IsSystemFlagSeted(SF_GODBLESS))//护身
	{
		DWORD dwParam = m_SystemFlag.GetParam(SF_GODBLESS);
		if (type == (dwParam & 0xffff))
		{
			if (Getrand(100) < (int)(dwParam >> 16))
			{
				switch (type)
				{
				case 1:
					SaySystem("秒杀成功!");
					break;
				case 2:
					SaySystem("重击成功!");
					break;
				case 3:
					SaySystem("神御成功!");
					break;
				case 4:
					SaySystem("神佑成功!");
					break;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

VOID CAliveObject::SendFeatureChanged()//外观改变  41  数据部分缺衣服的SHAPE
{
	DWORD dwFeather = GetFeather();
	BYTE temp = GetShape();
	WORD wArray[2] = { 0 };
	wArray[0] = temp;
	wArray[1] = 0;
	WORD w1 = dwFeather & 0xffff;
	WORD w2 = (dwFeather & 0xffff0000) >> 16;
	WORD w3 = (GetSex() << 8) | (BYTE)GetDirection();
	SendAroundMsg(GetId(), SM_FEATURECHANGED, w1, w2, w3, (LPVOID)wArray, 4);
	SendMsg(GetId(), SM_FEATURECHANGED, w1, w2, w3, (LPVOID)wArray, 4);
}

VOID CAliveObject::GetFrontPosition(int& x, int& y)
{
	x = getX();
	y = getY();
	GETNEXTPOS(x, y, GetDirection());
}

VOID CAliveObject::OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type)
{
#ifdef _DEBUG
	if(pAttacker) pAttacker->SaySystem("%s 被你攻击, 受到：%u 伤害.", GetName(), nDamage);
#endif
	if (pAttacker && type == DT_PHYSICS && pAttacker->GetType() == OBJ_PLAYER && !IsStatusSet(SI_PALSY) &&
		pAttacker->IsSpecialEquipmentFunctionOn(SEF_PALSY))
	{
		//	抗麻痹的怪物
		CMonsterEx* pMon = (CMonsterEx*)this;
		if (GetType() == OBJ_MONSTER && pMon->GetDesc() && (pMon->GetDesc()->sprop.pFlag & SF_ANTPALSY) != 0)
			return;
		DWORD dwRate = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_PALSY, 0);
		if (Getrand(100) < (int)dwRate)
		{
			DWORD dwTime = CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_PALSY, 1);
			if (dwTime == 0)dwTime = 5000;
			SetStatus(SI_PALSY, 0, dwTime);
		}
	}
}

BOOL CAliveObject::CanBePushed(CAliveObject* pAttacker)
{
	const int nAttackerLevel = pAttacker->GetPropValue(PI_LEVEL);
	const int nLevel = GetPropValue(PI_LEVEL);
	if (nAttackerLevel > nLevel)
	{
		if (GetType() == OBJ_PLAYER)
		{
			const int nAttackerExpPercent = pAttacker->GetPropValue(PI_EXPPERCENT);
			const int nExpPercent = GetPropValue(PI_EXPPERCENT);
			return nAttackerExpPercent > nExpPercent;
		}
		return TRUE;
	}
	return FALSE;
}

CAliveObject* CAliveObject::FindNearestObj(int nRange, e_object_type eObjType)
{
	const int myX = getX();
	const int myY = getY();

	CLogicMap* pMap = (CLogicMap*)GetMap();
	if (pMap == nullptr) return nullptr;

	CMapCellInfo** pCellInfoBase = pMap->GetCellInfoBase();
	if (pCellInfoBase == nullptr) return nullptr;

	int mw = pMap->GetWidth();

	for (int radius = 0; radius <= nRange; radius++)
	{
		if (radius == 0)
		{
			CMapCellInfo* pInfo = *(pCellInfoBase + myX + myY * mw);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CAliveObject* pObj = (CAliveObject*)pNode->getObject();
					if (pObj && IsProperTarget(pObj) && pObj->GetType() == eObjType)
						return pObj;
					pNode = pNode->getNext();
				}
			}
			continue;
		}

		int x = myX - radius;
		int y = myY - radius;

		for (int i = 0; i < radius * 2; i++, x++)
		{
			CMapCellInfo* pInfo = *(pCellInfoBase + x + y * mw);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CAliveObject* pObj = (CAliveObject*)pNode->getObject();
					if (pObj && IsProperTarget(pObj) && pObj->GetType() == eObjType)
						return pObj;
					pNode = pNode->getNext();
				}
			}
		}

		for (int i = 0; i < radius * 2; i++, y++)
		{
			CMapCellInfo* pInfo = *(pCellInfoBase + x + y * mw);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CAliveObject* pObj = (CAliveObject*)pNode->getObject();
					if (pObj && IsProperTarget(pObj) && pObj->GetType() == eObjType)
						return pObj;
					pNode = pNode->getNext();
				}
			}
		}

		for (int i = 0; i < radius * 2; i++, x--)
		{
			CMapCellInfo* pInfo = *(pCellInfoBase + x + y * mw);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CAliveObject* pObj = (CAliveObject*)pNode->getObject();
					if (pObj && IsProperTarget(pObj) && pObj->GetType() == eObjType)
						return pObj;
					pNode = pNode->getNext();
				}
			}
		}

		for (int i = 0; i < radius * 2; i++, y--)
		{
			CMapCellInfo* pInfo = *(pCellInfoBase + x + y * mw);
			if (pInfo && pInfo->m_xObjectList.getCount() > 0)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CAliveObject* pObj = (CAliveObject*)pNode->getObject();
					if (pObj && IsProperTarget(pObj) && pObj->GetType() == eObjType)
						return pObj;
					pNode = pNode->getNext();
				}
			}
		}
	}
	return nullptr;
}