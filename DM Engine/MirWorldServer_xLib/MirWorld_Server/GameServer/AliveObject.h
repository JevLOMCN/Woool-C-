#pragma once
#include "mapobject.h"
#include <mutex>
#include <memory>

enum process_type
{
	PT_BEATTACK,
	PT_BEMAGICATTACK,
};
class CAliveObject;

// 引用对象类
class CRefObject
{
public:
	CRefObject() : m_pObject(nullptr), m_dwInstanceKey(0){}
	// 获取对象
	CAliveObject* getObject() { return m_pObject; }
	// 设置对象
	VOID SetObject(CAliveObject* pObject);
	// 验证对象实例键m_dwInstanceKey是否一致
	BOOL IsValid();
protected:
	CAliveObject* m_pObject; // 对象指针
	DWORD m_dwInstanceKey; // 对象实例键
};

/// 节点连接类
class CPeerConnection : public xListHost<CPeerConnection>::xListNode
{
public:
	CPeerConnection(CMapObject* pObject) : xListHost<CPeerConnection>::xListNode(this)
	{
		m_pObject = pObject;
	}
	CMapObject* getObject() { return m_pObject; }
protected:
	CMapObject* m_pObject;
};

/// 攻击对象类
class CAttackObject
{
public:
	CAttackObject()
	{
		nAttackCount = 0;
		nDamage = 0;
	}
	VOID Clear()
	{
		refObject.SetObject(nullptr);
		nAttackCount = 0;
		nDamage = 0;
	}
	CAliveObject* getObject() { return refObject.getObject(); }
	UINT GetAttackCount()const { return nAttackCount; }
	UINT GetDamage()const { return nDamage; }
	VOID UpdateValid()
	{
		if (!refObject.IsValid())
			Clear();
	}
	VOID AddAttackInfo(CAliveObject* pObj, UINT inDamage)
	{
		if (pObj != refObject.getObject())
		{
			refObject.SetObject(pObj);
			nAttackCount = 0;
			nDamage = 0;
		}
		nAttackCount++;
		nDamage += inDamage;
		timer.Savetime();
	}
	BOOL IsTimeOut(DWORD dwTime) { return timer.IsTimeOut(dwTime); }
private:
	CRefObject refObject;
	UINT nAttackCount;
	UINT nDamage;
	CServerTimer timer;
};

constexpr int VIEW_RANGE = 16; // 原版是12
typedef xListHost<VISIBLE_OBJECT> VISIBLE_OBJECT_LIST;
// 活体对象类, 继承自CMapObject对象
class CAliveObject : public CMapObject
{
public:
	CAliveObject(void);
	virtual ~CAliveObject(void);
	virtual VOID Clean();
	BOOL Walk(int dir, DWORD dwDelay = 0);
	BOOL WalkXY(int x, int y, int dir, DWORD dwDelay = 0);
	BOOL Run(int dir, DWORD dwDelay = 0);
	BOOL RunXY(int x, int y, int dir, DWORD dwDelay = 0);
	//发包告诉客户端停止动作执行
	VOID Stop();
	BOOL Turn(int dir);
	BOOL Attack(int dir, DWORD dwDelay = 0, e_humanattackmode mode = HAM_ALL);
	BOOL BeAttack(CAliveObject* pAttacker, int nDamage, damage_type damagetype = DT_PHYSICS, DWORD dwFlag = 0, damage_ReType damageReType = DRT_NONE);
	virtual int  getdir() { return 0; }
	BOOL GetMeal(int dir);
	BOOL Backward(int dir);
	VOID Say(const char* pszMsg, ...);
	VOID SaySystem(const char* pszMsg, ...);
	VOID SaySystem10(const char* pszMsg, ...);
	VOID SaySystemAttrib(DWORD dwAttrib, const char* pszMsg, ...);
	// 获取当前时间戳
	DWORD GetTimeStamp()
	{
		time_t t;
		time(&t);
		DWORD dwT2 = static_cast<DWORD>(t);
		return dwT2;
	}

