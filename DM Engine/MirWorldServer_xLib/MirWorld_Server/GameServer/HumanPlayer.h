#pragma once
#include "aliveobject.h"
#include "itembox.h"
#include "equipment.h"
#include "exchangeobj.h"
#include "scripttarget.h"
#include "flageventex.h"
#include "magicmanager.h"
#include "TaskManager.h"
#include <random>
#include <mutex>
#include <unordered_map>
#include "monstermanagerex.h"
#include "monsterex.h"
#include "ScriptNpc.h"
#include "TimeAchieve.h"

class CClientObj;
class CScriptPage;
class CGroupObject;
class CMonsterEx;
class CScriptNpc;

typedef char S_PARAM[128];
typedef DWORD V_PARAM;
typedef char S_CHARNAME[20];
typedef struct tagCREATEHUMANDESC
{
	tagCREATEHUMANDESC()
	{
		FILLSELF(0);
	}
	CHARDBINFO	dbinfo;
	CClientObj* pClientObj;
}CREATEHUMANDESC;

typedef struct tagItemPos
{
	DWORD dwMakeIndex;
	DWORD dwPos;
}ItemPos;

struct SpecialPotionInfo // 特殊药水信息
{
	PROP_INDEX propIndex;	// 属性索引
	const char* szMsg;		// 消息格式
};

class CGuildEx;
//	可以用另外的shell来替换自身, shell有存活时间.

class CHumanPlayer : public CAliveObject
{
public:
	CHumanPlayer(void);
	virtual ~CHumanPlayer(void);
	VOID Clean();
	BOOL Init(CREATEHUMANDESC& desc);
	e_object_type GetType() { return OBJ_PLAYER; }
	VOID OnAroundMsg(CMapObject* pSender, const char* pszCodedMsg, int size);
	VOID OnEnterMap(CLogicMap* pMap);
	VOID OnLeaveMap(CLogicMap* pMap);

	BOOL OnEquipItem(int pos, ITEM& item);
	BOOL OnUnEquipItem(int pos, ITEM& item);
	//这里还有个GETSHPE
	BYTE GetShape(DWORD dwMakeIndex);
	VOID Sendfirstdlg(const char* pszString);
	VOID OnDeath(DWORD dwKiller);
	VOID CleanPets();
	// 获取玩家外观特征值
	DWORD GetFeather();
	//攻击方向
	int attackdir = 0;
	int getdir() { return attackdir; }
	//此用于41封包返回衣服SHAPE
	BYTE GetShape();
	DWORD GetHealth();
	DWORD GetStatus();
	//获取玩家性别
	BYTE GetSex() { return m_Humandesc.dbinfo.btSex; }
	//获取玩家职业
	BYTE GetPro() const { return m_Humandesc.dbinfo.btClass; }
	VOID Update();
	VOID DropGold(DWORD dwCount);
	BOOL GetDBInfo(CHARDBINFO& info);
	//玩家在数据库中的ID
	DWORD GetDBId() const { return m_Humandesc.dbinfo.dwDBId; }
	const char* GetName() { return m_Humandesc.dbinfo.szName; }
	const char* GetViewName() { return m_szLongName; }
	BOOL GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer = nullptr);

	BOOL CanRecover() { return (!m_bDead && m_ActionType == AT_STAND); }
	BOOL CanRecvMsg()
	{
		if (m_pClientObj == nullptr)return FALSE;
		return TRUE;
	}
	CClientObj* GetClientObject() { return m_pClientObj; }
	VOID GetViewDetail(xPacket& packet);
	VOID UpdateViewName();
	//获取时长封号特殊封号值
	BYTE GetFenghaoType23() override 
	{ 
		return m_FenghaoInfo.btType2 > 0 ? m_FenghaoInfo.btType2 : m_FenghaoInfo.btType3;
	}
	//发送时长封号加密数据
	VOID SendFengHaoData();
	//发送时长封号玩家信息
	VOID SendFengHaoGrowInfo();
	//时长封号穿戴
	VOID SendFengHaoEquip(int nCount);
	//获取玩家时长封号信息
	FenghaoInfo* GetFenghaoInfo() { return &m_FenghaoInfo; }
	//检查时长封号是否过期
	VOID CheckFengHaoTimeOut();
	//玩家登录时, 请求获取封号数据
	VOID OnFengHaoInfo(FenghaoInfo* pInfo);
	//更新时长封号数据到DB
	VOID UpdateFengHaoToDB();
	//重新计算时长封号属性:index封号序号，boOperate 是加属性还是减属性
	VOID RecalcFengHaoProp(BYTE index, BOOL boOperate, BOOL boProp = TRUE);

	BOOL CanDoAction(actiontype action);

	VOID GiveForgePoint(DWORD dwValue)
	{
		m_Humandesc.dbinfo.dwForgePoint += dwValue;
		UpdateSubProp();
	}

	BOOL TakeForgePoint(DWORD dwValue)
	{
		if (dwValue > m_Humandesc.dbinfo.dwForgePoint)
			m_Humandesc.dbinfo.dwForgePoint = 0;
		else
			m_Humandesc.dbinfo.dwForgePoint -= dwValue;
		UpdateSubProp();
		return TRUE;
	}
	DWORD GetForgePoint()const { return m_Humandesc.dbinfo.dwForgePoint; }

	BOOL GiveCredit(WORD wValue)
	{
		WORD wCur = m_Humandesc.dbinfo.dwFlag[0] & 0xffff;
		if (wValue > ((WORD)0xffff) - wCur)
			wCur = 0xffff;
		else
			wCur += wValue;
		m_Humandesc.dbinfo.dwFlag[0] = (m_Humandesc.dbinfo.dwFlag[0] & 0xffff0000) + wCur;
		UpdateSubProp();
		return TRUE;
	}
	BOOL TakeCredit(WORD wValue)
	{
		WORD wCur = m_Humandesc.dbinfo.dwFlag[0] & 0xffff;
		if (wCur < wValue)return FALSE;
		wCur -= wValue;
		m_Humandesc.dbinfo.dwFlag[0] = (m_Humandesc.dbinfo.dwFlag[0] & 0xffff0000) + wCur;
		UpdateSubProp();
		return TRUE;
	}
	DWORD GetCredit()const { return (m_Humandesc.dbinfo.dwFlag[0] & 0xffff); }
