#pragma once
#include "aiobjectex.h"
#include "itembox.h"

class CHumanPlayer;
class CVisibleEvent;
// 怪物类, 继承自CAiObjectEx基类
class CMonsterEx : public CAiObjectEx
{
public:
	CMonsterEx(void);
	virtual ~CMonsterEx(void);
	// 初始化怪物
	// pDesc: 怪物描述类指针
	// mapid: 地图ID
	// x,y: 初始坐标
	// pGen: 生成器信息
	// bGotoPoint: 是否需要移动到指定点
	// gotox,gotoy: 目标点坐标
	BOOL Init(MonsterClass* pDesc, int mapid, int x, int y, MONSTERGEN* pGen = nullptr, BOOL bGotoPoint = FALSE, int gotox = 0, int gotoy = 0);
	// 获取怪物名称
	const char* GetName() { return m_pDesc == nullptr ? "无名" : m_pDesc->base.szViewName; }
	// 获取显示名称
	const char* GetViewName();
	// 死亡事件处理
	virtual VOID OnDeath(DWORD dwKiller);
	// 更新怪物状态
	VOID Update();
	// 清理怪物数据
	VOID Clean();
	// 获取指定属性值
	int GetPropValue(PROP_INDEX index);
	// 减少属性值
	void DecPropValue(PROP_INDEX index, int value);
	// 增加属性值
	void AddPropValue(PROP_INDEX index, int value);
	// 处理对象进程
	VOID DoProcess(OBJECTPROCESS* pProcess);
	// 检查是否可以变身
	BOOL CheckChangeInto();
	// 执行变身
	BOOL ChangeInto(const char* pszClassName);
	// 获取对象类型
	e_object_type GetType()
	{
		if (GetOwner() != nullptr)return OBJ_PET;
		if (IsSystemFlagSeted(SF_FAKETARGET))return OBJ_PET;
		return OBJ_MONSTER;
	}
	//获取怪物外观类型
	e_object_ApprType GetApprType()
	{
		if (m_pDesc)
		{
			switch (m_pDesc->base.btImage)
			{
			case 0x73: // 大刀卫士
			case 0x74: // 弓箭守卫
				return APPR_GUARD;
			case 0x79: // 树木
				return APPR_TREE;
			default:
				return APPR_MONSTER;
			}
		}
		return APPR_MONSTER;
	}
	// 检查是否可以被召唤
	BOOL CanBeSlave() {
		if (m_pDesc == nullptr)
			return FALSE;
		return ((m_pDesc->sprop.pFlag & SF_CANBECALLED) == 0);
	}
	// 获取怪物外观特征值
	DWORD GetFeather()
	{
		if (m_pDesc == nullptr)
			return 0;
		if (m_pDesc->base.btImage == 0)
			return (m_pDesc->base.dwFeature & 0xffffff00);
		WORD wd = m_pDesc->base.btImage;
		BYTE wd1 = m_pDesc->base.raceimg;
		return static_cast<DWORD>(MAKELONG(MAKEWORD(wd1, 0), wd));
	}
	// 攻击目标
	BOOL AttackTarget(e_direction dir = ED_MAX);
	// 是否能做动作
	BOOL CanDoAction(actiontype action);
	// 受伤事件处理
	virtual VOID OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type);
	// 将目标添加到仇恨列表
	BOOL EnterTargetList(CAliveObject* pTarget, UINT nDamage);
	// 获取名称颜色
	BYTE GetNameColor(CMapObject* pViewer);
	// 获取自动恢复生命值的时间间隔(毫秒)
	int GetAutoRecoverHptime() {
		if (m_pDesc == nullptr) return 15 * 1000;
		if (m_pDesc->prop.recoverhptime == 0)
			m_pDesc->prop.recoverhptime = 15 * 1000;
		return m_pDesc->prop.recoverhptime * 1000;
	}
	// 获取自动恢复魔法值的时间间隔(毫秒)
	int GetAutoRecoverMptime() {
		if (m_pDesc == nullptr)return 30 * 1000;
		if (m_pDesc->prop.recovermptime == 0)
			m_pDesc->prop.recovermptime = 30 * 1000;
		return m_pDesc->prop.recovermptime * 1000;
	}
	// 获取自动恢复的生命值数量
	int GetAutoRecoverHp() 
	{
		if (m_pDesc == nullptr)return 0; 
		int hp = m_pDesc->prop.recoverhp;
		if (IsStatusSet(SI_GREENPOISON))//绿毒状态
			hp = -GetStatusParam(SI_GREENPOISON);
		return hp; 
	}
	// 获取自动恢复的魔法值数量
	int GetAutoRecoverMp() 
	{ 
		if (m_pDesc == nullptr)return 0; 
		int mp = m_pDesc->prop.recovermp;  
		if (IsStatusSet(SI_STRAWMAN)) // 诅咒术(男)红咒
			mp = -GetStatusParam(SI_STRAWMAN);
		return mp;
	}
	// 检查是否是合适的目标
	BOOL IsProperTarget(CAliveObject* pObject);
	// 主人变更事件
	VOID OnChangeOwner(CAliveObject* pOld, CAliveObject* pNew);
	// 添加经验值
	VOID AddExp(DWORD dwExp, int level, DWORD dwId = 0);
	// 获得经验值
	VOID WinExp(DWORD dwExp);
	// 检查是否可以进入地图
	BOOL CanEnterMap(CLogicMap* pMap);
	// 检查是否可以被圣言 默认按照次数计算成功.同时还可以按照几率计算, probability1 诱惑几率, probability2 黄名几率
	BOOL HolySeizedOk(CHumanPlayer* pPlayer, int level, int times, int probability1 = -1, int probability2 = -1);
	// 检查是否可以被驯服
	BOOL TrainOk(CHumanPlayer* pPlayer, int times);
	// 进入地图事件
	virtual VOID OnEnterMap(CLogicMap* pMap);
	// 检查是否可以恢复
	BOOL CanRecover() { return (!m_bDead); }
	// 清除豹子当前焦点物品
	VOID SetCurFocusItem() { m_pCurFocusItem = nullptr; }
	// 发送生命值魔法值变化消息
	VOID SendHpMpChanged(int damage = 0, WORD wEffect = 57)
	{
		if (GetType() == OBJ_MONSTER && damage < 0)
			damage = 0; // 怪加血时, 不飘血
		int wHp = GetPropValue(PI_CURHP); // 当前生命值
		int wMaxHp = GetPropValue(PI_MAXHP); // 最大生命值
		int wMp = GetPropValue(PI_CURMP);
		int wMaxMp = GetPropValue(PI_MAXMP);
		WORD heathHP = 100; // 血条长度按百分之显示
		WORD WbiliHP = (wMaxHp > 0) ? MAX(1, wHp * 100 / wMaxHp) : 0;// 百分比取整
		HealthStatus healthStatus;
		healthStatus.dwId = GetId();
		healthStatus.nHPChange = -damage;
		healthStatus.wEffect = wEffect;
		healthStatus.dwMP = wMp;
		healthStatus.dwMaxMP = wMaxMp;
		if (m_pDesc->base.btRace == MR_BOSS) // 如果是BOSS就显示具体血量
		{
			healthStatus.dwHP = wHp;
			healthStatus.dwMaxHP = wMaxHp;
			SendAroundMsg(GetId(), SM_HPMPCHANGED, wHp, 0, heathHP, (LPVOID)&healthStatus, sizeof(HealthStatus));
		}
		else // 其他显示百分比血量
		{
			healthStatus.dwHP = WbiliHP;
			healthStatus.dwMaxHP = heathHP;
			SendAroundMsg(GetId(), SM_HPMPCHANGED, WbiliHP, 0, heathHP, (LPVOID)&healthStatus, sizeof(HealthStatus));
		}
	}
	// 怪物的特殊魔法效果（治疗, 复活其他怪物）
	VOID SendSkill(DWORD tId, WORD x, WORD y, WORD wMagicID);
	// 获取性别
	BYTE GetSex() { if (m_pDesc == nullptr)return 0; if (m_pDesc->base.btImage != 0)return 0; return (m_pDesc->base.dwFeature & 1); }
	// 获取更新节点
	xListHost<CMonsterEx>::xListNode* getUpdateNode() { return &m_xUpdateNode; }
	// 系统标志设置事件
	VOID OnSystemFlagSeted(int index, DWORD dwParam = 0);
	// 系统标志清除事件
	VOID OnSystemFlagCleared(int index, DWORD dwParam = 0);
	// 目标变更事件
	VOID OnChangeTarget(CAliveObject* pOld, CAliveObject* pNew);
	// 同类共享目标
	VOID ShareTarget(CAliveObject* pNew);
	// 获取视野消息
	BOOL GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr);
	/// 刷新视野消息
	/// type: 刷新类型(0=获取视野消息, 1=获取离开视野消息, 2=同时获取两种消息)
	VOID RefreshViewmsg(int type = 0);
	/// 设置拾取物品的目标位置
	/// x: 目标X坐标
	/// y: 目标Y坐标
	VOID SetPickupItem(WORD x, WORD y)
	{
		PetgotoPOwner = TRUE;
		m_pCurFocusItem = SearchFitableItem(x, y);
	}
	// 检查是否正在返回主人身边
	BOOL IsGotoOwner() { return PetgotoPOwner; }
	// 获取怪物生成器信息
	MONSTERGEN* GetGen() { return m_pGen; }
	// 设置怪物生成器信息
	VOID SetGen(MONSTERGEN* pGen) { m_pGen = pGen; }
	// 清除怪物生成器信息
	VOID ClearGen();
	// 获取怪物类描述
	MonsterClass* GetDesc() { return m_pDesc; }
	// 获取怪物外观值
	BYTE GetImage() { return (m_pDesc == nullptr ? 0 : m_pDesc->base.btImage); }
	// 获取死亡经验值
	DWORD GetDeadExp() { return (m_pDesc == nullptr ? 0 : m_pDesc->prop.exp); }
	// 设置怪物类型
	VOID SetSType(BYTE btType) { m_btType = btType; }
	// 挖肉事件
	VOID OnCuted(CHumanPlayer* pCuter);
	// 对目标造成伤害事件
	VOID OnDamageTarget(CAliveObject* pTarget, int nDamage, damage_type type);
	// 设置移动目标
	VOID SetGotoTarget(BOOL bGotoTarget = FALSE, WORD wTargetX = 0, WORD wTargetY = 0)
	{
		if (m_bGotoPoint != bGotoTarget)
		{
			if (bGotoTarget)
				AddRef();
			else
				DecRef();
		}
		m_bGotoPoint = bGotoTarget;
		m_wGotoX = wTargetX;
		m_wGotoY = wTargetY;
	}
	// 设置怪物类描述
	BOOL SetDesc(MonsterClass* pClass);
	// 执行脚本
	VOID DoScript(CHumanPlayer* pTarget, const char* pszPage);
	// 击杀目标事件
	VOID OnKillTarget(CAliveObject* pTarget);
	// 设置宠物叛变信息
	VOID SetBetrayInfo(DWORD dwBetray)
	{
		m_dwBetray = dwBetray;
		m_betrayTimer.Savetime();
		m_bSetOwner = TRUE;
	};
	// 是否是特殊刷怪
	BOOL IsSpecialGen() const { return m_boSpecialGen; }
	// 增加复活次数
	VOID AddRevivalCount(BYTE btCount) { m_btRevivalCount += btCount; }
	// 获取复活次数
	BYTE GetRevivalCount() const { return m_btRevivalCount; }
	// 死亡是否超过多少时间
	BOOL BodyIsOutTime(DWORD dwTimeOut) { return m_bodytimer.IsTimeOut(dwTimeOut); }
	// 自定义时间是否超过多少时间
	BOOL CustomIsOutTime(DWORD dwTimeOut) { return m_CustomTimer.IsTimeOut(dwTimeOut); }
	// 自定义时间保存更新
	VOID CustomSaveTime() { m_CustomTimer.Savetime(); }