	virtual VOID OnChangeMap(CLogicMap* pFromMap, UINT nFromX, UINT nFromY, CLogicMap* pToMap, UINT nToX, UINT nToY) {}
	virtual VOID OnLeaveMap(CLogicMap* pMap);
	virtual VOID OnEnterMap(CLogicMap* pMap);
	virtual VOID OnDeath(DWORD dwKiller) {}
	virtual BOOL PetAddgold(DWORD dwgold, BOOL bUpdateClient = TRUE) { return FALSE; }//豹子拾取金币增加到主人包裹
	virtual BOOL PetAddBagItem(ITEM& item, BOOL bSilence = FALSE, BOOL bUpdateDB = FALSE, BOOL bUpdateWeight = TRUE) { return FALSE; }//豹子拾取物品增加到主人包裹
	virtual VOID OnAttackTarget(CAliveObject* pTarget, int nDamage) {}
	virtual VOID OnDamageTarget(CAliveObject* pTarget, int nDamage, damage_type type)
	{
		if (pTarget)
		{
			m_AttackObject.UpdateValid();
			m_AttackObject.AddAttackInfo(pTarget, nDamage);
			if (pTarget->EnterTargetList(this, m_AttackObject.GetDamage()))
				m_AttackObject.Clear();
		}
	}
	// 如果目标在移动中, 返回目标激活坐标；否则返回目标当前坐标
	VOID GetPosition(WORD& wX, WORD& wY)const
	{
		if (m_ActionType == AT_WALK || m_ActionType == AT_RUN)
		{
			wX = m_wActionX;
			wY = m_wActionY;
		}
		else
		{
			wX = m_wX;
			wY = m_wY;
		}
	}
	virtual BOOL EnterTargetList(CAliveObject* pTarget, UINT nDamage) { return FALSE; }
	virtual VOID OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type);
	virtual BOOL CanDoAction(actiontype action);
	virtual BOOL CanMove() { return TRUE; }
	virtual VOID Update();

	VOID SendMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata = nullptr, int datasize = -1);
	VOID SendMsgToObject(CAliveObject* pReceiver, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata = nullptr, int datasize = -1);
	VOID SendAroundMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata = nullptr, int datasize = -1);
	VOID SendMapMsg(DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata = nullptr, int datasize = -1);

	virtual DWORD GetFeather() { return 0; }
	virtual BYTE GetShape() { return 0; }
	virtual DWORD GetHealth() { return 0; }
	virtual DWORD GetStatus() { return m_Status.GetFlag(); }
	virtual BYTE GetSex() { return 0; }
	virtual const char* GetName() { return "NONAME"; }

	virtual const char* GetViewName() { return GetName(); }

	BOOL SetAction(actiontype action, e_direction dir, WORD x, WORD y, DWORD dwActionTime);

	BOOL CompleteAction();

	BOOL GetOutViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr);
	virtual BOOL GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr);

	BYTE GetWalkSpeed()const { return m_btWalkSpeed; }
	VOID SetWalkSpeed(BYTE btSpeed) { m_btWalkSpeed = btSpeed; }
	virtual BYTE GetRunSpeed() { return m_btRunSpeed; }
	VOID SetRunSpeed(BYTE btSpeed) { m_btRunSpeed = btSpeed; }

	WORD GetDeInvisibleLevel() { return m_wDeInvisibleLevel; }
	e_direction GetDirection()const { return m_Direction; }
	e_direction GetValidDirection(int dir) { return static_cast<e_direction>((static_cast<int>(dir) + ED_MAX) % ED_MAX); }
	void SetDirection(e_direction dir)
	{
		m_Direction = dir;
		m_ActionDirection = dir;
	}

	WORD GetActionX()const { return m_wActionX; }
	WORD GetActionY()const { return m_wActionY; }
	//获取当前动作
	actiontype GetActionType()const { return m_ActionType; }
	//获取预前动作
	actiontype GetPreActionType()const { return m_PreActionType; }
	//设置预前动作
	VOID SetPreActionType(actiontype type){ m_PreActionType = type; }
	virtual e_class_type GetClassType() { return CLS_ALIVEOBJECT; }

	VOID ResetPos(WORD x, WORD y);

	inline int GetAttackPower(int nDamage, int nVal)
	{
		int nPower = 0;
		if (nVal < 0) nVal = 0;
		nPower = nDamage + (Getrand(nVal + 1));
		return nPower;
	}
	inline int GetPropPower()
	{
		int nMin = GetPropValue(PI_MINDC);
		int nMax = GetPropValue(PI_MAXDC);
		int lucky = GetPropValue(PI_LUCKY) - GetPropValue(PI_DAWN);
		if (lucky > 0 && Getrand(100) < g_pLucky[lucky])
			nMin = nMax - (nMax - nMin) * max(10 - lucky, 0) / 10;
		else if (lucky < 0)
			nMax = nMin + (nMax - nMin) * max(lucky + 10, 0) / 10;
		return GetRangeRand(nMin, nMax);
	}
	inline BOOL IsFullHp()
	{
		return GetPropValue(PI_CURHP) == GetPropValue(PI_MAXHP);
	}
	inline int GetPropMC()
	{
		int nMin = GetPropValue(PI_MINMC);
		int nMax = GetPropValue(PI_MAXMC);
		if (nMin == nMax)return nMax;
		else return GetRangeRand(nMin, nMax);
	}
	inline int GetPropAC()
	{
		int nMin = GetPropValue(PI_MINAC);
		int nMax = GetPropValue(PI_MAXAC);
		if (nMin == nMax)return nMax;
		else return GetRangeRand(nMin, nMax);
	}
	inline int GetPropMAC()
	{
		int nMin = GetPropValue(PI_MINMAC);
		int nMax = GetPropValue(PI_MAXMAC);
		if (nMin == nMax) return nMax;
		else return GetRangeRand(nMin, nMax);
	}
	// 物品数据转换成客户端结构
	VOID ItemToClient(ITEM& item);
	//获取随机power
	int	GetRangePower(int min, int max) { return GetRangeRand(min, max); }
	//用虚函数调用子类, 实现子类的抵抗次数减一
	virtual int SecondResMag_count() { return 0; }
	//这里计算魔法盾抵抗魔法攻击及免伤
	int	GetMagDamage(int nDamage)
	{
		int	nDefence = GetRangePower(GetPropValue(PI_MINMAC), GetPropValue(PI_MAXMAC));
		nDamage = MAX(0, nDamage - nDefence);
		if (nDamage > 0)
		{
			if (IsStatusSet(SI_BUBBLEDEFENCEUP))
			{
				int nodamage = GetNoDamage();
				double nodamagePercentage = nodamage / 100.0; // 将免伤值除以 100, 得到百分比值
				int temp = static_cast<int>(static_cast<double>(nDamage) * nodamagePercentage);
				nDamage -= temp;
				int tempd = SecondResMag_count();
				if (tempd == 0)
				{
					DWORD timetemp = m_Status.GetTime(20);
					m_Status.DecTime(20, timetemp);
				}
			}
		}
		return nDamage;
	}
	//这里计算魔法盾抵抗物理攻击及免伤
	int	GetHitDamage(int nDamage)
	{
		int	nDefence = GetRangePower(GetPropValue(PI_MINMAC), GetPropValue(PI_MAXMAC));
		nDamage = MAX(0, nDamage - nDefence);
		if (nDamage > 0)
		{
			if (IsStatusSet(SI_BUBBLEDEFENCEUP))
			{
				int nodamage = GetNoDamage();
				double nodamagePercentage = nodamage / 100.0; // 将无伤害值除以 100, 得到百分比值
				//将整数类型的 nDamage 转换为浮点数类型, 执行乘法运算, 然后再转换回整数类型
				int temp = static_cast<int>(static_cast<double>(nDamage) * nodamagePercentage);
				nDamage -= temp;
				int tempd = SecondResMag_count();
				if (tempd == 0)
				{
					DWORD timetemp = m_Status.GetTime(20);
					m_Status.DecTime(20, timetemp);
				}
			}
		}
		return nDamage;
	}