public:
	VOID OnPickupItem(ITEM& item, UINT x, UINT y);
	VOID OnDropItem(ITEM& item, UINT x, UINT y);
	VOID OnKillTarget(CAliveObject* pTarget);
	BOOL HasUpgradeWeapon()const { return (m_UpgradeItem.dwMakeIndex != 0); }
	ITEM& GetUpgradeWeapon() { return m_UpgradeItem; }
	int GetVarValue(const char* pszVar) const;
	void SetVarValue(const char* pszVar, int value) {}
	BOOL CreateBagItem(const char* pszName, BOOL IsBing = FALSE);
	VOID AddExp(DWORD dwExp, int level = 0, DWORD dwId = 0);
	VOID WinExp(DWORD dwExp, BOOL bNoBonus = FALSE, DWORD dwId = 0);
	VOID SetExp(DWORD dwExp);
	BOOL AddGold(DWORD dwCount, BOOL bUpdateClient = TRUE);
	BOOL PetAddgold(DWORD dwgold, BOOL bUpdateClient = TRUE);
	BOOL PetAddBagItem(ITEM& item, BOOL bSilence = FALSE, BOOL bUpdateDB = FALSE, BOOL bUpdateWeight = TRUE);
	BOOL TakeGold(DWORD dwCount, BOOL bUpdateClient = TRUE);
	BOOL TestAddGold(DWORD dwCount)const;
	DWORD GetMoney(money_type type)const;
	BOOL CostMoney(money_type type, DWORD dwCount, BOOL bUpdateClient = TRUE);
	BOOL AddMoney(money_type type, DWORD dwCount, BOOL bUpdateClient = TRUE);
	VOID SetBigGold(BOOL bBigGold) { m_Humandesc.dbinfo.bBigGold = bBigGold; }
	BOOL AddPet(CAliveObject* pObject);
	BOOL DelPet(CAliveObject* pObject);
	VOID SetPetTarget(CAliveObject* pObject);
	int GetPetCount()
	{
		int count = m_iPetCount;
		if (m_pPet != nullptr)
			count++;
		return count;
	}
	CSystemTime* GetLoginTime() { return &m_LoginTime; }
	DWORD GetLoginLong() {
		CSystemTime curtime;
		return m_LoginTime.GetToTimeSecond(curtime);
	}
	const char* GetMaster() { return m_sMaster; }
	const char* GetMarriage() { return m_sWife; }
	const char* GetStudent(UINT nIndex) { if (nIndex >= 3)return ""; return m_sStudents[nIndex]; }
	const char* GetGuildName();
public:
	DWORD GetLevelupExp();

	BOOL NoLawProtect();
	// 获取背包剩余空间数量
	int GetBagFree() { return m_ItemBox.GetFree(); }
	// 添加物品到背包
	BOOL AddBagItem(ITEM& item, BOOL bSilence = FALSE, BOOL bUpdateDB = FALSE, BOOL bUpdateWeight = TRUE);
	// 掉落物品
	BOOL DropItem(ITEM& item);
	// 拾取物品
	BOOL PickupItem();
	// 掉落背包物品
	BOOL DropBagItem(DWORD dwMakeIndex);
	//通过3073调用这个函数, 把宠物拾取的坐标传递给宠物
	VOID SetPetFindItem(WORD x, WORD y)
	{
		if (m_pPet)
			m_pPet->SetPickupItem(x, y);
	}
	// 发送豹子的名字
	VOID SendPetName(ITEM* pItem);

	BOOL EquipItem(int pos, ITEM& item, BOOL bForced = FALSE, BOOL bNoticePlayer = TRUE);
	BOOL EquipItem(int pos, DWORD dwMakeIndex);
	BOOL UnEquipItem(int pos, DWORD dwMakeIndex);

	BOOL Trade(CHumanPlayer* pPlayer = nullptr);
	BOOL PutTradeItem(DWORD dwMakeIndex);
	BOOL PutTradeMoney(money_type type, DWORD dwCount);
	BOOL CancelTrade();
	BOOL ConfirmTrade();

	ITEM* FindBagItem(DWORD dwMakeIndex);
	BOOL DeleteBagItem(DWORD dwMakeIndex);

	CItemBox& GetBag() { return m_ItemBox; }
	int	GetPropValue(PROP_INDEX index);
	void DecPropValue(PROP_INDEX index, int value);
	void AddPropValue(PROP_INDEX index, int value);

	int GetAutoRecoverHp();
	int GetAutoRecoverMp();
	int GetAutoRecoverHptime() { return 6000; }
	int GetAutoRecoverMptime() { return 15000; }
	//设置活力值
	void SetHuoLi(int h) { m_iHuoli = h; }
	//施放技能或者魔法, x, y 是鼠标的坐标
	BOOL SpellCast(int x, int y, UINT nTarget, WORD wMagicId);
	//战士技能攻击
	BOOL SpecialHit(int dir, WORD wSkillId);
	//公用飞行技能, 移形换影、遁地、化身蝙蝠
	BOOL SpellFly(int x, int y, WORD wMagicId);
	int getskillpower(WORD id)
	{
		//计算SKILL文件中Value1到Value2之间的随机值
		if (id == 64 || id == 65 || id == 30 || id == 8)
			return 0;
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(id);
		USERMAGIC* pMagic = GetMagic(id);
		int pow = magicskill.skills[pMagic->magic.btLevel].value1;
		int power = magicskill.skills[pMagic->magic.btLevel].value2;
		if (GetPropValue(PI_LUCKY) >= 9)//第一种幸运大于9
			return power;
		else if (GetPropValue(PI_LUCKY) >= 7)//幸运大于7
		{
			if ((power - pow) > 0)
			{
				int temp = ((power - pow) / 3) * 2;
				int adddamage = temp + rand() % ((power - pow) - temp + 1);
				return pow + adddamage;
			}
			else
				return pow;
		}
		else { //幸运小于7
			if ((power - pow) > 0)
				return pow + rand() % (power - pow + 1);
			else
				return pow;
		}
	}

	VOID Cry(const char* pszMsg, ...);
	VOID SayGroup(const char* pszMsg, ...);

	BOOL CheckMaterial(BYTE stdMode, BYTE shape, BYTE special, int nCount);
	VOID TakeMaterial(BYTE stdMode, BYTE shape, BYTE special, int nCount);
	//穿戴装备减少持久
	VOID DamageDura(int pos, int nDura, int nDuraRate = 1000);
	//身上材料减少持久
	VOID DamageMaterialDura(int pos, int nDura);
	//增加技能经验
	BOOL TrainMagic(USERMAGIC* pMagic, int exp = 0);

	BOOL CanBearItem(ITEM& item);

	BOOL DoUpgradeWeapon();
	BOOL TakeUpgradeWeapon();
	ITEM* FindEquipmentWithShape(BYTE btShape, int& pos);

	BOOL RemoveMagic(const char* pszMagic);
	BOOL RemoveMagic(UINT nMagicId);

	VOID SaveVars();
	VOID LoadVars();

	ITEM* GetPutItem() { return m_pPutItem; }
	//刻录回城神石
	BOOL RecordHomeXY(const char* pszName);