protected:
	// AI行走
	BOOL AiWalk(int dir, BOOL bCheckRun = FALSE);
	//选择攻击方式
	VOID SelectAttack(ai_attack_style attackStyle);
	// 获取合适的目标
	CAliveObject* GetProperTarget();
	// 检查攻击情况
	BOOL CheckSituation(AttackSituation& situation);
	// 检查目标是否可选
	BOOL IsTargetSelectable(CAliveObject* pTarget);
protected:
	BOOL m_bFirstEnterMap; // 是否首次进入地图
	StringCacheNode* m_pScriptPage; // 脚本页面
	BOOL m_bGotoPoint; // 是否需要移动到目标点
	WORD m_wGotoX; // 目标点X坐标
	WORD m_wGotoY; // 目标点Y坐标
	xListHost<CMonsterEx>::xListNode m_xUpdateNode; // 更新链表节点
	CMapObject* m_pCurFocusItem; // 焦点物品
	DWORD m_dwKillCount; // 击杀计数
	DWORD m_dwBetray; // 宠物叛变时间
	BOOL m_bSetOwner; // 是否设置主人
	WORD m_wCurHp; // 当前生命值
	WORD m_wCurMp; // 当前魔法值
	DWORD m_dwExp; // 经验值
	CServerTimer m_bodytimer; // 身体计时器
	CServerTimer m_IdleTimer; // 空闲计时器
	CServerTimer m_betrayTimer; // 宠物叛变计时器
	BOOL PetgotoPOwner = FALSE; // 宠物是否返回主人身边
	CAttackObject m_xAttackObj; // 攻击对象
	MONSTERGEN* m_pGen; // 怪物生成器信息
	MonsterClass* m_pDesc; // 怪物类描述
	BYTE m_btType; // 怪物类型
	BOOL m_fCuted; // 是否已被挖肉
	BOOL m_boSpecialGen; // 是否是特殊刷怪
	static xObjectPool<StringCacheNode> m_xStringCachePool; // 字符串缓存池
private:
	BOOL m_bNoAiDelayAttack = TRUE; //是否无延时攻击
	BOOL m_bIsShow = FALSE; // 是否出现动画
	char m_szOriginalName[20]; // 原始显示名字
	BYTE m_btRevivalCount; // 被复活的次数
};