public:
	virtual BOOL IsGodBlessEffectivable(special_godbless type, CAliveObject* pObject) { return FALSE; }
	virtual BOOL IsDefenceEffectiveObject(CAliveObject* pObject) { if (pObject == nullptr)return FALSE; return TRUE; }
	//飞到地图某点, 或者切换地图
	BOOL FlyTo(int mapid, int x, int y, BOOL bShowEffect = TRUE);
	//飞到地图某点, 或者切换地图
	BOOL FlyTo(CLogicMap* pMap, int x, int y, BOOL bShowEffect = TRUE);
	//增加经验
	virtual VOID AddExp(DWORD dwExp, int level, DWORD dwId = 0) {}
	//获取护身真气
	virtual int getHushenbuf() { return 0; }
	//重新施放护身真气,或者金刚护体
	virtual VOID resetHushenBuff(int x, int y, UINT nTarget, WORD wMagicId) {}
	//获取是否开启护身
	virtual BOOL GetHushen() { return FALSE; }
	//获取是否开启金刚护体
	virtual BOOL GetJingang() { return FALSE; }

	virtual int	GetPropValue(PROP_INDEX index)
	{
		if ((UINT)index >= PI_PROP_COUNT)return 0;
		switch (index)
		{
		case PI_MAXAC:
			return m_AddProp[index] - GetStatusParam(SI_REDPOISON); // 红毒(减护甲)
		case PI_MAXMAC:
			return m_AddProp[index] - GetStatusParam(SI_STRAWWOMAN); // 诅咒术(女)蓝咒-降魔防
		}
		return m_AddProp[index];
	}
	virtual void DecPropValue(PROP_INDEX index, int value) {}
	virtual void AddPropValue(PROP_INDEX index, int value) {}
	//回复血, 蓝, 回复时间
	virtual int GetAutoRecoverHp() { return 0; }
	virtual int GetAutoRecoverMp() { return 0; }
	virtual int GetAutoRecoverHptime() { return 0; }
	virtual int GetAutoRecoverMptime() { return 0; }
	// 发送生命值魔法值变化消息
	virtual VOID SendHpMpChanged(int damage = 0, WORD wEffect = 57) {}
	VOID DropGold(DWORD dwCount, int x, int y, DWORD dwOwner = 0);
	virtual BOOL AddPet(CAliveObject* pObject) { return TRUE; }
	virtual BOOL DelPet(CAliveObject* pObject) { return TRUE; }
	virtual BOOL CanEnterMap(CLogicMap* pMap) { return TRUE; }

	VOID Hide();
	VOID Show();

	virtual VOID DoProcess(OBJECTPROCESS* pProcess);
	BOOL AddProcess(e_process ident, DWORD dwParam1 = 0, DWORD dwParam2 = 0, DWORD dwParam3 = 0, DWORD dwParam4 = 0, DWORD dwDelay = 0, int repeattimes = 0, const char* pszString = nullptr, BOOL firstTime = FALSE);
	BOOL AddProcess(OBJECTPROCESS* pProcess);
	BOOL IsProcess(e_process ident) const;
	int GetProcessQueueCount() { return m_xQProcess.getcount(); }
	VOID UpdateVisibleObject(CMapObject* pObject);
	VOID AddVisibleObject(CMapObject* pObject);
	VOID RemoveVisibleObject(CMapObject* pObject);
	VOID SearchViewRange();
	VOID UpdateViewRange(UINT ox, UINT oy);
	VOID RefreshViewList();
	VOID UpdateViewObjectList(xListHost<CMapObject>* pList);
	VOID UpdateOutViewObjectList(xListHost<CMapObject>* pList);
	VOID UpdateMonster(UINT ox, UINT oy);
	VOID OnMoveTo(UINT ox, UINT oy, UINT nx, UINT ny) { UpdateViewRange(ox, oy); }