public:
	//更新52人物信息
	VOID UpdateProp();
	//更新人物附加属性752
	VOID UpdateSubProp();

	VOID SendFriendSystemError(friend_error fe, const char* pszName);
	VOID SendAddFriend(const char* pszName);
	VOID SendDelFriend(const char* pszName);

	VOID SendGoldChanged(DWORD dwChanged = 0);
	VOID SendMoneyChanged(money_type type = MT_GOLD);
	VOID SendWeightChanged();
	VOID SendSpecialStatusChanged(BOOL bToAround = TRUE);
	VOID SendMagicList();
	VOID SendEatOk();
	VOID SendEatFail();
	VOID SendBank(DWORD dwNpcId);
	VOID SendDuraChanged(int pos, WORD wCurDura, WORD wMaxDura);
	VOID SendGroupMode();
	BOOL IsGroupEnabled()const;
	VOID SetGroupMode(BOOL bEnabled);

	VOID SendTakeBagItem(ITEM* pItem);
	VOID SendWisper(const char* pszMsg, ...);
	VOID SendScrollText(const char* pszText);
	// 发送自定义快捷键
	VOID SendClientKeyConfig();
	// 发送客户端插件信息
	VOID SendClientPluginInfo();
	//使用物品
	VOID UseItem(DWORD dwItemIndex, DWORD dwPackIndex);

	int	GetEquipments(EQUIPMENT* pEquipments);

	BOOL AddGroupMember(CHumanPlayer* pObject);

	BOOL RemoveGroupMember(CHumanPlayer* pObject);

	VOID DoProcess(OBJECTPROCESS* pProcess);
	BOOL NeedGlobeProcess(int nProcessPerFrame)
	{
		if (m_pClientObj == nullptr) return FALSE;
		if (!m_pClientObj->IsConnected()) return FALSE;
		return GetProcessQueueCount() < nProcessPerFrame;
	}

	BOOL IsEnterMap()const { return m_bEnterMap; }
	BOOL IsFirstLogin()const { return m_bFirstLogin; }

	VOID LevelUp(int level = 0);
	VOID OnLevelUp(int level);
	VOID UpdateToDB();

	USERMAGIC* GetMagic(int magicid)
	{
		USERMAGIC* p = m_pMagic;
		while (p)
		{
			if (p->magic.wId == magicid)return p;
			p = p->pNext;
		}
		return nullptr;
	}
	USERMAGIC* GetMagicByName(const char* pszName)
	{
		USERMAGIC* p = m_pMagic;
		while (p)
		{
			if (strcmp(pszName, p->pClass->szName) == 0)
				return p;
			p = p->pNext;
		}
		return nullptr;
	}
	USERMAGIC* GetMagicByKey(BYTE btKey)
	{
		USERMAGIC* p = m_pMagic;
		while (p)
		{
			if (p->magic.btKey == btKey)return p;
			p = p->pNext;
		}
		return nullptr;
	}
	USERMAGIC* GetFirstMagic() { return m_pMagic; }

	const char* GetAccount();
	VOID SetMagic(MAGICDB* pArray, int count);
	BOOL AddMagic(WORD wId, BYTE btLevel = 0);
	BOOL AddMagic(const char* pszName, BYTE btLevel = 0);

	DWORD GetOwnerKey() { return GetDBId(); }
	VOID OnStatusSet(int index, DWORD dwParam = 0);
	VOID OnStatusClr(int index, DWORD dwParam = 0);

	VOID Send_Exchg_OtherAddItem(CHumanPlayer* pOther, ITEM& item);
	VOID Send_Exchg_OtherAddMoney(CHumanPlayer* pOther, money_type type, DWORD dwCount);
	CExchangeObj* GetExchangeObject() { return m_pExchangeObj; }
	VOID SetExchangeObject(CExchangeObj* pObj) { m_pExchangeObj = pObj; }
	BOOL CheckTradeOtherSideOk(CHumanPlayer* p);

	CGroupObject* GetGroupObject() { return m_pGroupObject; }
	VOID SetGroupObject(CGroupObject* pGroupObject) { m_pGroupObject = pGroupObject; }

	BOOL AddGroupMember(const char* pszName);
	BOOL RemoveGroupMember(const char* pszName);
	VOID UpdateGroupPosition();

	DWORD GetStartPointIndex()const { return m_dwStartPointIndex; }
	VOID SetStartPointIndex(DWORD dwStartPointIndex);

	VOID Home();
	BOOL RandomTeleport(int nMapId = -1);
	VOID CheckAndUpgradeTitle();
	VOID SendTitleChanged();

	VOID AddMp(int nAdd)
	{
		WORD wMp = static_cast<WORD>(GetPropValue(PI_CURMP));
		if (nAdd < 0)
		{
			nAdd = -nAdd;
			DecPropValue(PI_CURMP, nAdd);
		}
		else
		{
			m_Humandesc.dbinfo.mp += nAdd;
			if (m_Humandesc.dbinfo.mp > m_Humandesc.dbinfo.maxmp)
				m_Humandesc.dbinfo.mp = m_Humandesc.dbinfo.maxmp;
		}
		SendHpMpChanged();
	}

	VOID AddHp(int nAdd)
	{
		if (nAdd < 0)
		{
			nAdd = -nAdd;
			DecPropValue(PI_CURHP, nAdd);
		}
		else
			AddPropValue(PI_CURHP, nAdd);
		SendHpMpChanged(-nAdd);
	}

	VOID setVParam(UINT index, DWORD dwParam)
	{
		if (index > 9)return;
		m_vParam[index] = dwParam;
	}

	DWORD getVParam(UINT index)
	{
		if (index > 9)return 0;
		return m_vParam[index];
	}

	VOID setSParam(UINT index, const char* pszParam)
	{
		if (index > 9)return;
		o_strncpy(m_sParam[index], pszParam, 128);
	}

	const char* getSParam(UINT index)
	{
		if (index > 9)return "<OUT_OF_INDEX>";
		return m_sParam[index];
	}

	BOOL getSelfFlag(int index)const
	{
		if (index < 0 || index >= 64)return FALSE;
		int findex = index % 32;
		int dindex = index / 32;
		if (m_Humandesc.dbinfo.dwFlag[dindex + 2] & (1 << findex))
			return TRUE;
		return FALSE;
	}

	VOID setSelfFlag(int index)
	{
		if (index < 0 || index >= 64)return;
		int findex = index % 32;
		int dindex = index / 32;
		if ((m_Humandesc.dbinfo.dwFlag[dindex + 2] & (1 << findex)) == 0)
			m_Humandesc.dbinfo.dwFlag[dindex + 2] ^= (1 << findex);
	}

	VOID clrSelfFlag(int index)
	{
		if (index < 0 || index >= 128)return;
		int findex = index % 32;
		int dindex = index / 32;
		if (m_Humandesc.dbinfo.dwFlag[dindex + 2] & (1 << findex))
			m_Humandesc.dbinfo.dwFlag[dindex + 2] ^= (1 << findex);
	}

	VOID OnAttackTarget(CAliveObject* pTarget, int nDamage)
	{
		UpdateAutoMagic();
	}
	//伤害目标, 伤害值, 伤害类型
	VOID OnDamageTarget(CAliveObject* pTarget, int nDamage, damage_type type);
	//计算自己伤害
	VOID OnDamage(CAliveObject* pAttacker, int nDamage, damage_type type);
	//增加仓库物品
	BOOL AddBankItem(ITEM& item, BOOL bUpdateDB = TRUE);
	//存放仓库
	BOOL PutBankItem(DWORD dwMakeIndex);
	//拿走仓库物品
	BOOL TakeBankItem(DWORD dwMakeIndex);
	//获取经验倍数
	FLOAT GetExpFactor()const { return m_fExpFactor; }
	//设置经验倍数
	VOID SetExpFactor(FLOAT fFactor) { m_fExpFactor = fFactor; }
	//增加技能
	VOID OnAddMagic(USERMAGIC* pMagic);
	//更新被动技能
	VOID UpdateAutoMagic();
	//技能升级
	VOID OnMagicLevelup(USERMAGIC* pMagic);
	//切换攻击模式
	VOID ChangeAttackMode(int mode);
	//获取攻击模式
	e_humanattackmode GetAttackMode()const { return (e_humanattackmode)m_Humandesc.dbinfo.btAttackMode; }
	//切换聊天频道
	VOID ChangeChatChannel(e_chatchannel channel = CCH_MAX);
	//获取聊天频道
	e_chatchannel GetChatChannel()const { return m_ChatChannel; }
	VOID DisableChannel(e_chatchannel channel = CCH_MAX);
	VOID EnableChannel(e_chatchannel channel = CCH_MAX);
	BOOL IsChannelDisabled(e_chatchannel channel = CCH_MAX)const;

	//设置目标为私聊对象
	VOID SetWisperTarget(const char* pszName)
	{
		if (m_ChatChannel != CCH_WISPER)return;
		if (strcmp(pszName, m_szCurWisperTarget) == 0)return;
		o_strncpy(m_szCurWisperTarget, pszName, 31);
		SaySystemAttrib(CC_GREEN, "%s 被设置成当前密谈对象", pszName);
	}
	const char* GetWisperTarget() { if (m_szCurWisperTarget[0] == 0)return nullptr; return m_szCurWisperTarget; }

	VOID ChannelSay(e_chatchannel channel, const char* pszParam, const char* pszWords/*, ... */);
	BOOL ChannelHear(e_chatchannel channel, DWORD dwParam, const char* pszWords, ...);
	BOOL ChannelHearDirectly(e_chatchannel channel, DWORD dwParam, const char* pszWords, DWORD dwParam1 = 0, DWORD dwParam2 = 0);
	BOOL IsGameMaster();
	//是否可为朋友
	BOOL IsProperFriend(CAliveObject* pObject);
	//是否可为目标
	BOOL IsProperTarget(CAliveObject* pObject);
	//设置豹子的更新信息
	VOID SetPetMonster(const char* pszName)
	{
		MonsterClass* pDesc = CMonsterManagerEx::GetInstance()->GetClassByName(pszName);
		if (pDesc && m_pPet != nullptr)
		{
			m_pPet->SetDesc(pDesc);
			m_pPet->SendChangeName();
		}
	}
	BOOL IsHasPet() { return m_pPet != nullptr; }
	//重新施放护身真气,或者金刚护体
	VOID resetHushenBuff(int x, int y, UINT nTarget, WORD wMagicId);
	//获取是否开启护身真气,子类实现
	BOOL GetHushen();
	//获取是否开启金刚护体,子类实现
	BOOL GetJingang();
	//护身等级或者金刚等级
	int hushenleve = 0;
	//护身等级颜色或金刚等级颜色数组
	int hushenbuff[4] = { 255, 254, 147, 154 };
	//子类实现, 用于获取护身等级
	int getHushenbuf() { return hushenbuff[hushenleve]; }
	//获取魔法盾和金刚护体免伤的百分比
	int GetNoDamage() {
		if (IsStatusSet(SI_BUBBLEDEFENCEUP))//获取魔法盾百分比
			return NoDamage;
		if (IsSystemFlagSeted(SF_STRONGSHIELD))//金刚护体百分比
			return JingganNoDamage;
		return 0;
	}
	int SecondResMag_count() {
		if (ResMag_Count != 0)
			ResMag_Count--;
		return ResMag_Count;
	}
	//发送生命值、魔法值 变化消息
	VOID SendHpMpChanged(int damage = 0, WORD wEffect = 57)
	{
		int wHp = GetPropValue(PI_CURHP); // 当前生命值
		int wMaxHp = GetPropValue(PI_MAXHP); // 最大生命值
		int wMp = GetPropValue(PI_CURMP);
		int wMaxMp = GetPropValue(PI_MAXMP);
		WORD heathHP = 100; // 血条长度按百分之显示
		WORD WbiliHP = (wMaxHp > 0) ? (wHp * 100 / wMaxHp) : 0;// 百分比取整
		HealthStatus healthStatus;
		healthStatus.dwId = GetId();
		healthStatus.nHPChange = -damage;
		healthStatus.wEffect = wEffect;
		healthStatus.dwHP = wHp;
		healthStatus.dwMaxHP = wMaxHp;
		healthStatus.dwMP = wMp;
		healthStatus.dwMaxMP = wMaxMp;
		SendAroundMsg(GetId(), SM_HPMPCHANGED, WbiliHP, wMp, heathHP, (LPVOID)&healthStatus, sizeof(HealthStatus));
		if (CanRecvMsg())
			SendMsg(GetId(), SM_HPMPCHANGED, wHp, wMp, wMaxHp, (LPVOID)&healthStatus, sizeof(HealthStatus));
	}

	CGuildEx* GetGuild() { return m_pGuild; }
	VOID SetGuild(CGuildEx* pGuild, const char* pszTitle = nullptr, int iLevel = 0);/*{ m_pGuild = pGuild;}*/

	int	GetGuildFrontPage(char* pszBuffer, int buffersize);
	int	GetGuildLevel()const { return m_iGuildTitleLevel; }

	BOOL PostAddToGuildRequest(CHumanPlayer* poster);
	VOID ReplyAddToGuildRequest(BOOL bAccept);
	VOID AcceptAddToGuildRequest(CHumanPlayer* pMember);
	CHumanPlayer* GetAddToGuildRequester() { return m_pAddToGuildRequester; }
	BOOL IsMyFriend(CHumanPlayer* pPlayer)const;
	BOOL IsMyFriend(const char* pszName)const;

	BOOL PostAddFriendRequest(CHumanPlayer* poster);
	VOID ReplyAddFriendRequest(BOOL bAccept, const char* pszName);
	VOID AcceptAddFriendRequest(CHumanPlayer* pFriend);

	CMonsterEx* GetHorse() { return m_pHorse; }
	void SetHorse(CMonsterEx* pHorse) { m_pHorse = pHorse; if (m_pHorse == nullptr)m_bRideHorse = FALSE; }

	ITEM* GetWeapon();
	ITEM* GetDress();
	ITEM* GetEquipment(int pos);
	//骑马、下马
	BOOL RideHorse();

	BYTE GetRunSpeed() { if (m_bRideHorse)return 3; return 2; }

	DWORD GetPkValue()const { return m_dwPkValue; }

	VOID AddPkPoint(DWORD btPoint = 1);
	VOID DecPkPoint(DWORD btPoint = 1);
	BYTE GetNameColor(CMapObject* pViewer);

	VOID UpdateItemsToDB();
	const char* GetScriptVarValue(const char* pszName);
	const char* GetGuildTitle() { return m_szGuildTitle; }

	VOID OnEnterSafeArea();
	VOID OnLeaveSafeArea();
	VOID OnEnterCityArea();
	VOID OnLeaveCityArea();
	VOID OnEnterWarArea();
	VOID OnLeaveWarArea();

	VOID OnSystemFlagCleared(int index, DWORD dwParam = 0);
	VOID OnSystemFlagSeted(int index, DWORD dwParam = 0);

	//马鞭抓马
	BOOL DoTrainHorse(int dir);
	BOOL CanEnterMap(CLogicMap* pMap);
	BOOL EscapeMap();
	BOOL SetPrivateShop(int iCount, PRIVATESHOPQUERY* pQuery);
	BOOL StopPrivateShop();
	VOID SendStartPrivateShop();
	VOID SendStopPrivateShop();
	BOOL SendPrivateShopPage(CHumanPlayer* pQueryer, WORD wFlag);
	CHumanPlayer* GetCurPrivateShopView() { return m_curPrivateShopView; }
	VOID SetCurPrivateShopView(CHumanPlayer* player) { m_curPrivateShopView = player; }
	VOID UpdatePrivateShopToAround();
	BOOL BuyPrivateShopItem(CHumanPlayer* pBuyer, DWORD dwItemId, const char* pszName);
	int	GetPrivateShopItemCount()const { return m_iPrivateShopItemCount; }
	BOOL TestAddMoney(money_type type, DWORD dwCount)const;

	VOID OnCommunityInfo(const char* pszCommunityInfo);
	int	GetCommunityInfo(char* pszCommunityBuffer, int iSize);
	VOID UpdateCommunityInfoToClient(BOOL bNoticeOnline = TRUE);
	VOID SetPetsActive(BOOL bActive) { CAliveObject::SetPetsActive(bActive); SetPetTarget(nullptr); }
	BOOL AddFriend(CHumanPlayer* pFriend);
	BOOL DeleteFriend(const char* pszName);

	VOID ToggleFriendMode();

	VOID NoticeFriendOffline();

	VOID SetBagItemPos(BAGITEMPOS* pPosArray, int count);
	WORD GetBagItemPos(DWORD dwMakeIndex);

	BOOL IsDefenceEffectiveObject(CAliveObject* pObject);

	BOOL SetMagicLevel(const char* pszName, int level);
	BOOL SetMagicLevel(USERMAGIC* pMagic, int level);
	VOID SendMagicExpChg(USERMAGIC* pMagic);
	VOID OnPetDie(CAliveObject* pPet, CAliveObject* pKiller);
	VOID RefreshSpecialEquipment();
	VOID ChangeHair(BYTE	btHair);
	VOID ChangeWeaponView(BYTE btView);
	BOOL CheckEquipment(int pos, int stdmode, int image, int& posout);
	BOOL IsSpecialEquipmentFunctionOn(special_equipment_func func)
	{
		if (func >= SEF_MAX)return FALSE;
		if (func < 0)return FALSE;
		if (m_dwSpecialEquipmentFunctionFlags[func] & 0x80000000)
			return TRUE;
		return FALSE;
	}
	VOID ProcSpecialEquipmentFunctionOff();
	VOID ProcSpecialEquipmentFunctionOn();
	VOID SetSpecialEquipmentFunctionOn(special_equipment_func func, DWORD dwPosFlag)
	{
		if (func >= SEF_MAX)return;
		if (func < 0)return;
		dwPosFlag |= 0x80000000;
		if (m_dwSpecialEquipmentFunctionFlags[func] & 0x80000000)
			m_dwSpecialEquipmentFunctionFlags[func] = dwPosFlag;
		else
		{
			m_dwSpecialEquipmentFunctionFlags[func] = dwPosFlag;
			OnSpecialEquipmentFunctionOn(func);
		}
	}
	VOID SetSpecialEquipmentFunctionOff(special_equipment_func func)
	{
		if (func >= SEF_MAX)return;
		if (func < 0)return;
		if (m_dwSpecialEquipmentFunctionFlags[func] & 0x80000000)
		{
			m_dwSpecialEquipmentFunctionFlags[func] = 0;
			OnSpecialEquipmentFunctionOff(func);
		}
		else
			m_dwSpecialEquipmentFunctionFlags[func] = 0;
	}
	VOID OnSpecialEquipmentFunctionOn(special_equipment_func func);
	VOID OnSpecialEquipmentFunctionOff(special_equipment_func func);
	VOID OnEquipmentOn(int pos, ITEM& item);
	VOID OnEquipmentOff(int pos, ITEM& item);
	VOID OnDoAction(actiontype action);
	BOOL WillDie();
	VOID DamageSpecialEquipment(special_equipment_func func, int iDamage);
	WORD GetBodyEffect();
	VOID SendBodyEffectChanged();
	BOOL DoMine(int dir);
	BOOL IsGodBlessEffectivable(special_godbless type, CAliveObject* pObject);
	BOOL Damage(DWORD dwHitter, int value);
	VOID SendUpdateItem(ITEM& item);
	VOID UpdateMineEffect();
	VOID SetUpgradeItem(ITEM& item) { m_UpgradeItem = item; }
	VOID UpgradeWeapon();
	VOID SendWeaponBroken();
	//是否已经穿戴马牌
	BOOL IsEquipedHorse();
	//获取穿戴马牌物品
	ITEM* GetEquipedHorseItem();
	BOOL SummonPet(const char* pszName, BOOL bSetOwner = TRUE, int x = -1, int y = -1);
	CMonsterEx* SummonMonster(const char* pszName, BOOL bSetOwner = TRUE, int x = -1, int y = -1);

	BOOL Marry(CHumanPlayer* pEachOther);
	BOOL UnMarry();
	BOOL IsMarried()const { return m_sWife[0] != 0; }

	UINT GetStudentCount()const;
	BOOL HasMaster()const { return m_sMaster[0] != 0; }
	BOOL AddStudent(CHumanPlayer* pStudent);
	BOOL DeleteStudent(const char* pszName);
	BOOL IsMyStudent(const char* pszName)const;
	BOOL HasStudent(UINT nPos)const
	{
		if (nPos >= 3)return FALSE;
		return (this->m_sStudents[nPos][0] != 0);
	}
	BOOL LeaveTeacher();

	VOID SetMaster(CHumanPlayer* pMaster);
	VOID SetWife(CHumanPlayer* pWife);

	VOID SendAddCommunity(WORD wType, const char* pszName);
	VOID SendDeleteCommunity(WORD wType, const char* pszName);
	BOOL TakeEquipment(const char* pszEquipment, ITEM& item);
	BOOL TakeEquipment(int pos, ITEM& item);
	ITEM* GetEquipment(const char* pszName) { int pos = 0; return GetEquipment(pszName, pos); }
	ITEM* GetEquipment(const char* pszName, int& posout);
	BOOL AddTeacherCredit(UINT nCount)const;
	VOID SetBagCountLimit(int iCountLimit)
	{
		if (iCountLimit == BIGBAG_SLOT)
			m_Humandesc.dbinfo.bBigBag = 1;
		else if (iCountLimit == SMALLBAG_SLOT)
			m_Humandesc.dbinfo.bBigBag = 0;
		m_ItemBox.SetCountLimit(iCountLimit);
	}
	int GetBagCountLimit() { return m_ItemBox.GetCountLimit(); }
	VOID SendSetPetBag(UINT nSize);
	VOID SendPetBag(ITEMCLIENT* pItems, UINT nCount);
	VOID SendCloseScriptPage(UINT nId);
	VOID SendPage(CScriptShell* pShell, CScriptView* pView);
	VOID SendClosePage(CScriptShell* pShell);
	VOID ClearFirstLogin() { m_bFirstLogin = FALSE; }
	VOID ToggleHorseRest() { this->m_bHorseRest = !this->m_bHorseRest; }
	BOOL IsHorseRest()const { return this->m_bHorseRest; }
	VOID ShowPetInfo();

	VOID TransformInto(WORD wRace, WORD wImage, DWORD dwTime = 0xffffffff);
	ITEM* GetUsingItem() { return m_pUsingItem; }
	ITEM* GetPackItem() { return m_pPackItem; }
	VOID SetUsItem(USEITEM_RESULT id) { m_pUsingItem->dwParam[3] = id; }
	DWORD GetbaoziId()const { return m_baozhiID; }//取出豹子的IDINDEX
	ITEM* GetbaoziItem() {  //目的在于取出豹子的物品信息
		DWORD makeindex = GetbaoziId();
		if (makeindex != 0)
			return m_ItemBox.FindItem(makeindex);
		return nullptr;
	}
	VOID SetPetName(const char* pszName)const { strcpy(petname, pszName); }
	VOID SetUsingItem(ITEM* pItem) { m_pUsingItem = pItem; if (pItem)pItem->dwParam[3] = UR_NORESULT; }
	VOID OnPutItem(DWORD dwShellId, DWORD dwMakeIndex);

	VOID GetDBInfoPacket(xPacket& packet);

	BOOL CutBody(UINT nMonsterId, WORD x, WORD y, WORD dir);
	BOOL CanBePushed(CAliveObject* pAttacker);
	VOID PostMsg(const char* pszMsg, int length = 0);
	VOID SendTimeWeatherChanged();
	VOID SetPrivateShopSign(DWORD dwSign) { m_wPrivateShopFlags = 1; m_wPrivateShopSign = dwSign; }
	VOID SetPrivateShopStyle(WORD wStyle) { m_wPrivateShopStyle = wStyle; }
	VOID SetChatColor(BYTE btColor) { m_btChatColor = btColor; }
	VOID ReviewAroundNameColor();

	CScriptTargetForPlayer* GetScriptTarget() { return &m_ScriptTarget; }

	VOID OnPetBank(DBITEM* pItems, UINT nCount);
	//冰泉圣水修复穿戴物品持久
	BOOL RepairEquipment(UINT pos, UINT nCount = 1);

	DWORD GetParam(UINT nIndex)const { if (nIndex >= 4)return 0; return m_dwParams[nIndex]; }
	VOID SetParam(UINT nIndex, DWORD dwParam) { if (nIndex >= 4)return; m_dwParams[nIndex] = dwParam; }
	VOID ClearParam()const { memset(m_dwParams, 0, sizeof(m_dwParams)); }

	//玩家登录时, 请求获取任务数据
	VOID OnTaskInfo(TASKINFO* pInfo);
	//添加任务
	BOOL AddTask(WORD wTaskId);
	//删除任务
	BOOL DeleteTask(WORD wTaskId, BOOL bCompleteTask = FALSE);
	//更新任务
	BOOL UpdateTask(WORD wTaskId, WORD wTaskStep);
	//是否有任务
	BOOL HasTask(WORD wTaskId);
	//获取任务步骤
	UINT GetTaskStep(WORD wTaskId);
	//设置任务变量数值
	VOID SetTaskValue(WORD wTaskId, BYTE btNum, DWORD dwData);
	//获取任务变量数值
	int GetTaskValue(WORD wTaskId, BYTE btNum);
	//设置任务变量字符串
	VOID SetTaskString(WORD wTaskId, BYTE btNum, const char* szpData = nullptr);
	//获取任务变量字符串
	const char* GetTaskString(WORD wTaskId, BYTE btNum);
	//发送任务消息
	VOID SendTaskInfo();
	//更新任务数据到DB
	VOID UpdateTaskToDB();

	BOOL InSafeArea();
	BOOL SetPetBagSize(UINT nSize);
	BOOL GetItemFromPetBag(DWORD dwMakeIndex);
	BOOL PutItemToPetBag(DWORD dwMakeIndex);
	//宠物信息 类型Type=0收回,1普通宠物比如豹子类,2表示骑乘类,3其他宠物
	VOID SendOutPetInfo(ITEM* pItem, BYTE Type);
	VOID PetEatItem(DWORD nPetItemIdx, DWORD nItemIdx);
	VOID OnChangeMap(CLogicMap* pFromMap, UINT nFromX, UINT nFromY, CLogicMap* pToMap, UINT nToX, UINT nToY);
	// 获取天玄套的召唤宝宝等级
	DWORD GetTianXuanCallLevel(BYTE btLevel);
	// 获取天玄套的毒伤害增加值
	DWORD GetTianXuannDamage();
	// 幸运计算
	int CalcLucky();
	VOID PetsFlyto(CLogicMap* pToMap, UINT nToX, UINT nToY, BOOL IsBlock = TRUE);
	VOID SendOpenGameTimeInfo();
	int GetGameTIme()const { return m_Humandesc.dbinfo.nGameTime; }
	VOID AddGameTime(int nGameTime) { m_Humandesc.dbinfo.nGameTime += nGameTime; }
	VOID DecGameTime(int nGameTime)
	{ 
		if (m_Humandesc.dbinfo.nGameTime >= nGameTime)
			m_Humandesc.dbinfo.nGameTime -= nGameTime;
		else
			m_Humandesc.dbinfo.nGameTime = 0;
	}
	VOID SetGameTime(int nGameTime) { m_Humandesc.dbinfo.nGameTime = nGameTime; }
	VOID SetPersonCode(WORD wPersonCode){ m_Humandesc.dbinfo.wPersonCode = wPersonCode; }
	VOID SetPersonSign(const char* pszSign)
	{
		if (pszSign != nullptr)
		{
			strncpy(m_Humandesc.dbinfo.szPersonSign, pszSign, 19);
			m_Humandesc.dbinfo.szPersonSign[19] = '\0';
		}
	}
	VOID SetTempRank(const char* pszTempRank)
	{
		if (pszTempRank != nullptr)
		{
			strncpy(m_Humandesc.dbinfo.szTempRank, pszTempRank, 59);
			m_Humandesc.dbinfo.szTempRank[59] = '\0';
		}
	}
	//发送珍宝经验 ZhenBaoExp经验, ZhenBaoExpMax最大经验, ZhenBaoStar星星数
	VOID SendZhenBao(DWORD dwZhenBaoExp, DWORD dwZhenBaoExpMax = -1, DWORD dwZhenBaoStar = -1);
	//发送精力值
	VOID SendJingLiZhi(DWORD wStamina);
	//判断是否积满元气
	BOOL IsYuanQiFull()const { return m_bYuanQi; }
	//减少元气值
	VOID DecYuanqi(WORD nValue) 
	{ 
		m_wYuanQi -= nValue; 
		if (m_wYuanQi <= 0)
		{
			m_wYuanQi = 0;
			m_bYuanQi = FALSE;
		}
	}
	// 检查是否穿戴了指定物品
	BOOL CheckItemInfo(int pos, BYTE stdMode, BYTE btShape) { return m_Equipments.CheckItemInfo(pos, stdMode, btShape); }