#ifdef USE_FREE_MEMORY
	VISIBLE_OBJECT* NewVisibleObject() { VISIBLE_OBJECT* p = new VISIBLE_OBJECT; return p; }
	VOID DeleteVisibleObject(VISIBLE_OBJECT* pVisibleObject) { if (pVisibleObject->pObject)pVisibleObject->pObject->DecRef(); delete pVisibleObject; }
#else
	VISIBLE_OBJECT* NewVisibleObject() { VISIBLE_OBJECT* p = m_xVisibleObjectPool.newObject(); if (p) { p->pObject = nullptr; }return p; }
	VOID DeleteVisibleObject(VISIBLE_OBJECT* pVisibleObject) { if (pVisibleObject->pObject)pVisibleObject->pObject->DecRef(); m_xVisibleObjectPool.deleteObject(pVisibleObject); }
#endif

	DWORD GetVisibleObjectFlag()const { return m_nVisibleObjectFlag; }
	VOID AddVisibleObjectType(e_object_type type)
	{
		m_nVisibleObjectFlag |= (1 << (int)type);
	}
	VOID RemoveVisibleObjectType(e_object_type type)
	{
		DWORD dwFlag = (1 << (int)type);
		if (m_nVisibleObjectFlag & dwFlag)
			m_nVisibleObjectFlag ^= dwFlag;
	}

	VOID SendAroundMsg(const char* szMsg, int length);
	VOID ToDeath(DWORD dwKiller = 0);
	BOOL IsDeath()const { return m_bDead; }
	VOID SetDeath(BOOL bDead) { m_bDead = bDead; }
	//计算伤害
	virtual BOOL Damage(DWORD dwHitter, int value);
	//获取子类的免伤
	virtual int GetNoDamage() { return 0; }
	BOOL SetStatus(int index, DWORD dwParam = 0, DWORD dwLastTime = 0)
	{
		switch (index)
		{
		case SI_PALSY:
		{
			if (!m_status26Timer.IsTimeOut(m_dwStatus26)) return FALSE;
		}
		break;
		}
		if (m_Status.IsSeted(index))
		{
			m_Status.AddTime(index, dwLastTime);
			SendStatusChanged();
			return TRUE;
		}
		else if (m_Status.SetStatus(index, dwParam, dwLastTime))
		{
			OnStatusSet(index, dwParam);
			SendStatusChanged();
			return TRUE;
		}
		return FALSE;
	}
	DWORD GetStatusParam(int index)
	{
		if (m_Status.IsSeted(index))
			return m_Status.GetParam(index);
		return 0;
	}
	DWORD GetSetter(int index) 
	{
		if (m_Status.IsSeted(index))
			return m_Status.GetSetter(index);
		return 0;
	}
	VOID SetSetter(int index, DWORD dwSetter = 0)
	{
		if (m_Status.IsSeted(index))
			return m_Status.SetSetter(index, dwSetter);
	}
	BOOL IsStatusSet(int index) { return m_Status.IsSeted(index); }
	BOOL ClrStatus(int index)
	{
		DWORD dwParam = m_Status.GetParam(index);
		if (m_Status.ClrStatus(index))
		{
			OnStatusClr(index, dwParam);
			SendStatusChanged();
			return TRUE;
		}
		return FALSE;
	}
	//657状态改变
	VOID SendStatusChanged();
	//外观改变41
	VOID SendFeatureChanged();

	int	GetVisibleObjectCount() { return m_xVisibleObjectList.getCount(); }

	DWORD GetInstanceKey()const { return m_dwInstanceKey; }
	BOOL IsPetsActive()const { return m_bPetsActive; }
	virtual VOID SetPetsActive(BOOL bActive) { m_bPetsActive = bActive; }
	virtual DWORD GetOwnerKey() { return 0; }
	virtual VOID OnStatusSet(int index, DWORD dwParam = 0);
	virtual VOID OnStatusClr(int index, DWORD dwParam = 0);

	VOID SetNoDead(BOOL bNoDead) { m_bNoDead = bNoDead; }
	BOOL IsNoDead()const { return m_bNoDead; }
	VOID SetNoDamage(BOOL bNoDamage) { m_bNoDamage = bNoDamage; }
	BOOL IsNoDamage()const { return m_bNoDamage; }

	//扣掉人物技所消耗的蓝量
	BOOL CostMp(DWORD dwCostMp)
	{
		DWORD dwCurMp = (DWORD)GetPropValue(PI_CURMP);
		if (dwCurMp < dwCostMp) return FALSE;
		DecPropValue(PI_CURMP, (int)dwCostMp);
		return TRUE;
	}
	VOID SetSuperHit(BOOL bSuperHit) { m_bSuperHit = bSuperHit; }
	BOOL IsSuperHit()const { return m_bSuperHit; }

	DWORD GetHpRecoverTick()const { return m_dwHpRecoverTick; }
	VOID SetHpRecoverTick(DWORD dwTick) { m_dwHpRecoverTick = dwTick; }

	DWORD GetMpRecoverTick()const { return m_dwMpRecoverTick; }
	VOID SetMpRecoverTick(DWORD dwTick) { m_dwMpRecoverTick = dwTick; }

	VOID SetAddHp(DWORD dwAddValue, DWORD dwSpeed)
	{
		m_dwAddHp += dwAddValue;
		if (m_dwAddHp > (DWORD)GetPropValue(PI_MAXHP))
			m_dwAddHp = (DWORD)GetPropValue(PI_MAXHP);
		m_dwAddHpSpeed = dwSpeed;
	}
	VOID SetAddMp(DWORD dwAddValue, DWORD dwSpeed)
	{
		m_dwAddMp += dwAddValue;
		if (m_dwAddMp > (DWORD)GetPropValue(PI_MAXMP))
			m_dwAddMp = (DWORD)GetPropValue(PI_MAXMP);
		m_dwAddMpSpeed = dwSpeed;
	}
	VOID OnSetPos(WORD oldx, WORD oldy, WORD newx, WORD newy);
	virtual BOOL IsProperFriend(CAliveObject* pObject) { return TRUE; }
	virtual BOOL IsProperTarget(CAliveObject* pObject) { return TRUE; }
	virtual BOOL CanRecover() { return TRUE; }
	//技能爆炸
	BOOL MagicBoom(int nDamage, int x, int y, int nWide, int dwDelay = 0);
	BOOL DoPushed(int nDirection);
	BOOL DoLionPush(int nDirection);
	BOOL DoRush(int nDirection, DWORD dwFlag);
	int CharPushed(int nDirection, int nCount);
	BOOL DoMagicPushAround(int nCount = 1);
	//直线技能
	BOOL DoLineMagic(int nDamage, int nDirection, int nCount);
	//坐标技能
	BOOL DoPointMagic(int nDamage, int x, int y);

	BOOL IsSystemFlagSeted(int index) { return m_SystemFlag.IsSeted(index); }

	DWORD GetSystemFlagParam(int index)
	{
		if (!m_SystemFlag.IsSeted(index))return 0;
		return m_SystemFlag.GetParam(index);
	}
	VOID SetSystemFlag(int index, BOOL bFlag, DWORD dwParam = 0, DWORD dwTimeOut = 0xffffffff)
	{
		if (bFlag)
		{
			if (m_SystemFlag.IsSeted(index))
			{
				DWORD dwParamT = m_SystemFlag.GetParam(index);
				m_SystemFlag.ClrStatus(index);
				OnSystemFlagCleared(index, dwParamT);
			}
			m_SystemFlag.SetStatus(index, dwParam, dwTimeOut);
			OnSystemFlagSeted(index, dwParam);
		}
		else
		{
			DWORD dwParamT = m_SystemFlag.GetParam(index);
			m_SystemFlag.ClrStatus(index);
			OnSystemFlagCleared(index, dwParamT);
		}
	}
	virtual VOID OnSystemFlagSeted(int index, DWORD dwParam = 0) {}
	virtual VOID OnSystemFlagCleared(int index, DWORD dwParam = 0) {}
	VOID DefenceUp(int nState, int nSec);
	VOID ForceMove(int x, int y);

	virtual BYTE GetNameColor(CMapObject* pViewer) { return 255; }
	virtual BYTE GetFenghaoType23() { return 0; }

	VOID SendChangeName();

	VOID TakeDamage(DWORD dwAttackId, int nDamage, damage_type type, DWORD dwFlag = 0, DWORD dwDelay = 0, DWORD dwCount = 0);
	static VOID	GetVisibleObjectInfo(int& used, int& free, int& total)
	{
		used = m_xVisibleObjectPool.getUsedCount();
		free = m_xVisibleObjectPool.getFreeCount();
		total = m_xVisibleObjectPool.getCount();
	}
	virtual VOID OnDoAction(actiontype action) {}
	virtual BOOL WillDie() { return TRUE; }
	virtual BOOL CanBePushed(CAliveObject* pAttacker);
protected:
	char m_szLongName[128];
	//DWORD	m_dwSystemFlag;
	xStatus	m_SystemFlag;
	VOID CheckClearCloak()
	{
		if (IsStatusSet(SI_CLOAK) && m_Status.GetTimeOut(SI_CLOAK) != 0xffffffff)
			AddProcess(EP_CLRSTATUS, SI_CLOAK);
	}

	DWORD m_dwAddHp; // 增加生命值数量
	DWORD m_dwAddHpSpeed; // 增加生命值速度
	CServerTimer m_AddHpTimer; // 加生命值定时器

	DWORD m_dwAddMp; // 增加魔法值数量
	DWORD m_dwAddMpSpeed; // 增加魔法值速度
	CServerTimer m_AddMpTimer; // 加魔法值定时器

	CServerTimer m_HpRecoverTimer; // 恢复生命值定时器
	DWORD m_dwHpRecoverTick; // 恢复生命值间隔时间

	CServerTimer m_MpRecoverTimer; // 恢复魔法值定时器
	DWORD m_dwMpRecoverTick; // 恢复魔法值间隔时间

	CServerTimer m_ActionTimer; // 动作定时器
	CServerTimer m_CustomTimer; // 自定义使用定时器

	BOOL m_bSuperHit;
	BOOL m_bNoDead;
	BOOL m_bNoDamage;
	xStatus	m_Status;

	xPtrQueue<OBJECTPROCESS> m_xQProcess;
	unsigned long long m_dwProcessFlags;  // 64个bit可以表示64种process

	e_direction	m_Direction;
	BYTE m_btWalkSpeed;
	BYTE m_btRunSpeed;
	WORD m_wDeInvisibleLevel;
	actiontype	m_ActionType;
	actiontype m_PreActionType; // 动作前的动作,主要记录除了站立以外的动作
	WORD m_wActionX, m_wActionY;
	e_direction	m_ActionDirection;
	DWORD m_dwActionCompleteTime;

	DWORD m_dwInstanceKey;
	BOOL m_bPetsActive;

	DWORD m_nVisibleObjectFlag;

	xListHost<VISIBLE_OBJECT> m_xVisibleObjectList;
	xListHost<VISIBLE_OBJECT> m_xVisibleItemsList;

	BOOL m_bDead;
	BOOL m_bPosLocked;

	static xObjectPool<VISIBLE_OBJECT>	m_xVisibleObjectPool;
	int m_AddProp[PI_PROP_COUNT];

	VOID AddProp(int index, int value)
	{
		if (index < 0 || index >= PI_PROP_COUNT)return;
		m_AddProp[index] += value;
	}
	VOID DecProp(int index, int value)
	{
		if (index < 0 || index >= PI_PROP_COUNT)return;
		m_AddProp[index] -= value;
	}
	VOID SetAddProp(int index, int value)
	{
		if (index < 0 || index >= PI_PROP_COUNT)return;
		m_AddProp[index] = value;
	}