public: //成就相关函数
	//初始化成就数据
	VOID InitAchievement(int nCount);
	//调整成就组进度值
	BOOL ChangeAchieveGroupExp(BYTE btGroupId, BYTE btType, DWORD btRecentCount);
	//调整指定成就ID进度值
	BOOL ChangeAchieveExp(WORD wId, BYTE btType, DWORD btRecentCount);
	//调整指定成就ID状态
	BOOL SetAchieveState(WORD wId, BYTE btStatu);
	//调整指定成就ID完成时间
	BOOL SetAchieveTime(WORD wId, DWORD dwTime);
	//发送更新指定成就相关信息
	BOOL SendGotAchieve(WORD wId);
	//调整玩家成就点
	BOOL ChangeAchievePoint(BYTE btType, DWORD dwExp);
	//组包玩家的成就数据
	VOID PacketAchieve(xPacket& packet, BYTE btType, int nAchieveCount);
	//获取玩家当前的成就进度
	DWORD GetAchieveExp() const { return m_Achievement.dwExp; }
	//获取玩家当前的成就等级
	BYTE GetAchieveLevel() const { return m_Achievement.btLevel; }
	//获取指定成就的进度值
	DWORD GetAchieveExpById(WORD wId) const;
	//获取指定成就的状态值
	BYTE GetAchieveStateById(WORD wId) const;
	//获取指定成就的完成时间
	DWORD GetAchieveCompleteTimeById(WORD wId) const;
	// 检查成就等级是否升级
	VOID CheckAchieveLevelUp();