protected:
	CAttackObject m_AttackObject;
	CRefObject m_refObjTarget;
	CRefObject m_refObjHitter;
	CRefObject m_refObjOwner;
	CServerTimer m_tmrAttack;

	virtual VOID OnChangeTarget(CAliveObject* pOld, CAliveObject* pNew) {}
	virtual VOID OnChangeHitter(CAliveObject* pOld, CAliveObject* pNew) {}
	virtual VOID OnChangeOwner(CAliveObject* pOld, CAliveObject* pNew) {}
public:
	virtual VOID OnKillTarget(CAliveObject* pTarget) {}
	//玩家宠物死亡
	virtual VOID OnPetDie(CAliveObject* pPet, CAliveObject* pKiller) {}
	// 清除豹子当前焦点物品
	virtual VOID SetCurFocusItem() {}

	VOID CleanVisibleList();
	VOID SetTarget(CAliveObject* pObject);
	VOID SetHitter(CAliveObject* pHitter);
	VOID SetOwner(CAliveObject* pOwner);
	VOID SetotherOwner(CAliveObject* pOwner) { m_refObjOwner.SetObject(pOwner); }
	CAliveObject* GetTarget() { return m_refObjTarget.getObject(); }
	CAliveObject* GetHitter() { return m_refObjHitter.getObject(); }
	CAliveObject* GetOwner() { return m_refObjOwner.getObject(); }
	BOOL IsGodBlessEffective(int type);
	VOID GetFrontPosition(int& x, int& y);
	// 在自己的指定范围内寻找一个指定类型的对象-螺旋搜索方式
	CAliveObject* FindNearestObj(int nRange, e_object_type eObjType);
	virtual BOOL IsSpecialEquipmentFunctionOn(special_equipment_func func) { return FALSE; }
	//判断中技能延时是否到时间
	BOOL IsSkillTime(int wMagicId) const
	{
		switch (wMagicId)
		{
		case 6:
			if (m_skill6Timer.IsTimeOut(m_dwSkill6)) return TRUE;
		case 45:
			if (m_skill45Timer.IsTimeOut(m_dwSkill45)) return TRUE;
		default:
			return FALSE;
			break;
		}
	}
	//设置多少时间内不中技能伤害
	VOID SetSkillTime(int wMagicId, DWORD nTime)
	{
		switch (wMagicId)
		{
		case 6:
		{
			m_dwSkill6 = nTime;
			m_skill6Timer.Savetime();
		}
		break;
		case 45:
		{
			m_dwSkill45 = nTime;
			m_skill45Timer.Savetime();
		}
		break;
		default:
			break;
		}
	}
	//设置缩少时间内不中状态
	VOID SetStatusTime(int index, DWORD nTime)
	{
		switch (index)
		{
		case SI_PALSY:
		{
			m_dwStatus26 = nTime;
			m_status26Timer.Savetime();
		}
		break;
		default:
			break;
		}
	}
	// 设置当前攻击类型
	VOID SetCurAttackType(damage_type curAttackType) { m_CurAttackType = curAttackType; }
	// 获取当前攻击类型
	damage_type GetCurAttackType() const { return m_CurAttackType; }
private:
	CServerTimer m_skill6Timer; // 不中施毒术计时器
	DWORD m_dwSkill6; // 不中施毒术时间
	CServerTimer m_skill45Timer; // 不中诅咒术计时器
	DWORD m_dwSkill45; // 不中诅咒术时间
	CServerTimer m_status26Timer; // 不中麻痹状态计时器
	DWORD m_dwStatus26; // 不中麻痹状态时间
	damage_type m_CurAttackType; // 当前攻击类型
};