protected:
	VOID SendClientfunction();
	DWORD m_dwForgePoint;
	mutable DWORD m_dwParams[4];

	CScriptTargetForPlayer m_ScriptTarget;
	VOID GetPrivateShopView(PRIVATESHOPHEADER& header)const;
	CHumanPlayer* m_curPrivateShopView;

	AbilityShellRef m_xAbilityShellRef;
	//char m_szCurrentNpcPage[128];

	CServerTimer m_tmrGameTime; // 时长区-计时

	CSystemTime m_LoginTime;
	char m_szTempScriptVarValue[256];

	DWORD m_dwPkValue;
	CServerTimer m_tmrPkTimer;
	CServerTimer m_tmrJustPk;
	BOOL m_bJustPk;
	VOID CheckPk(CAliveObject* pTarget);

	BOOL m_bRefuseAddFriend;

	CHumanPlayer* m_pAddToGuildRequester;
	DWORD m_dwAddToGuildRequesterInstanceKey;
	CServerTimer m_AddToGuildTimer;
	CGuildEx* m_pGuild;
	char m_szGuildTitle[64];
	int	 m_iGuildTitleLevel;
	//被动技能重新计算增加属性
	void RecalcHitSpeed();

	BOOL MagMakeDefenceArea(int x, int y, int nRange, int nSec, int nState);

	CItemBox m_ItemBank; // 仓库
	CItemBox m_ItemPetBag; // 宠物背包

	BOOL m_bChatChannelDisabled[CCH_MAX];
	CServerTimer m_ChatChannelTimer[CCH_MAX];
	e_chatchannel m_ChatChannel;
	char m_szCurWisperTarget[32];

	INT	m_iHuoli;

	BOOL m_bEnterMap;//是否进入地图
	BOOL m_bFirstLogin;//是否首次登录
	CClientObj* m_pClientObj;
	CREATEHUMANDESC	m_Humandesc; // 玩家保存的数据
	HUMANDATADESC* m_pHumanDataDesc; // 玩家配置的数据
	CItemBox m_ItemBox;
	CEquipment m_Equipments;//穿戴物品
	CExchangeObj* m_pExchangeObj;//交易对象
	CGroupObject* m_pGroupObject;//组队对象
	USERMAGIC* m_pMagic;

	BOOL m_fMagicLoaded;
	CServerTimer m_DBTimer; // 保存数据检查
	CServerTimer m_StaminaTimer; // 精力值检查

	CMonsterEx* m_pPets[5]; // 宠物对象列表
	CMonsterEx* m_pPet;//豹子对象
	DWORD m_dwPetKey;
	WORD m_wPetSkill;
	int	m_iPetCount;
	DWORD m_dwStartPointIndex;
	BOOL m_bFirstEnterMap;
	char m_szCurrentTitle[200];
	int	m_iCurrentTitleIndex;

	S_PARAM m_sParam[10]; // 个人变量
	V_PARAM m_vParam[10]; // 个人变量

	USERMAGIC* m_pAutoMagic[8];
	int	m_iAutoMagicCount;
	USERMAGIC* m_pExpMagic;
	USERMAGIC* m_pTimeOutDeActiveMagic;
	FLOAT m_fExpFactor;
	CMonsterEx* m_pHorse; //骑乘类对象
	mutable char petname[20];//豹子的名字
	BOOL ISzhaohuan;//是否召唤出来
	DWORD m_baozhiID;
	int ResMag_Count = 0;//魔法盾抵抗次数
	int NoDamage = 0;//魔法盾抵抗百分比
	int JingganNoDamage = 0;//金刚护体免伤
	BOOL m_bRideHorse;
	CServerTimer m_HorseTimer;
	CAliveObject* m_pSeizedObject;
	int	m_iSeizedTimes;
	CServerTimer m_PkPointTimer;

	int	m_iPrivateShopItemCount;
	PrivateShopItemCache m_PrivateShopCache[10];
	char m_szPrivateShopName[64];

	S_CHARNAME m_sWife; // 妻子
	S_CHARNAME m_sMaster; // 师傅
	S_CHARNAME m_sStudents[3]; // 3个徒弟
	S_CHARNAME m_sFriends[32]; // 32个好友

	DWORD m_dwSpecialEquipmentFunctionFlags[SEF_MAX];
	DWORD m_dwMineCounter;
	CServerTimer m_tmrMine;
	CServerTimer m_tmrSpecialAttackSkill;
	CServerTimer m_tmrRelive;
	ITEM m_UpgradeItem;
	BOOL m_bHorseRest; // 马是否休息

	ITEM* m_pUsingItem;
	ITEM* m_pPackItem; // 使用物品时, 与物品同时的物品
	ITEM* m_pPutItem;
	UINT m_nCutMonsterId;
	UINT m_nCutTimes;
	WORD m_wPrivateShopStyle;
	WORD m_wPrivateShopFlags;
	DWORD m_wPrivateShopSign;
	BYTE m_btChatColor;

	FenghaoInfo m_FenghaoInfo; // 玩家时长封号信息
	CServerTimer m_tmrFenghaoTime; // 时长区-封号计时
	AchievementData m_Achievement;  // 成就数据

	TASKINFO m_TaskInfo;
	int HushenBuffdamage = 0;//这个用来存储护身或者金刚受到的伤害
private:
	// 任务哈希表, 用于快速查找任务ID对应的索引
	std::unordered_map<WORD, int> m_TaskIdToIndexMap;
	// 重建任务哈希表, 从指定起始索引开始
	void RebuildTaskIdIndexMap(int startIndex = 0);
	// 解析任务系统的 @PS0 和 @PI0 这类动态变量（使用玩家特定参数）
	VOID ParseTaskParams(const char* pszText, char* pszOutBuffer, int iOutBufferSize, TASKNODE* pTaskNode);
private:
	int m_nMaterialBagPos; // CheckMaterial时, 在背包中找到的物品位置
	int m_nRecalcHit; // 在使用 RecalcHitSpeed 函数计算被动技能增加命中值多少, 用于计算增加和减少
	int m_nRecalcSpeed; // 在使用 RecalcHitSpeed 函数计算被动技能增加躲避值多少, 用于计算增加和减少
	BOOL boPoison; // 道士使用施毒、诅咒的类型切换
	DWORD m_dwZhenBaoExpMax;
	DWORD m_dwZhenBaoStar;
	WORD m_wYuanQi;		//当前元气值
	BOOL m_bYuanQi;		//是否触发元气
	WORD m_wStamina;	//精力值
	WORD m_wMaxStamina;	//最大精力值
};