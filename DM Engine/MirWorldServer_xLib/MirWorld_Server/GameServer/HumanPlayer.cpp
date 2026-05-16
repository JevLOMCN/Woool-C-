#include "StdAfx.h"
#include "humanplayer.h"
#include "SpecialEquipmentManager.h"
#include "StringListManager.h"
#include "gsclientobj.h"
#include "logicmap.h"
#include "server.h"
#include "downitemmgr.h"
#include "equipment.h"
#include "itemmanager.h"
#include "gameworld.h"
#include "npcmanager.h"
#include "scriptnpc.h"
#include "math.h"
#include "monsterex.h"
#include "monstermanagerex.h"
#include "humanplayermgr.h"
#include "groupobjectmgr.h"
#include "groupobject.h"
#include "titlemanager.h"
#include "bundlemanager.h"
#include "logicmapmgr.h"
#include "FireBurnEvent.h"
#include "guildmanagerex.h"
#include "guildex.h"
#include "sandcity.h"
#include "topmanager.h"
#include "systemscript.h"
#include "scriptvariable.h"
#include "ScriptDef.h"
#include "BossTJ.h"
#include "FengHaoGrowManager.h"

extern DWORD g_dwActionDelay[AT_MAX];
CHumanPlayer::CHumanPlayer(void) :m_pClientObj(nullptr), m_Equipments(this), m_ScriptTarget(this)
{
	m_ItemBox.Create(BIGBAG_SLOT); // 玩家背包大小
	m_ItemBank.Create(100); // 玩家仓库大小
	m_ItemPetBag.Create(10); // 豹子背包大小
	m_iHuoli = 0;
	m_nRecalcHit = 0;
	m_nRecalcSpeed = 0;
	m_pMagic = nullptr;
	m_pHorse = 0;
	m_bRideHorse = FALSE;
	ISzhaohuan = FALSE;
	m_bEnterMap = FALSE;
	m_bFirstLogin = FALSE;
	memset(petname, 0, sizeof(petname));
	m_nMaterialBagPos = -1;
	boPoison = FALSE;
	m_xQProcess.create(CGameWorld::GetInstance()->GetPlayerQueue());
	Clean();
}

CHumanPlayer::~CHumanPlayer(void)
{
}

VOID CHumanPlayer::Clean()
{
	memset(&m_TaskInfo, 0, sizeof(m_TaskInfo));
	memset(&m_xAbilityShellRef, 0, sizeof(m_xAbilityShellRef));
	ClearParam();
	this->m_ScriptTarget.Clean();
	USERMAGIC* p = m_pMagic, * pNext;
	while (p)
	{
		pNext = p->pNext;
		CGameWorld::GetInstance()->FreeUserMagic(p);
		p = pNext;
	}
	m_pMagic = nullptr;
	m_fMagicLoaded = FALSE;
	m_iPetCount = 0;	//	防止ALIVEOBJECT::CLEAN()中的SetPetActive中调用SETPETTARGET失败
	CAliveObject::Clean();
	m_DBTimer.Savetime();
	m_StaminaTimer.Savetime();
	m_tmrGameTime.Savetime();
	m_pExchangeObj = nullptr;
	m_pGroupObject = nullptr;
	m_dwStartPointIndex = 0;
	m_bFirstEnterMap = TRUE;
	m_szCurrentTitle[0] = 0;
	m_iCurrentTitleIndex = 0;
	memset(m_sParam, 0, sizeof(m_sParam));
	memset(m_vParam, 0, sizeof(m_vParam));
	m_ItemBox.Clean();
	m_ItemBank.Clean();
	m_ItemPetBag.Clean();
	memset(m_pAutoMagic, 0, sizeof(m_pAutoMagic));
	m_iAutoMagicCount = 0;
	m_fExpFactor = 1.0f;
	m_pExpMagic = nullptr;
	m_szCurWisperTarget[0] = 0;
	memset(m_bChatChannelDisabled, 0, sizeof(m_bChatChannelDisabled));
	SetGuild(nullptr);
	m_szGuildTitle[0] = 0;
	m_iGuildTitleLevel = 0;
	m_pAddToGuildRequester = nullptr;
	m_pHorse = nullptr;
	m_bRideHorse = FALSE;
	m_HorseTimer.Savetime();
	m_dwPkValue = 0;
	m_bJustPk = FALSE;
	m_pSeizedObject = nullptr;
	m_iSeizedTimes = 0;
	m_szGuildTitle[0] = 0;

	m_iPrivateShopItemCount = 0;
	SetCurPrivateShopView(nullptr);
	m_sMaster[0] = 0;
	m_sWife[0] = 0;
	memset(m_sStudents, 0, sizeof(m_sStudents));
	memset(m_sFriends, 0, sizeof(m_sFriends));
	memset(&m_FenghaoInfo, 0, sizeof(m_FenghaoInfo));
	InitAchievement(CTimeAchieve::GetInstance()->GetAchieveCount());

	m_bRefuseAddFriend = FALSE;
	m_pTimeOutDeActiveMagic = nullptr;

	memset(m_dwSpecialEquipmentFunctionFlags, 0, sizeof(m_dwSpecialEquipmentFunctionFlags));
	m_tmrMine.Savetime();
	m_dwMineCounter = 0;
	m_tmrSpecialAttackSkill.Savetime();
	m_tmrSpecialAttackSkill.SetTimeOut(0);
	memset(&m_UpgradeItem, 0, sizeof(m_UpgradeItem));

	m_bHorseRest = FALSE;
	m_pUsingItem = nullptr;
	m_pPackItem = nullptr;
	m_nCutMonsterId = 0;
	m_wPrivateShopStyle = 0;
	m_wPrivateShopFlags = 0;
	m_wPrivateShopSign = 0;
	m_btChatColor = 1;
	m_pPet = nullptr;
	m_dwPetKey = 0;
	m_wPetSkill = 0;
	m_dwZhenBaoExpMax = 0;
	m_dwZhenBaoStar = 0;
	m_wYuanQi = 0;
	m_bYuanQi = FALSE;
	m_wStamina = 0;
	m_wMaxStamina = 2000;
}

BOOL CHumanPlayer::Init(CREATEHUMANDESC& desc)
{
	//对刚出生的角色进行初始化
	m_pClientObj = desc.pClientObj;

	int tx, ty, tmapid;
	HUMANDATADESC* pHumanData = nullptr;
	FIRSTLOGIN_INFO* pFirstLoginInfo = nullptr;
	if (desc.dbinfo.wLevel == 0) //如果DBINFO的等级为0
	{
		m_bFirstLogin = TRUE;
		pFirstLoginInfo = CGameWorld::GetInstance()->GetFirstLoginInfo();
		pHumanData = CGameWorld::GetInstance()->GetHumanDataDesc(desc.dbinfo.btClass, pFirstLoginInfo->level); // 加载角色基础数据
	}
	else
		pHumanData = CGameWorld::GetInstance()->GetHumanDataDesc(desc.dbinfo.btClass, desc.dbinfo.wLevel); // 加载角色基础数据
	if (pHumanData == nullptr) return FALSE;
	m_pHumanDataDesc = pHumanData;
	START_POINT* pt = nullptr;
	if (m_bFirstLogin)
	{
		desc.dbinfo.wLevel = pFirstLoginInfo->level;
		BOOL bRet = CGameWorld::GetInstance()->GetBornPoint(desc.dbinfo.btClass, tmapid, tx, ty, desc.dbinfo.szStartPoint);
		assert(bRet);
		if (!bRet) return FALSE;
		desc.dbinfo.x = tx;
		desc.dbinfo.y = ty;
		desc.dbinfo.mapid = tmapid;
		desc.dbinfo.hp = pHumanData->wHp;
		desc.dbinfo.mp = pHumanData->wMp;
		desc.dbinfo.maxhp = pHumanData->wHp;
		desc.dbinfo.maxmp = pHumanData->wMp;
		desc.dbinfo.btAttackMode = HAM_PEACE;
		desc.dbinfo.dwGold = pFirstLoginInfo->dwGold;
		desc.dbinfo.dwYuanbao = pFirstLoginInfo->dwYuanBao;
		desc.dbinfo.dwCurExp = 0;
		desc.dbinfo.handweight = pHumanData->handweight;
		desc.dbinfo.weight = pHumanData->bagweight;
		desc.dbinfo.bodyweight = pHumanData->bodyweight;
		desc.dbinfo.maxac = pHumanData->maxac;
		desc.dbinfo.minac = pHumanData->minac;
		desc.dbinfo.maxmac = pHumanData->maxmac;
		desc.dbinfo.minmac = pHumanData->minmac;
		desc.dbinfo.maxmc = pHumanData->maxmc;
		desc.dbinfo.minmc = pHumanData->minmc;
		desc.dbinfo.maxsc = pHumanData->maxsc;
		desc.dbinfo.minsc = pHumanData->minsc;
		desc.dbinfo.maxdc = pHumanData->maxdc;
		desc.dbinfo.mindc = pHumanData->mindc;

		desc.dbinfo.btAttackMode = HAM_PEACE;
		desc.dbinfo.nGameTime = pFirstLoginInfo->nGameTime;
	}
	else
	{
		desc.dbinfo.maxhp = pHumanData->wHp;
		desc.dbinfo.maxmp = pHumanData->wMp;
		if (desc.dbinfo.hp > desc.dbinfo.maxhp)
			desc.dbinfo.hp = desc.dbinfo.maxhp;
		if (desc.dbinfo.mp > desc.dbinfo.maxmp)
			desc.dbinfo.mp = desc.dbinfo.maxmp;
		desc.dbinfo.handweight = pHumanData->handweight;
		desc.dbinfo.weight = pHumanData->bagweight;
		desc.dbinfo.bodyweight = pHumanData->bodyweight;
		desc.dbinfo.maxac = pHumanData->maxac;
		desc.dbinfo.minac = pHumanData->minac;
		desc.dbinfo.maxmac = pHumanData->maxmac;
		desc.dbinfo.minmac = pHumanData->minmac;
		desc.dbinfo.maxmc = pHumanData->maxmc;
		desc.dbinfo.minmc = pHumanData->minmc;
		desc.dbinfo.maxsc = pHumanData->maxsc;
		desc.dbinfo.minsc = pHumanData->minsc;
		desc.dbinfo.maxdc = pHumanData->maxdc;
		desc.dbinfo.mindc = pHumanData->mindc;
	}

	if (desc.dbinfo.hp == 0)
	{
		desc.dbinfo.hp = GetRangeRand(1, 10);
		BOOL bRet = CGameWorld::GetInstance()->GetStartPoint(desc.dbinfo.szStartPoint, tmapid, tx, ty);
		if (!bRet)
			bRet = CGameWorld::GetInstance()->GetBornPoint(desc.dbinfo.btClass, tmapid, tx, ty, desc.dbinfo.szStartPoint);
		if (!bRet) return FALSE;
		desc.dbinfo.x = tx;
		desc.dbinfo.y = ty;
		desc.dbinfo.mapid = tmapid;
	}

	START_POINT* p = CGameWorld::GetInstance()->GetStartPoint(desc.dbinfo.szStartPoint);
	if (p != nullptr)
		this->m_dwStartPointIndex = p->index;
	m_Humandesc = desc;
	setXY(desc.dbinfo.x, desc.dbinfo.y);
	SetMapId(desc.dbinfo.mapid);
	SetDirection((e_direction)desc.dbinfo.dir);
	int iBagCount = 0;
	if (desc.dbinfo.bBigBag)
		iBagCount = BIGBAG_SLOT;
	else
		iBagCount = SMALLBAG_SLOT;
	m_ItemBox.SetCountLimit(iBagCount);
	this->m_ItemBank.SetCountLimit(CGameWorld::GetInstance()->GetVar(EVI_STOREAGESIZE));
	m_Equipments.Clean();
	m_iHuoli = 0;
	m_nVisibleObjectFlag = 0;
	AddVisibleObjectType(OBJ_NPC); // 设置是否可见的
	AddVisibleObjectType(OBJ_PLAYER);
	AddVisibleObjectType(OBJ_MONSTER);
	AddVisibleObjectType(OBJ_DOWNITEM);
	AddVisibleObjectType(OBJ_VISIBLEEVENT);
	AddVisibleObjectType(OBJ_PET);

	SendMsg(GetId(), 0x9591, 256, 7, 3, "2, 0, 9, 0"); // 发送时长版本号
	SendClientfunction();
	SendMsg(0, 0x949, 1, 100, 0);
	SendMsg(0, 0x510, 0, 0, 0);
	SendMsg(0, 0x328, 1, 0, 0); // 服务器通知客户端是否使用动态加密算法, 以及动态加密数据的长度设置
	SendMsg(GetId(), 0x9a9, 0, 0, 0);//关闭万兽谱、羽翼
	SendMsg(GetId(), 0x9594, 0, iBagCount, 0);//38292 发送背包大小
	SendMsg(GetId(), 0x9593, 1, 0, desc.dbinfo.wPersonCode, (LPVOID)desc.dbinfo.szPersonSign);//38291 设置个性化签名
	SendMsg(GetId(), 0x9593, 2, 0, 0, (LPVOID)desc.dbinfo.szTempRank);//38291 设置临时称号
	Sendfirstdlg(CGameWorld::GetInstance()->GetNotice());//658 发送登录窗口提示
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(desc.dbinfo.mapid);
	if (pMap == nullptr) return FALSE;
	SendMsg(GetId(), SM_SETMAP, m_wX, m_wY, 0, (LPVOID)pMap->GetName());//人物在地图的位置
	SendMsg(0xf2d505d7, SM_READY, 0, 0, 0);//1106
	UpdateProp(); // 52人物信息
	RecalcHitSpeed(); // 计算命中、躲避
	UpdateSubProp(); // 752附加属性
	SendStatusChanged(); // 657发送状态改变
	SaySystemAttrib(CC_GREEN, "[%s攻击模式]", g_pAttackModeDesc[m_Humandesc.dbinfo.btAttackMode]);//攻击模式
	SaySystemAttrib(CC_GREEN, getstrings(SD_CHANGEATTACKMODE)); // 发送更改攻击模式, CTR-H
	SendTimeWeatherChanged(); // 发送天气时间
	CheckAndUpgradeTitle(); // 发送窗口标题
	SendGroupMode(); // 发送组队状态
	SendMoneyChanged(MT_YUANBAO); // 发送元宝数量
	if (m_Humandesc.dbinfo.szGuildName[0] != 0)
	{
		CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(m_Humandesc.dbinfo.szGuildName);
		if (pGuild)
			pGuild->MemberLogon(this);
		else
			PRINT(ERROR_RED, "%s 找不到自己所在的行会 %s, 请检查是否是数据转移的问题!\n", m_Humandesc.dbinfo.szName, m_Humandesc.dbinfo.szGuildName);
	}
	ChangeChatChannel(CCH_NORMAL);
	SaySystemAttrib( CC_GREEN, "更改频道 CTRL+S 查看频道信息 @ccinfo" );
	m_tmrPkTimer.Savetime();
	m_dwPkValue = m_Humandesc.dbinfo.dwFlag[1];
	UpdateViewName();
	SendFengHaoData();
	m_LoginTime = CSystemTime();
	return TRUE;
}

VOID CHumanPlayer::SendClientfunction()
{
	char szTempBuffer[8]{};
	xPacket packet(szTempBuffer, 7);
	DWORD nValue = 0x00;
	packet.push((LPVOID)&nValue, 4);
	nValue = 0x00;
	packet.push((LPVOID)&nValue, 4);
	WORD LoParam1{}, HiParam1{};
	LoParam1 += 1 << 6; // 新邮件
	//LoParam1 += 1 << 7; // 开启2.4内容, 包括友好度、结婚、结义
	LoParam1 += 1 << 10; // 2019版活跃度
	LoParam1 += 1 << 13; // 资源服务器重新连接
	BYTE LoParam2{}, HiParam2{};
	LoParam2 += 1 << 0; // 豹子摆摊功能
	LoParam2 += 1 << 2; // 新彩虹精灵
	LoParam2 += 1 << 4; // 快捷消费使用功能
	HiParam2 += 1 << 1; // 主线任务删除按钮
	HiParam2 += 1 << 2; // 开启IGW
	HiParam2 += 1 << 7; // 安全区穿人
	BYTE LoParam3{}, HiParam3{};
	LoParam3 += 1 << 1; // 元宝拍卖行和现金拍卖行是否合并
	LoParam3 += 1 << 3; // 打开宝石系统
	LoParam3 += 1 << 5; // 关闭彩虹网弹框
	HiParam3 += 1 << 6; // 开启自定义快捷键功能
	SendMsg(static_cast<DWORD>(MAKELONG(LoParam1, HiParam1)), 0x330, MAKEWORD(LoParam2, HiParam2), MAKEWORD(LoParam3, HiParam3),
		0, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CHumanPlayer::Sendfirstdlg(const char* pszString)
{
	char szBuffer[1024]{};
	xPacket packet(szBuffer, 1023);
	packet.push((LPVOID)&m_Humandesc.dbinfo.nGameTime, 4);
	packet.push((LPVOID)&m_Humandesc.dbinfo.wLevel, 2);
	packet.push(pszString);
	SendMsg(GetId(), SM_FIRSTDIALOG, 257, 1101, 1, (LPVOID)packet.getbuf(), packet.getsize());
	SendMsg(m_Humandesc.dbinfo.dwYuanbao, 0xe679, 0, 0, 0); // 发送元宝数量
}

VOID CHumanPlayer::SendOpenGameTimeInfo()
{
	char szBuffer[64]{};
	xPacket packet(szBuffer, 63);
	const char* s1C = "GameTimeMgr";
	packet.push(s1C);
	packet.push(1);
	int nValue = 0x01;
	packet.push((LPVOID)&nValue, 4);
	packet.push(1);
	nValue = 0x02;
	packet.push((LPVOID)&nValue, 4);
	packet.push((LPVOID)&m_Humandesc.dbinfo.nGameTime, 4);
	packet.push(16);
	nValue = 0x02;
	packet.push((LPVOID)&nValue, 4);
	packet.push((LPVOID)&m_Humandesc.dbinfo.nGameTime, 4);
	packet.push(12);
	SendMsg(GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

#ifdef _DEBUG
//static char szDecodeBuffer[65536];
#endif
VOID CHumanPlayer::OnAroundMsg(CMapObject* pSender, const char* pszCodedMsg, int size)
{
	if (m_pClientObj == nullptr) return;
	assert(m_pClientObj != nullptr);
#ifdef _DEBUG // 打印发出去的日志
	//if (pszCodedMsg && size > 0)
	//{
	//	if (pszCodedMsg[0] == '#')
	//	{
	//		char* pStart = (char*)pszCodedMsg + 1;
	//		if (*pStart >= '0' && *pStart <= '9') pStart++;
	//		int length = _UnGameCode(pStart, (BYTE*)szDecodeBuffer);
	//		CServer::GetInstance()->OnUnknownMsg((PMIRMSG)szDecodeBuffer, length, 2);
	//	}
	//}
#endif
	m_pClientObj->PostMsg(pszCodedMsg, size);
}

VOID CHumanPlayer::OnLeaveMap(CLogicMap* pMap)
{
	if (pMap->IsFlagSeted(MF_SABUKPALACE) && CSandCity::GetInstance()->IsWarStarted())
		CSandCity::GetInstance()->OnLeavePalace(this);
	CAliveObject::OnLeaveMap(pMap);
	m_bEnterMap = FALSE;
}

VOID CHumanPlayer::OnEnterMap(CLogicMap* pMap)
{
	SendMsg(GetId(), SM_SETMAP, m_wX, m_wY, 0, (LPVOID)pMap->GetName());//人物在地图的位置
	DWORD dwParam[4] = { GetFeather(), GetStatus(), GetGroupObject() != nullptr, 0 };
	SendMsg(GetId(), SM_SETPLAYER, m_wX, m_wY, GetSex() << 8 | GetDirection(), (LPVOID)dwParam, sizeof(DWORD) * 4);//人物外观更新
	CAliveObject::OnEnterMap(pMap);
	SendTimeWeatherChanged();
	UpdateViewName();
	if (GetStatus() > 0)
		SendStatusChanged();
	if (IsSystemFlagSeted(SF_COMMUNITYLOADED))
		UpdateCommunityInfoToClient(FALSE);
	BOOL bSendSpecialStatus = FALSE;
	if (IsSystemFlagSeted(SF_GODBLESS))
	{
		bSendSpecialStatus = TRUE;
		if (!IsSystemFlagSeted(SF_WINDSHIELD) && !IsSystemFlagSeted(SF_STRONGSHIELD))
			SendMsg(GetId(), 0x532c, 0, 0, 0);
	}
	if (IsSystemFlagSeted(SF_WINDSHIELD) || IsSystemFlagSeted(SF_STRONGSHIELD))
		bSendSpecialStatus = TRUE;
	if (bSendSpecialStatus)
		SendSpecialStatusChanged(FALSE);
	//	附身
	//if( IsSystemFlagSeted(SF_GODBLESS) )
	//{
	//	SendMsg( GetId(), 0x532c, 0, 0, 0 );
	//	SendMsg( GetId(), 0x532c, 4, m_SystemFlag.GetTimeOut(SF_GODBLESS)/1000, 0 );
	//}
	//	如果地图是沙城宫殿, 并且攻城战已经开始, 那么就调用进入宫殿的事件
	if (pMap->IsFlagSeted(MF_SABUKPALACE) && CSandCity::GetInstance()->IsWarStarted())
		CSandCity::GetInstance()->OnEnterPalace(this);
	RefreshSpecialEquipment();
	m_bEnterMap = TRUE;
}

DWORD CHumanPlayer::GetFeather()
{
	BYTE btShape = m_Equipments[_U_WEAPON].baseitem.btShape;
	if (m_Equipments[_U_WEAPON].baseitem.btStdMode == 5 && btShape == 40) // 马鞭外观值, 重写
		btShape--;
	if (m_Equipments[_U_WEAPON].baseitem.btStdMode == 6 && btShape == 19) // 鹤嘴锄外观值, 重写
		btShape = btShape + 6;
	//四个字节分别是：bGender | (bWeapon << 8) | (bHair << 16) | (bDress << 24)
	return MAKEFEATHER(
		(m_Equipments[_U_DRESS].dwMakeIndex != 0 ? ((m_Equipments[_U_DRESS].baseitem.btShape & 0xf) << 4 | (m_Equipments[_U_DRESS].baseitem.wFeature & 0xf)) : 0), // 衣服外观
		m_Humandesc.dbinfo.btHair, // 头发外观
		(m_Equipments[_U_WEAPON].dwMakeIndex != 0 ? btShape : 0), // 武器外观
		m_bRideHorse ? ((m_pHorse->GetFeather() & 0xff0000) >> 16) + 0x40 : 0
	);
}

DWORD CHumanPlayer::GetHealth()
{
	return 0x00640064;
}

DWORD CHumanPlayer::GetStatus()
{
	return m_Status.GetFlag();
}

BYTE CHumanPlayer::GetShape()
{
	BYTE temp = m_Equipments[_U_DRESS].baseitem.btShape > 0 ? m_Equipments[_U_DRESS].baseitem.btShape : 0;
	return temp;
}

BOOL CHumanPlayer::GetDBInfo(CHARDBINFO& info)
{
	info = m_Humandesc.dbinfo;
	info.x = m_wX;
	info.y = m_wY;
	info.mapid = m_Mapid;
	info.dir = m_Direction;
	return TRUE;
}

BOOL CHumanPlayer::AddBagItem(ITEM& item, BOOL bSilence, BOOL bUpdateDB, BOOL bUpdateWeight)
{
	if (m_ItemBox.AddItem(item))
	{
		if (!bSilence && m_pClientObj != nullptr)
			m_pClientObj->SendAddItem(item);
		if (bUpdateWeight)
			SendWeightChanged();
		if (bUpdateDB)
			CItemManager::GetInstance()->UpdateItemOwner(GetDBId(), item.dwMakeIndex, IDF_BAG, 0);
		return TRUE;
	}
	return FALSE;
}

//子类实现虚函数, 豹子拾取物品到主人背包
BOOL CHumanPlayer::PetAddBagItem(ITEM& item, BOOL bSilence, BOOL bUpdateDB, BOOL bUpdateWeight)
{
	if (m_ItemBox.AddItem(item))
	{
		if (!bSilence && m_pClientObj != nullptr)
			m_pClientObj->SendAddItem(item);
		if (bUpdateWeight)
			SendWeightChanged();
		if (bUpdateDB)
			CItemManager::GetInstance()->UpdateItemOwner(GetDBId(), item.dwMakeIndex, IDF_BAG, 0);
		return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::DropItem(ITEM& item)
{
	if (item.IsBind())
	{
		SaySystem("%s 是绑定物品, 不能丢弃!", item.baseitem.szName);
		return FALSE;
	}
	POINT pt;
	BOOL bFlag = FALSE;
	int count = m_pMap->GetDropItemPoint(getX(), getY(), &pt, 1);
	if (count > 0)
	{
		bFlag = CDownItemMgr::GetInstance()->DropItem(m_pMap, item, pt.x, pt.y, FALSE, this);
		if (bFlag)
			OnDropItem(item, pt.x, pt.y);
	}
	return bFlag;
}

BOOL CHumanPlayer::PickupItem()
{
	CDownItemObject* pObj = (CDownItemObject*)m_pMap->FindObject(m_wX, m_wY, OBJ_DOWNITEM);
	if (pObj == nullptr)return FALSE;
	if (!pObj->UpdateValid()) return FALSE;
	if (!pObj->IsGold())
	{
		if (m_ItemBox.GetFree() == 0)
		{
			SaySystem(getstrings(SD_PICKUP_BAGFULL));
			return FALSE;
		}
		if (!CanBearItem(pObj->GetItem()))
		{
			SaySystem(getstrings(SD_PICKUP_WEIGHTFULL));
			return FALSE;
		}
	}
	DWORD dwOwnerId = pObj->GetOwnerId();
	if (dwOwnerId == 0 || dwOwnerId == this->GetDBId() || (m_pGroupObject != nullptr && m_pGroupObject->IsMemberDBId(dwOwnerId)))
		return CDownItemMgr::GetInstance()->PickupItem(m_pMap, pObj, this);
	SaySystem(getstrings(SD_PICKUP_NOTYOURS));
	return FALSE;
}

BOOL CHumanPlayer::DropBagItem(DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	if (pItem == nullptr)return FALSE;
	if (CItemManager::GetInstance()->ItemLimited(*pItem, IL_NODROP))
	{
		SaySystem(getstrings(SD_ITEMLIMIT_NODROP));
		return FALSE;
	}
	if (!DropItem(*pItem))return FALSE;
	m_ItemBox.RemoveItem(dwMakeIndex);
	return TRUE;
}

BYTE CHumanPlayer::GetShape(DWORD dwMakeIndex)
{
	BYTE S = 0;
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	if (pItem != nullptr)
		S = pItem->baseitem.btShape;
	return S;
}

BOOL CHumanPlayer::EquipItem(int pos, DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	if (pItem == nullptr) return FALSE;
	ITEM itemout;
	if (m_Equipments.EquipItem(pos, *pItem, itemout))
	{
		CItemManager::GetInstance()->UpdateItemPos(dwMakeIndex, IDF_EQUIPMENT, (WORD)pos);
		m_ItemBox.RemoveItem(dwMakeIndex);
		if (itemout.dwMakeIndex != 0)
		{
			AddBagItem(itemout);
			CItemManager::GetInstance()->UpdateItemPos(itemout.dwMakeIndex, IDF_BAG, 0);
		}
		return TRUE;
	}
	SaySystem(m_Equipments.GetErrorMsg());
	return FALSE;
}

BOOL CHumanPlayer::EquipItem(int pos, ITEM& item, BOOL bForced, BOOL bNoticePlayer)
{
	ITEM itemout;
	if (m_Equipments.EquipItem(pos, item, itemout, bForced, bNoticePlayer))
	{
		item = itemout;
		return TRUE;
	}
	SaySystem(m_Equipments.GetErrorMsg());
	return FALSE;
}

BOOL CHumanPlayer::UnEquipItem(int pos, DWORD dwMakeIndex)
{
	if (dwMakeIndex == 0 || m_Equipments[pos].dwMakeIndex != dwMakeIndex)
		return FALSE;
	BOOL bAddToBag = TRUE;
	if (m_Equipments[pos].wCurDura == 0)
		bAddToBag = FALSE;
	else if (CItemManager::GetInstance()->ItemLimited(m_Equipments[pos], IL_NOTAKEOFF))
	{
		SaySystem(getstrings(SD_ITEMLIMIT_NOTAKEOFF));
		return FALSE;
	}
	ITEM item;
	if (bAddToBag && m_ItemBox.GetFree() < 1)
		return FALSE;
	if (!m_Equipments.UnEquipItem(pos, item))
		return FALSE;
	if (bAddToBag)
		AddBagItem(item, FALSE, TRUE);
	else
	{
		CItemManager::GetInstance()->DeleteItem(dwMakeIndex);
		this->SendTakeBagItem(&item);
	}
	return TRUE;
}

BOOL CHumanPlayer::AddGroupMember(CHumanPlayer* pObject)
{
	return TRUE;
}

BOOL CHumanPlayer::RemoveGroupMember(CHumanPlayer* pObject)
{
	return TRUE;
}

int CHumanPlayer::GetPropValue(PROP_INDEX index)
{
	int	sRet = 0;
	switch (index)
	{
	case PI_MINAC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.minac; break;
	case PI_MAXAC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxac + (this->m_xAbilityShellRef.pShell == nullptr ? 0 : this->m_xAbilityShellRef.pShell->wAddAc); break;

	case PI_MINMAC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.minmac; break;
	case PI_MAXMAC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxmac + (this->m_xAbilityShellRef.pShell == nullptr ? 0 : this->m_xAbilityShellRef.pShell->wAddMac); break;

	case PI_MINDC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.mindc; break;
	case PI_MAXDC:	sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxdc; break;

	case PI_MINMC:
	{
		sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.minmc;
		sRet += CAliveObject::GetPropValue(index);
		if (!this->IsSpecialEquipmentFunctionOn(SEF_HUANMOALL) && !this->IsSpecialEquipmentFunctionOn(SEF_HUANMOHALF))
		{
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO01))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO01, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO02))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO02, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO03))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO03, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO04))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO04, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO05))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO05, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO06))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO06, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO07))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO07, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO08))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO08, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO09))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO09, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_HUANMO10))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_HUANMO10, 1);
		}
		if (!this->IsSpecialEquipmentFunctionOn(SEF_MOXUEALL) && !this->IsSpecialEquipmentFunctionOn(SEF_MOXUEHALF))
		{
			if (this->IsSpecialEquipmentFunctionOn(SEF_MOXUE01))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_MOXUE01, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_MOXUE02))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_MOXUE02, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_MOXUE03))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_MOXUE03, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_MOXUE04))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_MOXUE04, 1);
			if (this->IsSpecialEquipmentFunctionOn(SEF_MOXUE05))
				sRet -= CSpecialEquipmentManager::GetInstance()->GetFunctionParam(SEF_MOXUE05, 1);
		}
		if (sRet <= 0)sRet = 0;
		return sRet;
	}
	break;
	case PI_MAXMC:			sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxmc; break;

	case PI_MINSC:			sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.minsc; break;
	case PI_MAXSC:			sRet = m_Equipments.GetProp(index) + m_Humandesc.dbinfo.maxsc; break;

	case PI_HITRATE:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->hitrate); break;
	case PI_ESCAPE:			sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->escape); break;
	case PI_MAGESCAPE:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->magescape); break;
	case PI_MAGHITRATE:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->magicnicety); break;
	case PI_POISONESCAPE:	sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->poisonescape); break;
	case PI_POISONHITRATE:	sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->poisonnicety); break;

	case PI_ATTACKSPEED:	sRet = m_Equipments.GetProp(index); break;
	case PI_LUCKY:			sRet = m_Equipments.GetProp(index); break;
	case PI_DAWN:			sRet = m_Equipments.GetProp(index); break;

	case PI_HPRECOVER:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->hprecover); break;
	case PI_MPRECOVER:		sRet = m_Equipments.GetProp(index) + (m_pHumanDataDesc == nullptr ? 0 : m_pHumanDataDesc->magicrecover); break;
	case PI_POISONRECOVER:	sRet = m_Equipments.GetProp(index); break;

	case PI_HARD:			sRet = m_Equipments.GetProp(index); break;
	case PI_HOLLY:			sRet = m_Equipments.GetProp(index); break;

	case PI_LEVEL:			sRet = m_Humandesc.dbinfo.wLevel; break;

	case PI_CURBAGWEIGHT:	sRet = m_ItemBox.CalcWeight(); break;
	case PI_MAXBAGWEIGHT:	sRet = IsSpecialEquipmentFunctionOn(SEF_BAGOVERLOAD) ? m_Humandesc.dbinfo.weight * 2 : m_Humandesc.dbinfo.weight; break;

	case PI_CURHANDWEIGHT:	sRet = m_Equipments[_U_WEAPON].baseitem.btWeight; break;
	case PI_MAXHANDWEIGHT:	sRet = m_Humandesc.dbinfo.handweight; break;

	case PI_CURBODYWEIGHT:	sRet = m_Equipments.CalcEquipmentsWeight(-1); break;
	case PI_MAXBODYWEIGHT:	sRet = IsSpecialEquipmentFunctionOn(SEF_OVERLOAD) ? m_Humandesc.dbinfo.bodyweight * 2 : m_Humandesc.dbinfo.bodyweight; break;

	case PI_CURHP:			sRet = m_Humandesc.dbinfo.hp; break;
	case PI_CURMP:			sRet = m_Humandesc.dbinfo.mp; break;
	case PI_MAXHP:			sRet = m_Humandesc.dbinfo.maxhp + (this->m_xAbilityShellRef.pShell == nullptr ? 0 : this->m_xAbilityShellRef.pShell->wAddHp); break;
	case PI_MAXMP:			sRet = m_Humandesc.dbinfo.maxmp + (this->m_xAbilityShellRef.pShell == nullptr ? 0 : this->m_xAbilityShellRef.pShell->wAddMp); break;
	case PI_HPPERCENT:		sRet = ROUND(m_Humandesc.dbinfo.hp / GetPropValue(PI_MAXHP));break;
	case PI_EXP:			sRet = m_Humandesc.dbinfo.dwCurExp; break;
	case PI_LEVELUPEXP:		sRet = m_pHumanDataDesc->dwLevelupExp;break;
	case PI_EXPPERCENT:		sRet = ROUND(m_Humandesc.dbinfo.dwCurExp / m_pHumanDataDesc->dwLevelupExp);break;
	}
	return (sRet + CAliveObject::GetPropValue(index));
}

VOID CHumanPlayer::DecPropValue(PROP_INDEX index, int value)
{
	switch (index)
	{
	case PI_CURHP:
	{
		if (IsSpecialEquipmentFunctionOn(SEF_PROTECT))
		{
			WORD wDecMp = static_cast<WORD>(value);
			WORD wCurMp = static_cast<WORD>(GetPropValue(PI_CURMP));
			DecPropValue(PI_CURMP, wDecMp);
			if (wCurMp < wDecMp)
				value = wDecMp - wCurMp;
			else
				return;
		}

		WORD wDecHp = (WORD)value;
		if (wDecHp > m_Humandesc.dbinfo.hp)
			m_Humandesc.dbinfo.hp = 0;
		else
			m_Humandesc.dbinfo.hp -= wDecHp;
		if (IsStatusSet(SI_GREENPOISON))//绿毒状态
		{
			if (m_Humandesc.dbinfo.hp <= 0) ToDeath(GetSetter(SI_GREENPOISON)); // 如果血量被减到0了, 就判断死亡
		}
	}
	break;
	case PI_CURMP:
	{
		WORD wDecMp = (WORD)value;
		if (wDecMp > m_Humandesc.dbinfo.mp)
			m_Humandesc.dbinfo.mp = 0;
		else
			m_Humandesc.dbinfo.mp -= wDecMp;
	}
	break;
	}
}

void CHumanPlayer::AddPropValue(PROP_INDEX index, int value)
{
	WORD wAddValue = static_cast<WORD>(value);
	switch (index)
	{
	case PI_CURHP:
	{
		WORD wMaxHp = static_cast<WORD>(GetPropValue(PI_MAXHP));
		if (wMaxHp - m_Humandesc.dbinfo.hp > wAddValue)
			m_Humandesc.dbinfo.hp += wAddValue;
		else
			m_Humandesc.dbinfo.hp = wMaxHp;
	}
	break;
	case PI_CURMP:
	{
		WORD wMaxMp = static_cast<WORD>(GetPropValue(PI_MAXMP));
		if (wMaxMp - m_Humandesc.dbinfo.mp > wAddValue)
			m_Humandesc.dbinfo.mp += wAddValue;
		else
			m_Humandesc.dbinfo.mp = wMaxMp;
	}
	break;
	}
}

VOID CHumanPlayer::UpdateProp()
{
	HUMANPROP prop;
	prop.wLevel = m_Humandesc.dbinfo.wLevel;
	prop.dwCurexp = m_Humandesc.dbinfo.dwCurExp;
	prop.dwMaxexp = m_pHumanDataDesc->dwLevelupExp;
	//生命值、魔法值
	int iCurHp = GetPropValue(PI_CURHP);
	int iMaxHp = GetPropValue(PI_MAXHP);
	int iCurMp = GetPropValue(PI_CURMP);
	int iMaxMp = GetPropValue(PI_MAXMP);
	prop.wCurHp = static_cast<WORD>(MIN(iCurHp, static_cast<int>(USHRT_MAX)));
	prop.wMaxHp = static_cast<WORD>(MIN(iMaxHp, static_cast<int>(USHRT_MAX)));
	prop.wCurMp = static_cast<WORD>(MIN(iCurMp, static_cast<int>(USHRT_MAX)));
	prop.wMaxMp = static_cast<WORD>(MIN(iMaxMp, static_cast<int>(USHRT_MAX)));
	prop.dwHP = iCurHp;
	prop.dwMaxHP = iMaxHp;
	prop.dwMP = iCurMp;
	prop.dwMaxMP = iMaxMp;
	//属性值
	int iMinAc = GetPropValue(PI_MINAC);
	int iMaxAc = GetPropValue(PI_MAXAC);
	int iMinMagicDef = GetPropValue(PI_MINMAC);
	int iMaxMagicDef = GetPropValue(PI_MAXMAC);
	int iMinAtk = GetPropValue(PI_MINDC);
	int iMaxAtk = GetPropValue(PI_MAXDC);
	int iMinMagAtk = GetPropValue(PI_MINMC);
	int iMaxMagAtk = GetPropValue(PI_MAXMC);
	int iMinSprAtk = GetPropValue(PI_MINSC);
	int iMaxSprAtk = GetPropValue(PI_MAXSC);
	prop.btMinDef = static_cast<WORD>(MIN(iMinAc, static_cast<int>(USHRT_MAX)));
	prop.btMaxDef = static_cast<WORD>(MIN(iMaxAc, static_cast<int>(USHRT_MAX)));
	prop.btMinMagicDef = static_cast<WORD>(MIN(iMinMagicDef, static_cast<int>(USHRT_MAX)));
	prop.btMaxMagicDef = static_cast<WORD>(MIN(iMaxMagicDef, static_cast<int>(USHRT_MAX)));
	prop.btMinAtk = static_cast<WORD>(MIN(iMinAtk, static_cast<int>(USHRT_MAX)));
	prop.btMaxAtk = static_cast<WORD>(MIN(iMaxAtk, static_cast<int>(USHRT_MAX)));
	prop.btMinMagAtk = static_cast<WORD>(MIN(iMinMagAtk, static_cast<int>(USHRT_MAX)));
	prop.btMaxMagAtk = static_cast<WORD>(MIN(iMaxMagAtk, static_cast<int>(USHRT_MAX)));
	prop.btMinSprAtk = static_cast<WORD>(MIN(iMinSprAtk, static_cast<int>(USHRT_MAX)));
	prop.btMaxSprAtk = static_cast<WORD>(MIN(iMaxSprAtk, static_cast<int>(USHRT_MAX)));

	prop.wBaseDC1 = static_cast<WORD>(MIN(iMinAtk, static_cast<int>(USHRT_MAX)));
	prop.wBaseDC2 = static_cast<WORD>(MIN(iMaxAtk, static_cast<int>(USHRT_MAX)));
	prop.wBaseMC1 = static_cast<WORD>(MIN(iMinMagAtk, static_cast<int>(USHRT_MAX)));
	prop.wBaseMC2 = static_cast<WORD>(MIN(iMaxMagAtk, static_cast<int>(USHRT_MAX)));
	prop.wBaseSC1 = static_cast<WORD>(MIN(iMinSprAtk, static_cast<int>(USHRT_MAX)));
	prop.wBaseSC2 = static_cast<WORD>(MIN(iMaxSprAtk, static_cast<int>(USHRT_MAX)));

	prop.dwExtendAC1 = iMinAc;
	prop.dwExtendAC2 = iMaxAc;
	prop.dwExtendMAC1 = iMinMagicDef;
	prop.dwExtendMAC2 = iMaxMagicDef;
	prop.dwExtendDC1 = iMinAtk;
	prop.dwExtendDC2 = iMaxAtk;
	prop.dwExtendMC1 = iMinMagAtk;
	prop.dwExtendMC2 = iMaxMagAtk;
	prop.dwExtendSC1 = iMinSprAtk;
	prop.dwExtendSC2 = iMaxSprAtk;

	prop.wCurBagWeight = GetPropValue(PI_CURBAGWEIGHT);
	prop.wMaxBagWeight = GetPropValue(PI_MAXBAGWEIGHT);
	prop.btCurBodyWeight = GetPropValue(PI_CURBODYWEIGHT);
	prop.btCurHandWeight = GetPropValue(PI_CURHANDWEIGHT);
	prop.btMaxBodyWeight = GetPropValue(PI_MAXBODYWEIGHT);
	prop.btMaxHandWeight = GetPropValue(PI_MAXHANDWEIGHT);

	prop.wStamina = m_wStamina;
	prop.wMaxStamina = m_wMaxStamina;
	prop.wPersonalCode = m_Humandesc.dbinfo.wPersonCode;

	WORD w1 = m_Humandesc.dbinfo.btClass; // 职业
	SendMsg(m_Humandesc.dbinfo.dwGold, SM_UPDATEPROP, w1, 0, 0, &prop, sizeof(prop)); // 发送人物信息
}

VOID CHumanPlayer::UpdateSubProp()
{
	DWORD dwArr = ((m_Humandesc.dbinfo.dwFlag[0] & 0xffff) << 8) | (GetPropValue(PI_MAGESCAPE) & 0xff); // 声望 | 魔法躲避/10
	WORD w1 = GetPropValue(PI_ESCAPE) << 8 | GetPropValue(PI_HITRATE); // 躲避 | 命中
	WORD w2 = GetPropValue(PI_POISONESCAPE) << 8 | GetPropValue(PI_POISONRECOVER); // 中毒躲避/10 | 中毒恢复
	WORD w3 = GetPropValue(PI_HPRECOVER) << 8 | GetPropValue(PI_MPRECOVER); // 生命恢复 | 魔法恢复
	HUMANSUPROP suprop;
	suprop.wHuoli = m_iHuoli;
	suprop.wHuoliMax = 600;
	suprop.bColor = 1;
	suprop.dwForgePoint = m_Humandesc.dbinfo.dwForgePoint;
	suprop.bLucky = (BYTE)GetPropValue(PI_LUCKY);
	suprop.bDawn = (BYTE)GetPropValue(PI_DAWN);
	suprop.btMagicNicety = GetPropValue(PI_MAGHITRATE);
	suprop.btPoisonNicety = GetPropValue(PI_POISONHITRATE);

	suprop.dwSpeedPoint = GetPropValue(PI_ESCAPE);
	suprop.dwUnKnow = m_Humandesc.dbinfo.dwForgePoint;
	suprop.dwHitPoint = GetPropValue(PI_HITRATE);
	suprop.dwMagicNicety = GetPropValue(PI_MAGHITRATE);
	suprop.dwAntiMagic = GetPropValue(PI_MAGESCAPE);
	suprop.dwPoisonNicety = GetPropValue(PI_POISONHITRATE);
	suprop.dwAntiPoison = GetPropValue(PI_POISONESCAPE);
	SendMsg(dwArr, SM_UPDATESUBPROP, w1, w2, w3, &suprop, sizeof(suprop));
}

BOOL CHumanPlayer::Trade(CHumanPlayer* pPlayer)
{
	BOOL IsFasst = FALSE;
	if (pPlayer == nullptr)
	{
		int x = getX();
		int y = getY();
		GETNEXTPOS(x, y, GetDirection());
		pPlayer = (CHumanPlayer*)m_pMap->FindObject(x, y, OBJ_PLAYER);
		if (pPlayer == nullptr)
		{
			SaySystem("对面没有人, 无法交易!");
			return FALSE;
		}
		int tx = pPlayer->getX();
		int ty = pPlayer->getY();
		GETNEXTPOS(tx, ty, pPlayer->GetDirection());
		if (tx != getX() || ty != getY())
		{
			SaySystem("对方没有面对着你, 无法交易!");
			return FALSE;
		}
	}
	else
		IsFasst = TRUE;
	m_pExchangeObj = new CExchangeObj;
	if (m_pExchangeObj == nullptr)
		return FALSE;
	m_pExchangeObj->SetFastExchange(IsFasst);
	pPlayer->m_pExchangeObj = m_pExchangeObj;
	return m_pExchangeObj->Begin(this, pPlayer);
}

VOID CHumanPlayer::GetViewDetail(xPacket& packet)
{
	//基本信息
	VIEWDETAIL detail;
	o_strncpy(detail.szName, GetName(), 15);
	detail.btNamelen = (BYTE)strlen(detail.szName);
	if (m_pGuild)
	{
		detail.btGuildNamelen = (BYTE)strlen(m_pGuild->GetName());
		o_strncpy(detail.szGuildName, m_pGuild->GetName(), 14);
		if (detail.btGuildNamelen > 14)detail.btGuildNamelen = 14;
		o_strncpy(detail.szTitleName, m_szGuildTitle, 15);
		detail.btTitleNameLen = (BYTE)strlen(m_szGuildTitle);
		if (detail.btTitleNameLen > 15)
			detail.btTitleNameLen = 15;
	}
	detail.dwFeature = GetFeather();
	detail.dwNameColor = GetNameColor(this);
	detail.btSex = GetSex();
	for (int i = 0; i < _U_MAX; i++)
	{
		detail.items[i] = *(ITEMCLIENT*)&m_Equipments[i];
		detail.items[i].SetLen(); // 设置物品长度
	}
	//扩展信息
	VIEWDETAIL_EX detailEx;
	detailEx.wLevel = GetPropValue(PI_LEVEL); // 玩家等级
	detailEx.btJob = GetPro(); // 玩家职业
	memset(detailEx.btExtKown, 255, 17);
	detailEx.btExtKown[0] = 0;
	//push数据
	packet.push(&detail, sizeof(VIEWDETAIL));
	int nValue = (BYTE)strlen(m_Humandesc.dbinfo.szPersonSign);
	packet.push(&nValue, 1); // 个性签名长度
	const char* pszPersonSign = m_Humandesc.dbinfo.szPersonSign;
	packet.push(pszPersonSign);
	packet.push(&detailEx, sizeof(VIEWDETAIL_EX));
}

BOOL CHumanPlayer::CreateBagItem(const char* pszName, BOOL IsBing)
{
	if (m_pClientObj == nullptr)return FALSE;
	return CItemManager::GetInstance()->CreateItem(pszName, m_pClientObj->getId(), m_pClientObj->GetKey(), GetDBId(), 0, IDF_BAG, IsBing);
}

int CHumanPlayer::GetEquipments(EQUIPMENT* pEquipments)
{
	int count = 0;
	for (int i = 0; i < _U_MAX; i++)
	{
		if (m_Equipments[i].dwMakeIndex != 0)
		{
			pEquipments[count].item = *(ITEMCLIENT*)&m_Equipments[i];
			pEquipments[count].pos = i;
			count++;
		}
	}
	return count;
}

int CHumanPlayer::GetVarValue(const char* pszVar) const
{
	if (_stricmp(pszVar, "job") == 0)
		return m_Humandesc.dbinfo.btClass;
	if (_stricmp(pszVar, "sex") == 0)
		return m_Humandesc.dbinfo.btSex;
	if (_stricmp(pszVar, "gold") == 0)
		return m_Humandesc.dbinfo.dwGold;
	if (_stricmp(pszVar, "yuanbao") == 0)
		return m_Humandesc.dbinfo.dwYuanbao;
	if (_stricmp(pszVar, "level") == 0)
		return m_Humandesc.dbinfo.wLevel;
	if (_stricmp(pszVar, "pkpoint") == 0)
		return m_dwPkValue;
	if (_stricmp(pszVar, "credit") == 0)
		return (m_Humandesc.dbinfo.dwFlag[0] & 0xffff);
	return -1;
}

VOID CHumanPlayer::DoProcess(OBJECTPROCESS* pProcess)
{
	if (pProcess == nullptr) return;
	switch (pProcess->ident)
	{
	case EP_PUTITEM:
	{
		SendMsg(0, 0x8850, 0, 0, 0, (LPVOID)pProcess->pszParam);
	}
	break;
	case EP_EXCHANGEBOX:
	{
		char szTempBuffer[128]{};
		xPacket packet(szTempBuffer, 128);
		DWORD nLooks = pProcess->dwParam[0];
		packet.push(&nLooks, 2);
		packet.push(pProcess->pszParam);
		packet.push(1);
		SendMsg(GetId(), 0x273, MAKEWORD(6, pProcess->dwParam[1]), 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
		SaySystem("恭喜 %s 宝箱开启, 获得 %s 物品!", GetName(), pProcess->pszParam);
		CreateBagItem(pProcess->pszParam);
	}
	break;
	case EP_MAPTELEPORT:
	{
		DWORD dwMapId = GetMapId();
		if (dwMapId >= pProcess->dwParam[0] &&
			dwMapId <= pProcess->dwParam[1])
		{
			if (pProcess->dwParam[3] == 0)
				RandomTeleport(pProcess->dwParam[2]);
			else
				FlyTo(pProcess->dwParam[2], pProcess->dwParam[3] & 0xffff, (pProcess->dwParam[3] & 0xffff0000) >> 16);
		}
	}
	break;
	case EP_WEATHERCHANGED:
	{
		if (pProcess->dwParam[0] == GetMapId()) SendTimeWeatherChanged();
	}
	break;
	case EP_TIMECHANGED:
	{
		// 取得当前地图的天气, 然后结合自己的时间
		SendTimeWeatherChanged();
	}
	break;
	case EP_CATCHHORSE:
	{
		if (m_pHorse)
		{
			ITEM item;
			if (CItemManager::GetInstance()->CreateTempItem("马牌", item))
			{
				o_strncpy((char*)&item.baseitem.btMinDef, m_pHorse->GetName(), 10);
				ITEM old = item;
				if (EquipItem(_U_CHARM, item, FALSE, FALSE))
				{
					if (item.dwMakeIndex != 0) AddBagItem(item);
					SendMsg(GetId(), SM_ADDBAGITEM, 0, 0, 1, &old, sizeof(old));
					SendMsg(old.dwMakeIndex, 0x23, _U_CHARM, 0, 0);
				}
			}
		}
	}
	break;
	case EP_GODBLESS:
	{
		SendAroundMsg(GetId(), 0x44d, pProcess->dwParam[0] >> 16, pProcess->dwParam[0] & 0xffff, pProcess->dwParam[1]);
		SendMsg(GetId(), 0x44d, pProcess->dwParam[0] >> 16, pProcess->dwParam[0] & 0xffff, pProcess->dwParam[1]);
	}
	break;
	case EP_SPECIALPOTION:
	{
		DWORD Idx = pProcess->dwParam[0]; // 索引序号
		DWORD nPower = pProcess->dwParam[1]; // 效果值
		DWORD nTime = pProcess->dwParam[2] / 1000; // 持续时间
		static const SpecialPotionInfo specialPotionInfo[6] = {
			{PI_MAXDC,       "攻击力增加%d秒"},
			{PI_MAXMC,       "魔法力增加%d秒"},
			{PI_MAXSC,       "道术增加%d秒"},
			{PI_ATTACKSPEED, "攻击速度增加%d秒"},
			{PI_MAXHP,       "生命值增加%d秒"},
			{PI_MAXMP,       "魔法值增加%d秒"},
		};
		if (pProcess->repeattimes == 1)
			DecProp(specialPotionInfo[Idx].propIndex, nPower);
		else
		{
			AddProp(specialPotionInfo[Idx].propIndex, nPower);
			SaySystemAttrib(CC_GREEN, specialPotionInfo[Idx].szMsg, nTime);
		}
		UpdateProp();
	}
	break;
	case EP_APPEAR:
	{
		Show();
	}
	break;
	case EP_WARSTART:
	{
		if (InWarArea())
		{
			SendMsg(4, 0x2c4, 0, 0, 0);
			UpdateViewName();
		}
		else if (!InCityArea())
			SendChangeName();
	}
	break;
	case EP_WAREND:
	{
		if (InWarArea())
		{
			SendMsg(0, 0x2c4, 0, 0, 0);
			UpdateViewName();
		}
		else if (!InCityArea())
			SendChangeName();
	}
	break;
	case EP_CREATEBAGITEM:
	{
		if (pProcess->dwParam[0] > 0)
		{
			for (DWORD i = 0; i < pProcess->dwParam[0]; i++)
			{
				CreateBagItem(pProcess->pszParam);
			}
		}
		else
			CreateBagItem(pProcess->pszParam);
	}
	break;
	case EP_RANDOMTELEPORT:
	{
		if (pProcess->dwParam[0] == 0)
			RandomTeleport();
		else
			RandomTeleport(pProcess->dwParam[0]);
	}
	break;
	case EP_HOME:
	{
		Home();
	}
	break;
	case EP_SABUKHOME:
	{
		CSandCity::GetInstance()->Home(this);
	}
	break;
	case EP_SENDCODEDTEXT:
	{
		if (m_pClientObj != nullptr)
			m_pClientObj->PostMsg(pProcess->pszParam, (int)strlen(pProcess->pszParam));
	}
	break;
	case EP_SCROLLTEXT:
	{
		SendScrollText(pProcess->pszParam);
	}
	break;
	case EP_SECNONDTIMEOUT:
	{
		SendMsg(pProcess->dwParam[0], 0x0339, pProcess->dwParam[1], pProcess->dwParam[2], 0, pProcess->pszParam);
	}
	break;
	case EP_RECOVERHP:
	{
		WORD curhp = static_cast<WORD>(GetPropValue(PI_CURHP));
		WORD maxhp = static_cast<WORD>(GetPropValue(PI_MAXHP));
		if (IsDeath())
		{
			pProcess->repeattimes = 0;
			SendMsg(GetId(), 510, 0, 0, 1);
			break;
		}
		if (pProcess->repeattimes == 1)
			SendMsg(GetId(), 510, 0, 0, 1);
		if (curhp == maxhp) break;
		WORD addHp = (WORD)pProcess->dwParam[0];
		if (addHp >= maxhp - curhp)
			curhp = maxhp;
		else
			curhp += addHp;
		m_Humandesc.dbinfo.hp = curhp;
		SendHpMpChanged(-addHp);
	}
	break;
	case EP_RECOVERMP:
	{
		WORD curmp = GetPropValue(PI_CURMP);
		WORD maxmp = GetPropValue(PI_MAXMP);
		if (IsDeath())
		{
			pProcess->repeattimes = 0;
			SendMsg(GetId(), 510, 0, 0, 2);
			break;
		}
		if (pProcess->repeattimes == 1)
			SendMsg(GetId(), 510, 0, 0, 2);
		if (curmp == maxmp) break;
		WORD addMp = (WORD)pProcess->dwParam[0];
		if (addMp >= maxmp - curmp)
			curmp = maxmp;
		else
			curmp += addMp;
		m_Humandesc.dbinfo.mp = curmp;
		SendHpMpChanged();
	}
	break;
	case EP_FIRSTLOGINPROCESS:
	{
		//赠送物品
		FIRSTLOGIN_INFO* pInfo = CGameWorld::GetInstance()->GetFirstLoginInfo();
		FIRSTLOGIN_ITEM* pItem = pInfo->pItems;
		while (pItem)
		{
			for (int i = 0; i < pItem->nCount; i++)
			{
				if (pItem->btJob != 99 && pItem->btJob != GetPro())
					continue;
				if (pItem->btSex != 99 && pItem->btSex != GetSex())
					continue;
				CreateBagItem(pItem->szItem, pItem->boBind);
			}
			pItem = pItem->pNext;
		}
		CSystemScript::GetInstance()->Execute(GetScriptTarget(), "FirstEnv.First", FALSE);
		CSystemScript::GetInstance()->Execute(GetScriptTarget(), "LoginEnv.Login", FALSE);
		//清除首次登录记录
		ClearFirstLogin();
	}
	break;
	case EP_CLOSEPAGE:
	{
		SendMsg(pProcess->dwParam[0], 0x284, 0, 0, 0, nullptr);
	}
	break;
	case EP_OPENSCRIPTPAGE:
	{
		if (pProcess->dwParam[0] == 0xffffffff)
		{
			if (!CSystemScript::GetInstance()->Execute(GetScriptTarget(), pProcess->pszParam, FALSE))
				SendCloseScriptPage(0xffffffff);
			break;
		}
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pProcess->dwParam[0]);
		if (pNpc != nullptr)
			pNpc->QuerySelectLink(this, pProcess->pszParam, FALSE);
	}
	break;
	case EP_FIREBURN: // 火墙
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		CFireBurnEvent::Create(this, x, y, pProcess->dwParam[1], pProcess->dwParam[2], pProcess->dwParam[3] * 1000);
	}
	break;
	case EP_POISON: // 施毒术
	{
		BOOL bSuccess = TRUE;
		DWORD nTarget = pProcess->dwParam[0];
		CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(nTarget); // 攻击目标
		DWORD needitem = pProcess->dwParam[1];
		DWORD du = pProcess->dwParam[2];
		int hongdu = du & 0xffff; // 低位
		int lvdu = (du & 0xffff0000) >> 16; // 高位
		int nPower = pProcess->dwParam[3];
		if (!pObject->IsSkillTime(6)) break;
		nPower = nPower * (1 - pObject->GetPropValue(PI_POISONRECOVER));
		if (needitem == SNI_REDPOISON) // 红毒30
		{ 
			pObject->SetStatus(SI_REDPOISON, hongdu, nPower * 1000);
			pObject->SetSetter(SI_REDPOISON, GetId()); // 设置施毒者
		}
		else if (needitem == SNI_GREENPOISON) // 绿毒31
		{ 
			pObject->SetStatus(SI_GREENPOISON, lvdu, nPower * 1000);
			pObject->SetSetter(SI_GREENPOISON, GetId()); // 设置施毒者
		}
		else
			bSuccess = FALSE;
		int escape = pObject->GetPropValue(PI_POISONESCAPE);
		if (escape > 0)
		{
			if (GetPropValue(PI_POISONHITRATE) < Getrand(escape))
				bSuccess = FALSE;
		}
		if (nPower <= 0)
			bSuccess = FALSE;
		//	防止隐身毒
		if (bSuccess)
		{
			if (pObject->IsProperTarget(this))
				pObject->SetTarget(this);
			CheckPk(pObject);
			SetPetTarget(pObject);
		}
	}
	break;
	case EP_STRAW: // 诅咒术
	{
		BOOL bSuccess = TRUE;
		DWORD nTarget = pProcess->dwParam[0];
		CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(nTarget); // 攻击目标
		DWORD needitem = pProcess->dwParam[1];
		DWORD du = pProcess->dwParam[2];
		int value2 = du & 0xffff; // 低位
		int value1 = (du & 0xffff0000) >> 16; // 高位
		int nPower = pProcess->dwParam[3];
		if (!pObject->IsSkillTime(45)) break;
		nPower = nPower * (1 - pObject->GetPropValue(PI_POISONRECOVER));
		if (needitem == SNI_STRAWMAN)
			pObject->SetStatus(SI_STRAWMAN, value2, nPower * 1000);
		else if (needitem == SNI_STRAWWOMAN)
			pObject->SetStatus(SI_STRAWWOMAN, value1, nPower * 1000);
		else
			bSuccess = FALSE;
		int escape = pObject->GetPropValue(PI_POISONESCAPE);
		if (escape > 0)
		{
			if (GetPropValue(PI_POISONHITRATE) < Getrand(escape))
				bSuccess = FALSE;
		}
		if (nPower <= 0)
			bSuccess = FALSE;
		//	防止隐身毒
		if (bSuccess)
		{
			if (pObject->IsProperTarget(this))
				pObject->SetTarget(this);
			CheckPk(pObject);
			SetPetTarget(pObject);
		}
	}
	break;
	case EP_DEFENCEUP: // 幽灵盾
	case EP_MAGDEFENCEUP: // 神圣战甲术
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		MagMakeDefenceArea(x, y, pProcess->dwParam[1], pProcess->dwParam[2], pProcess->dwParam[3]);
	}
	break;
	case EP_GROUPCLOAK: // 群体隐身术
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		for (int i = 0; i < 9; i++)
		{
			int px = x;
			int py = y;
			if (i < 8) GETNEXTPOS(px, py, i);
			CMapCellInfo* pInfo = m_pMap != nullptr ? m_pMap->GetMapCellInfo(px, py) : nullptr;
			if (pInfo)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CMapObject* pObject = pNode->getObject();
					pNode = pNode->getNext();
					if (pObject)
					{
						if (pObject->GetClassType() == CLS_ALIVEOBJECT && IsProperFriend((CAliveObject*)pObject))
						{
							DWORD dwParam = (pObject->getX() << 16) | pObject->getY();
							if (!((CAliveObject*)pObject)->IsStatusSet(SI_CLOAK))
								((CAliveObject*)pObject)->AddProcess(EP_SETSTATUS, SI_CLOAK, dwParam, pProcess->dwParam[1] * 1000);
						}
					}
				}
			}
		}
	}
	break;
	case EP_GROUPMAGHEALING: // 群体治愈术
	{
		DWORD xy = pProcess->dwParam[0];
		int x = xy & 0xffff; // 低位
		int y = (xy & 0xffff0000) >> 16; // 高位
		int nRange = pProcess->dwParam[1]; // 范围
		int nHP = pProcess->dwParam[2]; // 回血总量
		int nTimeHp = pProcess->dwParam[3]; //每次加血量
		for (int i = 0; i < nRange; i++)
		{
			int px = x;
			int py = y;
			CMapCellInfo* pInfo = m_pMap != nullptr ? m_pMap->GetMapCellInfo(px, py) : nullptr;
			if (i < 8) GETNEXTPOS(px, py, i);
			if (pInfo)
			{
				xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
				while (pNode)
				{
					CMapObject* pObject = pNode->getObject();
					pNode = pNode->getNext();
					if (pObject)
					{
						if (pObject->GetClassType() == CLS_ALIVEOBJECT && IsProperFriend((CAliveObject*)pObject))
							((CAliveObject*)pObject)->AddProcess(EP_MAGHEALING, nHP, nTimeHp, 0, 0, 600);
					}
				}
			}
		}
	}
	break;
	case EP_SKILLFLY:
	{
		m_pMap->MoveObject(this, pProcess->dwParam[0], pProcess->dwParam[1]);
	}
	break;
	default:
	{
		CAliveObject::DoProcess(pProcess);
	}
	break;
	}
}

VOID CHumanPlayer::Update()
{
	//分帧更新
	const DWORD dwUpdateKey = GetUpdateKey();
	DWORD dwkey = (dwUpdateKey % 2);
	switch (dwkey)
	{
	case 0:
	{
		if (m_bJustPk)// 检测PK
		{
			if (m_tmrJustPk.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_GRAYNAMETIME) * 1000))
			{
				m_bJustPk = FALSE;
				SendChangeName();
			}
		}
		if (m_dwPkValue > 0) // PK值大于0
		{
			if (m_PkPointTimer.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_ONEPKPOINTTIME) * 1000))
			{
				BYTE btColor = GetNameColor(this);
				m_dwPkValue--;
				m_PkPointTimer.Savetime();
				if (btColor != GetNameColor(this))
					SendChangeName();
			}
		}
		// 6分钟检查
		if (m_Humandesc.dbinfo.wLevel >= 7 && m_StaminaTimer.IsTimeOut(6 * 60 * 1000))
		{
			int expFactor = (int)ceilf(m_pMap->GetExpFactor());
			if (expFactor > 1)
				m_wStamina += expFactor;
			else
				m_wStamina++;
			if (m_wStamina <= m_wMaxStamina)
			{
				SendJingLiZhi(m_wStamina);
				UpdateProp();
			}
			else
				m_wStamina = m_wMaxStamina;
			m_StaminaTimer.Savetime();
		}
		// 30分钟保存数据
		if (m_DBTimer.IsTimeOut(CGameWorld::GetInstance()->GetVar(EVI_CHARINFOBACKUPTIME) * 60 * 1000))
		{
			if (CGameWorld::GetInstance()->CanSaveToDB())
			{
				CGameWorld::GetInstance()->UpdateDBUpdateTimer();
				m_DBTimer.Savetime();
				UpdateToDB();
			}
		}
		// 1秒检查
		if (m_tmrGameTime.IsTimeOut(1000))
		{
			m_tmrGameTime.Savetime();
			if (m_Humandesc.dbinfo.nGameTime > -1) // 时长区-游戏时间计算
			{
				if (m_Humandesc.dbinfo.nGameTime != 0)
				{
					m_Humandesc.dbinfo.nGameTime--;
					if (m_Humandesc.dbinfo.nGameTime == 0)
						CSystemScript::GetInstance()->Execute(GetScriptTarget(), "GameTimeMgr.TimeOver", FALSE);
				}
			}
			
			if (m_bYuanQi == FALSE && m_wYuanQi < 2000)
			{
				m_wYuanQi++;
				SendMsg(GetId(), 0x9611, m_wYuanQi, 2000, 0);
				if (m_wYuanQi >= 2000)
					m_bYuanQi = TRUE;
			}
		}
		// 60秒检查
		if (m_tmrFenghaoTime.IsTimeOut(60*1000))
		{
			m_tmrFenghaoTime.Savetime();
			CheckFengHaoTimeOut();
		}
	}
	break;
	case 1:
	{
		if (this->IsSpecialEquipmentFunctionOn(SEF_CLOAK))// 检测隐身
		{
			if (!IsStatusSet(SI_CLOAK))
			{
				if (CanDoAction(AT_ATTACK))
					SetStatus(SI_CLOAK, 0, 0xffffffff);
			}
		}
		if (m_pAddToGuildRequester != nullptr) // 请求加入行会人
		{
			if (m_AddToGuildTimer.IsTimeOut(60 * 1000))
				ReplyAddToGuildRequest(FALSE);
		}
		// 检测烈火剑法、雷霆剑
		if (m_pTimeOutDeActiveMagic != nullptr && (m_pTimeOutDeActiveMagic->dwFlag & USERMAGICFLAG_ACTIVED))
		{
			if (m_pTimeOutDeActiveMagic->useTimer.IsTimeOut())
			{
				m_pTimeOutDeActiveMagic->dwFlag ^= USERMAGICFLAG_ACTIVED;
				if (m_pTimeOutDeActiveMagic->magic.wId == 26)
				{
					SaySystem("精神火球消失");
					if (m_pClientObj)m_pClientObj->PostMsg("#+UFIR!", 7);
				}
				else if (m_pTimeOutDeActiveMagic->magic.wId == 44)
				{
					SaySystem("雷电力量消失");
					if (m_pClientObj)m_pClientObj->PostMsg("#+UTHU!", 7);
				}
			}
		}
		if (m_pMap) // 检测进入地图
		{
			DWORD dwParam = 0;
			if (m_pMap->IsFlagSeted(MF_LEVELABOVE, dwParam))
			{
				if (GetPropValue(PI_LEVEL) <= LOWORD(dwParam))
				{
					SaySystem("您等级不符合本地图的要求!");
					if (HIWORD(dwParam) > 0)
						RandomTeleport(HIWORD(dwParam));
					else
						EscapeMap();
				}
			}
			if (m_pMap->IsFlagSeted(MF_LEVELBELOW, dwParam))
			{
				if (GetPropValue(PI_LEVEL) >= LOWORD(dwParam))
				{
					SaySystem("您等级不符合本地图的要求!");
					if (HIWORD(dwParam) > 0)
						RandomTeleport(HIWORD(dwParam));
					else
						EscapeMap();
				}
			}
			if (m_pMap->IsFlagSeted(MF_PKPOINTABOVE, dwParam))
			{
				if (GetPkValue() <= LOWORD(dwParam))
				{
					SaySystem("您的PK值不符合本地图的要求!");
					if (HIWORD(dwParam) > 0)
						RandomTeleport(HIWORD(dwParam));
					else
						EscapeMap();
				}
			}
			if (m_pMap->IsFlagSeted(MF_PKPOINTBELOW, dwParam))
			{
				if (GetPkValue() >= LOWORD(dwParam))
				{
					SaySystem("您的PK值不符合本地图的要求!");
					if (HIWORD(dwParam) > 0)
						RandomTeleport(HIWORD(dwParam));
					else
						EscapeMap();
				}
			}
		}
	}
	break;
	}
	if (m_pExchangeObj != nullptr && !m_pExchangeObj->GetFastExchange())//如果不是快速交易, 就检测双方占位
	{
		if (!CheckTradeOtherSideOk(m_pExchangeObj->GetOtherSidePlayer(this)))
			m_pExchangeObj->End(this, ET_CANCEL);
	}
	CAliveObject::Update();
}

VOID CHumanPlayer::AddExp(DWORD dwExp, int level, DWORD dwId)
{
	//	以后要根据level来算出经验, 还有组队的因素
	if (m_pGroupObject)
		m_pGroupObject->AdjustGroupExp(this, dwExp, dwId);
	else
		WinExp(dwExp, FALSE, dwId);
}

VOID CHumanPlayer::WinExp(DWORD dwExp, BOOL bNoBonus, DWORD dwId)
{
	if (!bNoBonus)
	{
		DWORD dwAddExp = 0;
		if (m_pExpMagic)
		{
			m_pExpMagic->nAddPower--;
			if (m_pExpMagic->nAddPower <= 0)
			{
				dwAddExp = dwExp;
				m_pExpMagic->nAddPower = (7 - m_pExpMagic->magic.btLevel);
				m_pExpMagic->nAddPower -= Getrand(m_pExpMagic->nAddPower);
				if (m_pExpMagic->pClass->szSpecial[0] != 0)SaySystem(m_pExpMagic->pClass->szSpecial);
				TrainMagic(m_pExpMagic);
			}
		}
		//World配置倍数 + 地图倍数
		float factor = CGameWorld::GetInstance()->GetExpFactor() + (m_pMap != nullptr ? m_pMap->GetExpFactor() : 0.0f);
		if (dwId > 0)
		{
			if (IsGodBlessEffective(SG_DOUBLEEXP))
			{
				factor += 1.0f;
				AddProcess(EP_GODBLESS, dwId, 8);
			}
		}
		dwExp = ROUND(factor * dwExp) + dwAddExp;
		dwExp = ROUND(GetExpFactor() * dwExp);
	}
	m_Humandesc.dbinfo.dwCurExp += dwExp;

	SendMsg(m_Humandesc.dbinfo.dwCurExp, SM_GETEXP, dwExp & 0xffff, (dwExp & 0xffff0000) >> 16, 0, nullptr);
	for (int i = 0; i < m_iPetCount; i++)
	{
		if (m_pPets[i] && !m_pPets[i]->IsDeath())
		{
			dwExp /= 10;
			if (dwExp == 0)dwExp = 1;
			((CMonsterEx*)m_pPets[i])->WinExp(dwExp);
		}
	}
	if (m_Humandesc.dbinfo.dwCurExp >= m_pHumanDataDesc->dwLevelupExp)
	{
		m_Humandesc.dbinfo.dwCurExp -= m_pHumanDataDesc->dwLevelupExp;
		LevelUp(m_Humandesc.dbinfo.wLevel + 1);
	}
	CheckAndUpgradeTitle();
}

VOID CHumanPlayer::SetExp(DWORD dwExp)
{
	m_Humandesc.dbinfo.dwCurExp = dwExp;
}

VOID CHumanPlayer::LevelUp(int level)
{
	if (level == m_Humandesc.dbinfo.wLevel)return;
	HUMANDATADESC* pHumanData = CGameWorld::GetInstance()->GetHumanDataDesc(m_Humandesc.dbinfo.btClass, level);
	if (pHumanData == nullptr)return;
	m_Humandesc.dbinfo.wLevel = level;
	m_Humandesc.dbinfo.hp = pHumanData->wHp;
	m_Humandesc.dbinfo.mp = pHumanData->wMp;
	m_Humandesc.dbinfo.maxhp = pHumanData->wHp;
	m_Humandesc.dbinfo.maxmp = pHumanData->wMp;
	m_Humandesc.dbinfo.handweight = pHumanData->handweight;
	m_Humandesc.dbinfo.weight = pHumanData->bagweight;
	m_Humandesc.dbinfo.bodyweight = pHumanData->bodyweight;
	m_Humandesc.dbinfo.maxac = pHumanData->maxac;
	m_Humandesc.dbinfo.minac = pHumanData->minac;
	m_Humandesc.dbinfo.maxmac = pHumanData->maxmac;
	m_Humandesc.dbinfo.minmac = pHumanData->minmac;
	m_Humandesc.dbinfo.maxmc = pHumanData->maxmc;
	m_Humandesc.dbinfo.minmc = pHumanData->minmc;
	m_Humandesc.dbinfo.maxsc = pHumanData->maxsc;
	m_Humandesc.dbinfo.minsc = pHumanData->minsc;
	m_Humandesc.dbinfo.maxdc = pHumanData->maxdc;
	m_Humandesc.dbinfo.mindc = pHumanData->mindc;
	m_pHumanDataDesc = pHumanData;
	OnLevelUp(level);
	DWORD dwId = GetId();
	SendMsg(m_Humandesc.dbinfo.dwCurExp, SM_LEVELUP, level, dwId & 0xffff, (dwId >> 16) & 0xffff, 0, 0); //发送升级特效
	UpdateProp();
	UpdateSubProp();
}

VOID CHumanPlayer::OnLevelUp(int level)
{
	if (level == 7) // 升级到 7 发送加载BOSS图鉴列表数据
		CBossTJ::GetInstance()->SendBossList(this);
	if (level > 45) // 46级开始才有封号判断
		CheckAndUpgradeTitle();
	CSystemScript::GetInstance()->Execute(GetScriptTarget(), "LevelUpEnv.LevelUp", FALSE);
}

VOID CHumanPlayer::SendWeightChanged()
{
	SendMsg(GetPropValue(PI_CURBAGWEIGHT), SM_WEIGHTCHANGED, GetPropValue(PI_CURBODYWEIGHT), GetPropValue(PI_CURHANDWEIGHT), 0, 0, 0);
}

VOID CHumanPlayer::SendGoldChanged(DWORD dwChanged)
{
	SendMsg(m_Humandesc.dbinfo.dwGold, SM_SETGOLD, 0, 0, 0, 0, 0);
}

VOID CHumanPlayer::SendEatOk()
{
	SendMsg(0, SM_EAT_OK, 0, 0, 0, 0, 0);
}

VOID CHumanPlayer::SendEatFail()
{
	SendMsg(0, SM_EAT_FAIL, 0, 0, 0, 0, 0);
}

BOOL CHumanPlayer::AddGold(DWORD dwCount, BOOL bUpdateClient)
{
	DWORD dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXGOLD);
	if (m_Humandesc.dbinfo.dwGold > dwMaxGold)return FALSE;
	if (dwCount > (dwMaxGold - m_Humandesc.dbinfo.dwGold))return FALSE;
	m_Humandesc.dbinfo.dwGold += dwCount;
	if (bUpdateClient)SendGoldChanged();
	return TRUE;
}

BOOL CHumanPlayer::PetAddgold(DWORD dwgold, BOOL bUpdateClient)
{
	DWORD dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXGOLD);
	if (m_Humandesc.dbinfo.dwGold > dwMaxGold)return FALSE;
	if (dwgold > (dwMaxGold - m_Humandesc.dbinfo.dwGold))return FALSE;
	m_Humandesc.dbinfo.dwGold += dwgold;
	if (bUpdateClient)SendGoldChanged();
	return TRUE;
}

BOOL CHumanPlayer::TakeGold(DWORD dwCount, BOOL bUpdateClient)
{
	if (m_Humandesc.dbinfo.dwGold < dwCount)
		return FALSE;
	m_Humandesc.dbinfo.dwGold -= dwCount;
	if (bUpdateClient)SendGoldChanged();
	return TRUE;
}

BOOL CHumanPlayer::TestAddGold(DWORD dwCount)const
{
	DWORD dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXGOLD);
	if (m_Humandesc.dbinfo.dwGold > dwMaxGold)return FALSE;
	if (dwCount > (dwMaxGold - m_Humandesc.dbinfo.dwGold))return FALSE;
	return TRUE;
}

BOOL CHumanPlayer::TestAddMoney(money_type type, DWORD dwCount)const
{
	DWORD dwMax = CGameWorld::GetInstance()->GetVar(type == MT_GOLD ? EVI_MAXGOLD : EVI_MAXYUANBAO);
	DWORD dwCur = GetMoney(type);
	if (dwCur > dwMax)return FALSE;
	if (dwCount > (dwMax - dwCur))return FALSE;
	return TRUE;
}

VOID CHumanPlayer::DropGold(DWORD dwCount)
{
	if (dwCount > m_Humandesc.dbinfo.dwGold)
	{
		SaySystem("您没有那么多钱可以扔!");
		return;
	}
	POINT pt;
	int count = m_pMap->GetDropItemPoint(getX(), getY(), &pt, 1);
	if (count > 0)
	{
		m_Humandesc.dbinfo.dwGold -= dwCount;
		CAliveObject::DropGold(dwCount, pt.x, pt.y);
		SendGoldChanged();
	}
}

VOID CHumanPlayer::SendScrollText(const char* pszText)
{
	SendMsg(0, SM_SCROLLTEXT, 0, 0, 0, (LPVOID)pszText);
}

VOID CHumanPlayer::SendClientKeyConfig()
{
	char szTempBuffer[6144]{};
	xPacket packet(szTempBuffer, 6143);
	int nValue = 0x64;
	packet.push(&nValue, 4);
	ClientKeyState* clientKeyConfig = CGameWorld::GetInstance()->GetClientKeyConfig();
	packet.push(clientKeyConfig, sizeof(ClientKeyState) * 100);
	SendMsg(GetId(), 0x97a, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CHumanPlayer::SendClientPluginInfo()
{
	char szTempBuffer[512]{};
	xPacket packet(szTempBuffer, 512);
	//位运算，开启客户端插件功能：
	//开特权大包裹
	//内挂持续使用
	//挂机绑金上限
	//时长充值按钮
	//玄武炉无限制
	//开启物品来源
	//开启无限刀
	//整理包裹触发
	int nValue = 255;
	packet.push(&nValue, 4);
	//插入包裹名称
	const char* sBagName = "VIP包裹";
	packet.push((LPVOID)sBagName, 15);
	packet.push(1);
	//插入充值网址
	const char* sPayWeb = "https://www.jiangjiali.com";
	packet.push((LPVOID)sPayWeb, 254);
	packet.push(1);
	SendMsg(GetId(), 0xa02, 0, 10086, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CHumanPlayer::UpdateToDB()
{
	if (this->m_pClientObj == nullptr)return;
	CHARDBINFO info;
	CDBClientObj* pObj = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
	if (pObj == nullptr)return;

	if (GetDBInfo(info))
	{
		if (!IsDeath())
		{
			if (info.hp == 0)
				info.hp = info.maxhp;
		}
		else
			info.hp = 0;
		info.dwFlag[1] = GetPkValue();
		pObj->SendPutDbInfo(GetId(), m_pClientObj->GetKey(), info);
	}
	if (IsSystemFlagSeted(SF_COMMUNITYLOADED))
	{
		char szCommunityText[4096];
		int length = GetCommunityInfo(szCommunityText, 4096);
		pObj->UpdateCommunity(GetDBId(), szCommunityText);
	}
	//if( IsSystemFlagSeted( SF_BANKLOADED ) )
	//{
	//	BAGITEMPOS	itempos[100];
	//	int count = m_ItemBank.GetItemPos( itempos, 100 );
	//	if( count > 0 )pObj->SendUpdateItemPos( IDF_BANK, itempos, count );
	//}
	if (this->m_fMagicLoaded)
	{
		MAGICDB	array[255] = { 0 };
		USERMAGIC* pMagic = m_pMagic;
		int	count = 0;
		while (pMagic)
		{
			array[count++] = pMagic->magic;
			pMagic = pMagic->pNext;
		}
		if (count > 0)
			pObj->SendUpdateMagic(GetDBId(), count, array);
	}
}

VOID CHumanPlayer::SetMagic(MAGICDB* pArray, int count)
{
	USERMAGIC* pMagic = nullptr;
	for (int i = 0; i < count; i++)
	{
		pMagic = GetMagic(pArray[i].wId);
		if (pMagic == nullptr)
		{
			pMagic = CGameWorld::GetInstance()->AllocUserMagic();
			pMagic->magic = pArray[i];
			pMagic->pClass = CMagicManager::GetInstance()->GetClassById(pArray[i].wId);
			if (pMagic->pClass == nullptr)
			{
				CGameWorld::GetInstance()->FreeUserMagic(pMagic);
				continue;
			}
			pMagic->pNext = m_pMagic;
			m_pMagic = pMagic;
			pMagic->useTimer.SetTimeOut(0);
			OnAddMagic(pMagic);
		}
		else
		{
			pMagic->magic = pArray[i];
			pMagic->useTimer.SetTimeOut(0);
		}
	}
	RecalcHitSpeed();
	UpdateSubProp();
	m_fMagicLoaded = TRUE;
}

BOOL CHumanPlayer::AddMagic(WORD wId, BYTE btLevel)
{
	if (!m_fMagicLoaded)return FALSE;
	USERMAGIC* pUserMagic = GetMagic(wId);
	if (pUserMagic)return FALSE;
	MAGICCLASS* pMagicClass = CMagicManager::GetInstance()->GetClassById(wId);
	if (pMagicClass == nullptr)return FALSE;

	pUserMagic = CGameWorld::GetInstance()->AllocUserMagic();
	pUserMagic->magic.btKey = 0;
	pUserMagic->magic.btLevel = btLevel;
	pUserMagic->magic.dwCurTrain = 0;
	pUserMagic->magic.wId = wId;
	pUserMagic->pClass = pMagicClass;

	MAGIC magic;
	if (CMagicManager::GetInstance()->CreateMagic((UINT)wId, magic))
	{
		magic.btLevel = btLevel;
		SendMsg(0, 0xd2, 0, 0, 1, &magic, sizeof(magic));
		SaySystem("您学会了 %s 技能!", pMagicClass->szName);
		pUserMagic->pNext = m_pMagic;
		m_pMagic = pUserMagic;
		OnAddMagic(pUserMagic);
		RecalcHitSpeed();
		UpdateSubProp();
	}
	return TRUE;
}

BOOL CHumanPlayer::AddMagic(const char* pszName, BYTE btLevel)
{
	if (!m_fMagicLoaded)return FALSE;
	MAGICCLASS* pMagicClass = CMagicManager::GetInstance()->GetClassByName(pszName);
	if (pMagicClass == nullptr)return FALSE;
	USERMAGIC* pUserMagic = GetMagic(pMagicClass->id);
	if (pUserMagic)return FALSE;

	pUserMagic = CGameWorld::GetInstance()->AllocUserMagic();
	pUserMagic->magic.btKey = 0;
	pUserMagic->magic.btLevel = btLevel;
	pUserMagic->magic.dwCurTrain = 0;
	pUserMagic->magic.wId = pMagicClass->id;
	pUserMagic->pClass = pMagicClass;

	MAGIC magic;
	if (CMagicManager::GetInstance()->CreateMagic((UINT)pMagicClass->id, magic))
	{
		magic.btLevel = btLevel;
		SendMsg(0, 0xd2, 0, 0, 1, &magic, sizeof(magic));
		SaySystem("您学会了 %s 技能!", pszName);
		pUserMagic->pNext = m_pMagic;
		m_pMagic = pUserMagic;
		OnAddMagic(pUserMagic);
		RecalcHitSpeed();
		UpdateSubProp();
	}
	return TRUE;
}

VOID CHumanPlayer::SendMagicList()
{
	int count = 0;
	USERMAGIC* p = m_pMagic;
	MAGIC g_tmpMagicBuffer[27];//最多技能的是法师, 27个.
	while (p)
	{
		if (CMagicManager::GetInstance()->CreateMagic((UINT)p->magic.wId, g_tmpMagicBuffer[count]))
		{
			g_tmpMagicBuffer[count].cKey = (char)p->magic.btKey;
			g_tmpMagicBuffer[count].btLevel = p->magic.btLevel;
			g_tmpMagicBuffer[count].iCurExp = p->magic.dwCurTrain;
			count++;
		}
		p = p->pNext;
	}
	SendMsg(0, 0xd3, 0, 0, 0, (LPVOID)g_tmpMagicBuffer, sizeof(MAGIC) * count);
}

const char* CHumanPlayer::GetAccount()
{
	if (m_pClientObj)return m_pClientObj->GetAccount(); return "<nullptr>";
}

BOOL CHumanPlayer::GetViewmsg(char* pszMsg, int& length, CMapObject* pViewer)
{
	BOOL bRet = CAliveObject::GetViewmsg(pszMsg, length, pViewer);
	if (!bRet) return FALSE;
	if (m_szCurrentTitle[0] != 0)
	{
		WORD wFlag = 8 | ((m_iCurrentTitleIndex + 1) << 8);
		char szTempBuffer[1024];
		int tempSize = 0;
		SmartEncodeMessage(szTempBuffer, tempSize, GetId(), 0x532c, wFlag, 0, 0, (LPVOID)m_szCurrentTitle);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}
	WORD wFlag = 0;
	WORD wSecond = 0;
	if (IsSystemFlagSeted(SF_WINDSHIELD))
		wFlag |= 2;
	if (IsSystemFlagSeted(SF_STRONGSHIELD))
		wFlag |= 1;
	if (IsSystemFlagSeted(SF_GODBLESS))
	{
		wFlag |= 4;
		wSecond = m_SystemFlag.GetTimeOut(SF_GODBLESS) / 1000;
	}
	//SendMsg( GetId(), 0x532c, wFlag, wSecond, 0 );
	if (wFlag > 0)
	{
		char szTempBuffer[1024];
		int tempSize = 0;
		SmartEncodeMessage(szTempBuffer, tempSize, GetId(), 0x532c, wFlag, wSecond, 0, nullptr, 0);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}

	if (IsSpecialEquipmentFunctionOn(SEF_HEALHALO))
	{
		char szTempBuffer[1024];
		int tempSize = 0;
		SmartEncodeMessage(szTempBuffer, tempSize, GetId(), 0x532c, 0x10, 1, 0, nullptr, 0);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}
	else if (IsSpecialEquipmentFunctionOn(SEF_MIRAGE))
	{
		char szTempBuffer[1024];
		int tempSize = 0;
		SmartEncodeMessage(szTempBuffer, tempSize, GetId(), 0x532c, 0x10, 2, 0, nullptr, 0);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}

	if (GetActionType() == AT_PRIVATESHOP)
	{
		PRIVATESHOPHEADER psheader;
		o_strncpy(psheader.szName, m_szPrivateShopName, 51);
		char szTempBuffer[1024];
		int tempSize = 0;
		SmartEncodeMessage(szTempBuffer, tempSize, GetId(), 0xfca0, getX(), getY(), (WORD)GetDirection(),
			&psheader, sizeof(psheader));
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}

	DWORD dwParam = 0;
	if (IsSystemFlagSeted(SF_TRANSFORMED))
	{
		dwParam = GetSystemFlagParam(SF_TRANSFORMED);
		char szTempBuffer[1024];
		int tempSize = 0;
		SmartEncodeMessage(szTempBuffer, tempSize, GetId(), 0x532c, 0x40, (dwParam & 0xffff0000) >> 16, dwParam & 0xffff, nullptr);
		memcpy(pszMsg + length, szTempBuffer, tempSize);
		length += tempSize;
	}
	return TRUE;
}

VOID CHumanPlayer::SendWisper(const char* pszMsg, ...)
{
	char szBuff[248];
	va_list	vl;
	va_start(vl, pszMsg);
	_vsnprintf(szBuff, sizeof(szBuff) - 1, pszMsg, vl);
	va_end(vl);
	szBuff[120] = '\0';
	SendMsg(0, 0x67, 0xfffc, 0, 1, szBuff);
}

VOID CHumanPlayer::Cry(const char* pszMsg, ...)
{
	char szBuff[248];
	sprintf(szBuff, "(!)%s: ", GetName());
	va_list	vl;
	va_start(vl, pszMsg);
	size_t nLen = strlen(szBuff);
	_vsnprintf(szBuff + nLen, 247 - nLen, pszMsg, vl);
	va_end(vl);
	szBuff[120] = '\0';
	//	SendMsg( 0, 0x67, 0xfffc, 0, 1, szBuff );
	SendMsg(GetId(), 0x64, 0x9700, 0x38, 0x100, (LPVOID)szBuff);
	SendMapMsg(GetId(), 0x64, 0x9700, 0x38, 0x100, (LPVOID)szBuff);
}

VOID CHumanPlayer::SendTakeBagItem(ITEM* pItem)
{
	SendMsg(GetId(), 0xca, 0, 0, 1, (LPVOID)pItem, sizeof(ITEMCLIENT));
}

BOOL CHumanPlayer::AddPet(CAliveObject* pObject)
{
	if (pObject == nullptr) return FALSE;
	CMonsterEx* pPet = (CMonsterEx*)pObject;
	MonsterClass* pDesc = pPet->GetDesc();
	if (pDesc && pDesc->petset.Type == APT_RIDE)
	{
		SetHorse(pPet);
		return TRUE;
	}
	else if (pDesc && pDesc->petset.Type == APT_PET)
	{
		m_pPet = pPet;
		return TRUE;
	}
	//如果有列表里有,则直接返回
	for (int i = 0; i < m_iPetCount; i++)
	{
		if (m_pPets[i] == pObject) return TRUE;
	}
	if (m_iPetCount >= 5) return FALSE;
	m_pPets[m_iPetCount++] = pPet;
	return TRUE;
}

BOOL CHumanPlayer::DelPet(CAliveObject* pObject)
{
	if (pObject == nullptr) return FALSE;
	CMonsterEx* pPet = (CMonsterEx*)pObject;
	MonsterClass* pDesc = pPet->GetDesc();
	if (pDesc && pDesc->petset.Type == APT_RIDE) // 处理坐骑
	{
		if (m_pHorse == nullptr) return FALSE;
		SetHorse(nullptr);
		return TRUE;
	}
	else if (pDesc && pDesc->petset.Type == APT_PET) // 处理豹子宠物
	{
		if (m_pPet == nullptr) return FALSE;
		m_pPet->SetOwner(nullptr);//设置豹子的主人为空
		memset(petname, 0, sizeof(petname));
		ISzhaohuan = FALSE;
		m_baozhiID = 0;
		m_pPet = nullptr;
		return TRUE;
	}
	// 处理普通宠物列表
	for (int i = 0; i < m_iPetCount; i++)
	{
		if (m_pPets[i] == pPet)
		{
			for (int j = i; j < m_iPetCount - 1; j++)
			{
				m_pPets[j] = m_pPets[j + 1];
			}
			m_iPetCount--;
			m_pPets[m_iPetCount] = nullptr;
			break;
		}
	}
	return TRUE;
}

VOID CHumanPlayer::OnStatusSet(int index, DWORD dwParam)
{
	CAliveObject::OnStatusSet(index, dwParam);
	UpdateProp();
}

VOID CHumanPlayer::OnStatusClr(int index, DWORD dwParam)
{
	CAliveObject::OnStatusClr(index, dwParam);
	UpdateProp();
}

VOID CHumanPlayer::Send_Exchg_OtherAddItem(CHumanPlayer* pOther, ITEM& item)
{
	SendMsg(pOther->GetId(), SM_OTHERPUTTRADEITEM, 0, 0, 0, &item, sizeof(ITEMCLIENT));
}

VOID CHumanPlayer::Send_Exchg_OtherAddMoney(CHumanPlayer* pOther, money_type type, DWORD dwCount)
{
	SendMsg(dwCount, SM_OTHERPUTTRADEGOLD, 0, 0, (WORD)type, nullptr);
}

BOOL CHumanPlayer::PutTradeItem(DWORD dwMakeIndex)
{
	if (m_pExchangeObj == nullptr)return FALSE;
	ITEM* pItem = this->m_ItemBox.FindItem(dwMakeIndex);
	if (pItem == nullptr)return FALSE;
	if (CItemManager::GetInstance()->ItemLimited(*pItem, IL_NOEXCHANGE))
	{
		SaySystem("该物品不能交易");
		return FALSE;
	}
	if (m_pExchangeObj->PutItem(this, *pItem))
	{
		m_ItemBox.RemoveItem(pItem->dwMakeIndex);
		return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::PutTradeMoney(money_type type, DWORD dwCount)
{
	if (m_pExchangeObj == nullptr)return FALSE;
	DWORD dwTradeGold = m_pExchangeObj->GetGold(this, type);
	if (dwTradeGold == dwCount)return TRUE;
	if (dwTradeGold < dwCount)
	{
		if (!CostMoney(type, dwCount - dwTradeGold, FALSE))
		{
			SaySystem("您没有那么多钱可以支付!");
			return FALSE;
		}
	}
	else
	{
		if (!AddMoney(type, dwTradeGold - dwCount, FALSE))
		{
			SaySystem("您身上的钱太多, 无法容纳更多的钱!");
			return FALSE;
		}
	}
	if (!m_pExchangeObj->PutMoney(this, type, dwCount))
	{
		AddMoney(type, dwCount);
		return FALSE;
	}
	return TRUE;
}

BOOL CHumanPlayer::CancelTrade()
{
	if (m_pExchangeObj == nullptr)return FALSE;
	return m_pExchangeObj->End(this, ET_CANCEL);
}

BOOL CHumanPlayer::ConfirmTrade()
{
	if (m_pExchangeObj == nullptr)return FALSE;
	return m_pExchangeObj->End(this, ET_CONFIRM);
}

BOOL CHumanPlayer::CheckTradeOtherSideOk(CHumanPlayer* p)
{
	int x = getX();
	int y = getY();
	GETNEXTPOS(x, y, GetDirection());
	int tx = p->getX();
	int ty = p->getY();
	if (x != tx || y != ty)return FALSE;
	GETNEXTPOS(tx, ty, p->GetDirection());
	if (getX() != tx || getY() != ty)return FALSE;
	return TRUE;
}

DWORD CHumanPlayer::GetMoney(money_type type)const
{
	if (type == MT_GOLD)
		return m_Humandesc.dbinfo.dwGold;
	else if (type == MT_YUANBAO)
		return m_Humandesc.dbinfo.dwYuanbao;
	return 0;
}

BOOL CHumanPlayer::AddGroupMember(const char* pszName)
{
	CHumanPlayer* pMember = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
	if (pMember == nullptr)
	{
		SaySystem("无法邀请 %s , 可能已经下线了!", pszName);
		return FALSE;
	}
	if (!pMember->IsGroupEnabled())
	{
		SaySystem("对方拒绝编组!");
		return FALSE;
	}
	if (m_pGroupObject == nullptr)
	{
		if (CGroupObjectMgr::GetInstance()->CreateGroup(this, pMember))
			return TRUE;
		return FALSE;
	}
	else
	{
		if (m_pGroupObject->IsLeader(this))
		{
			if (m_pGroupObject->IsMember(pMember))
				SaySystem("已经是队员～!");
			else
				return m_pGroupObject->AddMember(pMember);
		}
		else
			SaySystem("对不起, 您不是队长, 没有权限!");
	}
	return FALSE;
}

BOOL CHumanPlayer::RemoveGroupMember(const char* pszName)
{
	if (m_pGroupObject)
	{
		CHumanPlayer* pMember = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
		if (pMember == nullptr)
		{
			SaySystem(" %s 不在线!", pszName);
			return FALSE;
		}
		if (m_pGroupObject->IsLeader(this))
		{
			m_pGroupObject->DelMember(pMember);
			return TRUE;
		}
		else
			SaySystem("您不是队长, 没有删除队员的权限!");
	}
	else
		SaySystem("您还没有建立队伍!");
	return FALSE;
}

VOID CHumanPlayer::UpdateGroupPosition()
{
	if (m_pGroupObject != nullptr)
		m_pGroupObject->UpdateMemberPosition(this);
}

VOID CHumanPlayer::SetStartPointIndex(DWORD dwStartPointIndex)
{
	m_dwStartPointIndex = dwStartPointIndex;
	START_POINT* pStartPoint = CGameWorld::GetInstance()->GetStartPoint(dwStartPointIndex);
	if (pStartPoint == nullptr)
	{
		pStartPoint = CGameWorld::GetInstance()->GetStartPoint((DWORD)0);
		if (pStartPoint == nullptr)return;
		m_dwStartPointIndex = 0;
		o_strncpy(m_Humandesc.dbinfo.szStartPoint, pStartPoint->szName, 31);
	}
	else
		o_strncpy(m_Humandesc.dbinfo.szStartPoint, pStartPoint->szName, 31);
}

VOID CHumanPlayer::Home()
{
	int	mapid, x, y;
	if (CGameWorld::GetInstance()->GetStartPoint(m_Humandesc.dbinfo.szStartPoint, mapid, x, y))
		FlyTo(mapid, x, y);
}

ITEM* CHumanPlayer::FindBagItem(DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	return pItem;
}

BOOL CHumanPlayer::DeleteBagItem(DWORD dwMakeIndex)
{
	return m_ItemBox.RemoveItem(dwMakeIndex);
}

VOID CHumanPlayer::SendGroupMode()
{
	WORD wFlag = 0;
	if (m_Humandesc.dbinfo.dwFlag[0] & 0x80000000)
		wFlag = 1;
	SendMsg(0, SM_GROUPMODE, wFlag, 0, 0);
}

BOOL CHumanPlayer::IsGroupEnabled()const
{
	if (m_Humandesc.dbinfo.dwFlag[0] & 0x80000000)
		return TRUE;
	return FALSE;
}

VOID CHumanPlayer::SetGroupMode(BOOL bEnabled)
{
	if (bEnabled)
		m_Humandesc.dbinfo.dwFlag[0] |= 0x80000000;
	else
	{
		if (IsGroupEnabled())
			m_Humandesc.dbinfo.dwFlag[0] ^= 0x80000000;
	}
	if (!bEnabled)
	{
		if (m_pGroupObject)
			m_pGroupObject->LeaveMember(this);
	}
	SendGroupMode();
}

VOID CHumanPlayer::SayGroup(const char* pszMsg, ...)
{
	char szBuff[248];
	va_list	vl;
	va_start(vl, pszMsg);
	_vsnprintf(szBuff, sizeof(szBuff) - 1, pszMsg, vl);
	va_end(vl);
	szBuff[120] = '\0';
	if (m_pGroupObject)
		m_pGroupObject->SendMsg(nullptr, GetId(), 0x68, 0xff2f, 0/*0x38*/, 1, szBuff);
	else
		SaySystem("您还没有加入编组, 无法使用编组聊天模式!");
}

BOOL CHumanPlayer::RandomTeleport(int nMapId)
{
	if (m_pMap == nullptr)return FALSE;
	//	检查是否允许随机移动
	CLogicMap* pMap = nullptr;
	if (nMapId == -1)
	{
		if (m_pMap->IsFlagSeted(MF_NORANDOMMOVE))return FALSE;
		nMapId = GetMapId();
		pMap = m_pMap;
	}
	else
	{
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
		if (pMap == nullptr)return FALSE;
	}
	int width = pMap->GetWidth();
	int height = pMap->GetHeight();
	int px = Getrand(width);
	int py = Getrand(height);
	POINT pt;
	while (pMap->IsBlocked(px, py))
	{
		if (pMap->GetValidPoint(px, py, &pt, 1))
		{
			px = pt.x, py = pt.y;
			break;
		}
		px = Getrand(width);
		py = Getrand(height);
	}
	return FlyTo(pMap, px, py);
}

VOID CHumanPlayer::CheckAndUpgradeTitle()
{
	int iTitleIndex = 0;
	if (CTitleManager::GetInstance()->GetTitle(this, m_szCurrentTitle, iTitleIndex))
	{
		if ((iTitleIndex + 1) != m_iCurrentTitleIndex)
		{
			m_iCurrentTitleIndex = iTitleIndex + 1;
			SendTitleChanged();
		}
	}
}

VOID CHumanPlayer::SendTitleChanged()
{
	if (m_szCurrentTitle[0] != 0)
	{
		WORD wFlag = 8 | ((m_iCurrentTitleIndex + 1) << 8);
		SendAroundMsg(GetId(), 0x532c, wFlag, 0, 0, (LPVOID)m_szCurrentTitle);
		SendMsg(GetId(), 0x532c, wFlag, 0, 0, (LPVOID)m_szCurrentTitle);
	}
}

VOID CHumanPlayer::SendDuraChanged(int pos, WORD wCurDura, WORD wMaxDura)
{
	SendMsg(wCurDura, 0x282, pos, wMaxDura, 0);
}

int CHumanPlayer::GetAutoRecoverHp()
{
	int hp = GetPropValue(PI_HPRECOVER);
	if (IsStatusSet(SI_GREENPOISON))//绿毒状态
		hp = -GetStatusParam(SI_GREENPOISON);
	return hp;
}

int CHumanPlayer::GetAutoRecoverMp()
{
	int mp = GetPropValue(PI_MPRECOVER);
	if (IsStatusSet(SI_STRAWMAN)) // 诅咒术(男)红咒
		mp = -GetStatusParam(SI_STRAWMAN);
	return mp;
}

BOOL CHumanPlayer::CostMoney(money_type type, DWORD dwCount, BOOL bUpdateClient)
{
	DWORD& dwMoney = (type == MT_GOLD) ? (m_Humandesc.dbinfo.dwGold) : (m_Humandesc.dbinfo.dwYuanbao);
	if (dwMoney < dwCount)return FALSE;
	dwMoney -= dwCount;
	if (bUpdateClient)SendMoneyChanged(type);
	return TRUE;
}

BOOL CHumanPlayer::AddMoney(money_type type, DWORD dwCount, BOOL bUpdateClient)
{
	DWORD& dwMoney = (type == MT_GOLD) ? (m_Humandesc.dbinfo.dwGold) : (m_Humandesc.dbinfo.dwYuanbao);
	DWORD dwMaxGold = 0;
	if (m_Humandesc.dbinfo.bBigGold)
		dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXBIGGOLD);
	else
		dwMaxGold = CGameWorld::GetInstance()->GetVar(EVI_MAXGOLD);
	DWORD dwMaxYuanBao = CGameWorld::GetInstance()->GetVar(EVI_MAXYUANBAO);
	DWORD dwMaxMoney = (type == MT_GOLD) ? dwMaxGold : dwMaxYuanBao;
	if (dwMoney + dwCount >= dwMaxMoney)
		return FALSE;
	dwMoney += dwCount;
	if (bUpdateClient)SendMoneyChanged(type);
	return TRUE;
}

VOID CHumanPlayer::SendMoneyChanged(money_type type)
{
	WORD wMsg = (type == MT_GOLD ? SM_GOLDCHANGED : SM_SETSUPERGOLD);
	SendMsg(GetMoney(type), wMsg, 0, 0, 0);
}

BOOL CHumanPlayer::TrainMagic(USERMAGIC* pMagic, int exp)
{
	if (pMagic->magic.btLevel < 3)
	{
		if (exp == 0) exp = (Getrand(3) + 1);
		if (IsSpecialEquipmentFunctionOn(SEF_TRAINSKILL))exp *= 2;
		if (exp != 0)
		{
			pMagic->magic.dwCurTrain += exp;
			if (pMagic->magic.dwCurTrain >= pMagic->pClass->dwNeedExp[pMagic->magic.btLevel])
			{
				pMagic->magic.btLevel++;
				OnMagicLevelup(pMagic);
				pMagic->magic.dwCurTrain = 0;
			}
			SendMagicExpChg(pMagic);
			return TRUE;
		}
	}
	return FALSE;
}

VOID CHumanPlayer::DamageDura(int pos, int nDura, int nDuraRate)
{
	ITEM* pItem = m_Equipments.GetEquipment(pos);
	if (pItem == nullptr)return;
	if (pItem->wCurDura == 0)
	{
		ITEM offitem;
		if (m_Equipments.UnEquipItem(pos, offitem))
		{
			CItemManager::GetInstance()->DeleteItem(offitem.dwMakeIndex);
			SendTakeBagItem(&offitem);
			if (pos == _U_WEAPON) SendWeaponBroken(); // 发送武器破碎
		}
		SendFeatureChanged();
		UpdateProp();
		UpdateSubProp();
		SendStatusChanged();
	}
	else
	{
		int t = pItem->wCurDura / nDuraRate;
		if (pItem->wCurDura < nDura)
			pItem->wCurDura = 0;
		else
			pItem->wCurDura -= nDura;
		if (t != pItem->wCurDura / nDuraRate)
			SendDuraChanged(pos, pItem->wCurDura, pItem->wMaxDura);
		CItemManager::GetInstance()->AddItemModifyFlag(*pItem, ITEMMODIFY_DURACHANGED);
	}
}

VOID CHumanPlayer::DamageMaterialDura(int pos, int nDura)
{
	ITEM* pItem = m_Equipments.GetEquipment(pos);
	if (pItem == nullptr)return;
	CItemManager::GetInstance()->DamageDura(*pItem, nDura);
	SendDuraChanged(pos, pItem->wCurDura, pItem->wMaxDura);
	if (pItem->wCurDura == 0)
	{
		ITEM offitem;
		if (m_Equipments.UnEquipItem(pos, offitem))
		{
			CItemManager::GetInstance()->DeleteItem(offitem.dwMakeIndex);
			SendTakeBagItem(&offitem);
		}
	}
}

BOOL CHumanPlayer::AddBankItem(ITEM& item, BOOL bUpdateDB)
{
	if (m_ItemBank.AddItem(item))
	{
		if (bUpdateDB)
			CItemManager::GetInstance()->UpdateItemOwner(GetDBId(), item.dwMakeIndex, IDF_BANK, m_ItemBank.GetCount() - 1);
		return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::PutBankItem(DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBox.FindItem(dwMakeIndex);
	if (!pItem)return FALSE;
	if (CItemManager::GetInstance()->ItemLimited(*pItem, IL_NOSTORAGE))
	{
		SaySystem("该物品不能存仓库!");
		return FALSE;
	}
	if (AddBankItem(*pItem))
	{
		m_ItemBox.RemoveItem(dwMakeIndex);
		SendWeightChanged();
		return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::TakeBankItem(DWORD dwMakeIndex)
{
	ITEM* pItem = m_ItemBank.FindItem(dwMakeIndex);
	if (!pItem)return FALSE;
	if (AddBagItem(*pItem, FALSE, TRUE))
	{
		m_ItemBank.RemoveItem(dwMakeIndex);
		return TRUE;
	}
	return FALSE;
}

VOID CHumanPlayer::SendBank(DWORD dwNpcId)
{
	static thread_local ITEMCLIENT items[100];
	int count = m_ItemBank.GetClientItems(items, 100);
	SendMsg(dwNpcId, 0x2c0, 0, 0, count, (LPVOID)items, sizeof(ITEMCLIENT) * count);
}

VOID CHumanPlayer::OnAddMagic(USERMAGIC* pMagic)
{
	pMagic->nAddPower = (7 - pMagic->magic.btLevel);
	pMagic->nAddPower -= Getrand(pMagic->nAddPower);
	if (((pMagic->pClass->dwFlag & MAGICFLAG_FORCED) != 0 && (pMagic->pClass->dwFlag & MAGICFLAG_ACTIVED) == 0)) //增加属性的被动技能
	{
		if (m_iAutoMagicCount >= 8)
			PRINT(ERROR_RED, "被动技能太多～～\n");
		else
			m_pAutoMagic[m_iAutoMagicCount++] = pMagic;
	}
	else if (pMagic->pClass->dwFlag & MAGICFLAG_FORCED_EXP)
		m_pExpMagic = pMagic;
}

VOID CHumanPlayer::UpdateAutoMagic()
{
	for (int i = 0; i < m_iAutoMagicCount; i++)
	{
		if (m_pAutoMagic[i] && (--m_pAutoMagic[i]->nAddPower) <= 0)
		{
			m_pAutoMagic[i]->nAddPower = 7 - m_pAutoMagic[i]->magic.btLevel;
			m_pAutoMagic[i]->nAddPower -= Getrand(m_pAutoMagic[i]->nAddPower);
			TrainMagic(m_pAutoMagic[i]);
			if (m_pAutoMagic[i]->pClass->dwFlag & MAGICFLAG_FORCED)
			{
				switch (m_pAutoMagic[i]->magic.wId)
				{
				case 7:	// 攻杀剑法
				{
					m_pAutoMagic[i]->dwFlag |= USERMAGICFLAG_ACTIVED;
					OnAroundMsg(this, "#+PWR!", 6);
				}
				break;
				case 40: // 残影
				{
					m_pAutoMagic[i]->dwFlag |= USERMAGICFLAG_ACTIVED;
					OnAroundMsg(this, "#+VIS!", 6);
				}
				break;
				case 41: // 血影 
				{
					m_pAutoMagic[i]->dwFlag |= USERMAGICFLAG_ACTIVED;
					OnAroundMsg(this, "#+SHAD!", 7);
				}
				break;
				}
			}
		}
	}
}

VOID CHumanPlayer::OnMagicLevelup(USERMAGIC* pMagic)
{
	if (pMagic->pClass->dwFlag & MAGICFLAG_FORCED)
	{
		switch (pMagic->magic.wId)
		{
		case 74://	擒龙手
		case 7:	//	攻杀剑法
		case 4:	//	精神战法
		case 3:	//	初级剑法
		{
			RecalcHitSpeed();
			UpdateSubProp();
		}
		break;
		}
	}
}

VOID CHumanPlayer::RecalcHitSpeed()
{
	if (!m_pMagic) return;
	DecProp(PI_HITRATE, m_nRecalcHit);// 先把增加的命中减掉
	//DecProp(PI_ESCAPE, m_nRecalcSpeed);// 先把增加的躲避减掉
	m_nRecalcHit = 0;
	//m_nRecalcSpeed = 0;
	USERMAGIC* pMagic = m_pMagic;
	while (pMagic)
	{
		switch (pMagic->magic.wId)
		{
		case 3: // 初级剑法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value3;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 4: // 精神战法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value3;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 7: // 攻杀剑法
		{
			pMagic->dwFlag |= USERMAGICFLAG_ACTIVED;
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value3;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 40: // 残影刀法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value3;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 41: // 血影刀法
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value2;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		case 74: // 擒龙手
		{
			Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
			int nValue = magicskill.skills[pMagic->magic.btLevel].value1;
			AddProp(PI_HITRATE, nValue);//增加命中
			m_nRecalcHit += nValue;
		}
		break;
		}
		pMagic = pMagic->pNext;
	}
}

BOOL CHumanPlayer::MagMakeDefenceArea(int x, int y, int nRange, int nSec, int nState)
{
	BOOL bFlag = FALSE;
	int nStartX = x - nRange;
	int nStartY = y - nRange;
	int nEndX = x + nRange;
	int nEndY = y + nRange;

	for (x = nStartX; x <= nEndX; x++)
	{
		for (y = nStartY; y <= nEndY; y++)
		{
			CMapCellInfo* pInfo = m_pMap->GetMapCellInfo(x, y);
			if (pInfo == nullptr)continue;
			xListHelper<CMapObject> objlist(&pInfo->m_xObjectList);
			for (CMapObject* pObject = objlist.first(); pObject != nullptr; pObject = objlist.next())
			{
				if (pObject->GetClassType() == CLS_ALIVEOBJECT && !((CAliveObject*)pObject)->IsDeath() && IsProperFriend((CAliveObject*)pObject))
				{
					((CAliveObject*)pObject)->DefenceUp(nState, nSec);
					if (pObject->GetType() == OBJ_PLAYER)
					{
						((CHumanPlayer*)pObject)->SaySystemAttrib(CC_GREENS, nState == 22 ? "防御力增加%d秒" : "抗魔法力增加%d秒", nSec);
					}
					bFlag = TRUE;
				}
			}
		}
	}
	return bFlag;
}

VOID CHumanPlayer::ChangeAttackMode(int mode)
{
	int itemp = m_Humandesc.dbinfo.btAttackMode;
	if (mode == -1)
	{
		if (itemp >= HAM_MAX - 1)
			itemp = 0;
		else
			itemp++;
	}
	else
		itemp = mode;
	m_Humandesc.dbinfo.btAttackMode = itemp;
	SaySystemAttrib(CC_GREEN, "[%s攻击模式]", g_pAttackModeDesc[m_Humandesc.dbinfo.btAttackMode]);
}

VOID CHumanPlayer::ChangeChatChannel(e_chatchannel channel)
{
	int itemp = (int)m_ChatChannel;
	if (channel == CCH_MAX)
		itemp++;
	else
		itemp = (int)channel;
	if (itemp == CCH_GM)
	{
		if (!IsGameMaster()) itemp++;
	}
	if (itemp >= CCH_MAX) itemp = 0;
	m_ChatChannel = (e_chatchannel)itemp;
	if (m_ChatChannel == CCH_WISPER)
	{
		if (g_pChatChannelDesc[m_ChatChannel])
		{
			SaySystemAttrib(CC_GREEN, "[%s聊天频道 当前密谈对象 %s]", g_pChatChannelDesc[m_ChatChannel], m_szCurWisperTarget[0] == 0 ? "空" : m_szCurWisperTarget);
			if (m_szCurWisperTarget[0] == 0)
				SaySystemAttrib(CC_GREEN, "[在密谈频道成功使用 /角色名 密谈一次后, 该角色即被设置成密谈对象]");
		}
		else
			SaySystemAttrib(CC_GREEN, "[%s聊天频道 已关闭]", g_pChatChannelDesc[m_ChatChannel]);
	}
	else
		SaySystemAttrib(CC_GREEN, "[%s聊天频道%s]", g_pChatChannelDesc[m_ChatChannel], g_pChatChannelDesc[m_ChatChannel] ? "" : " 已关闭");
}

VOID CHumanPlayer::DisableChannel(e_chatchannel channel)
{
	if (channel >= CCH_MAX || channel < 0) channel = GetChatChannel();
	if (channel < 0 || channel >= CCH_MAX) return;
	if (IsChannelDisabled(channel))
		SaySystemAttrib(CC_GREEN, "%s 频道已经被关闭", g_pChatChannelDesc[channel]);
	else
	{
		m_bChatChannelDisabled[channel] = TRUE;
		SaySystemAttrib(CC_GREEN, "%s 频道被关闭", g_pChatChannelDesc[channel]);
	}
}

VOID CHumanPlayer::EnableChannel(e_chatchannel channel)
{
	if (channel >= CCH_MAX || channel < 0) channel = GetChatChannel();
	if (channel < 0 || channel >= CCH_MAX) return;
	if (!IsChannelDisabled(channel))
		SaySystemAttrib(CC_GREEN, "%s 频道没有被关闭", g_pChatChannelDesc[channel]);
	else
	{
		m_bChatChannelDisabled[channel] = FALSE;
		SaySystemAttrib(CC_GREEN, "%s 频道被开通", g_pChatChannelDesc[channel]);
	}
}

BOOL CHumanPlayer::IsChannelDisabled(e_chatchannel channel)const
{
	if (channel >= CCH_MAX || channel < 0) channel = GetChatChannel();
	if (channel < 0 || channel >= CCH_MAX) return FALSE;
	return m_bChatChannelDisabled[channel];
}

BOOL CHumanPlayer::IsGameMaster()
{
	return IsSystemFlagSeted(SF_GAMEMASTER);
}

VOID CHumanPlayer::ChannelSay(e_chatchannel channel, const char* pszParam, const char* pszWords/*, ...*/)
{
	if (channel < 0 || channel >= CCH_MAX) channel = GetChatChannel();
	if (channel < 0 || channel >= CCH_MAX) return;
	DWORD dwWaitTime = CGameWorld::GetInstance()->GetChannelWaitTime(channel);
	if (!m_ChatChannelTimer[channel].IsTimeOut(dwWaitTime))
	{
		DWORD dwTime = GetTimeToTime(m_ChatChannelTimer[channel].GetSavedTime(), timeGetTime());
		SaySystem("%s频道 %u 秒后才能继续发言!", g_pChatChannelDesc[channel], (dwWaitTime + 999 - dwTime) / 1000);
		return;
	}
	m_ChatChannelTimer[channel].Savetime();

	char szBuff[248];
	o_strncpy(szBuff, pszWords, 120);
	switch (channel)
	{
	case CCH_NORMAL:
	{
		xListHelper<VISIBLE_OBJECT> obj(&this->m_xVisibleObjectList);
		char szText[512];
		sprintf(szText, "%s: %s", GetName(), szBuff);
		DWORD dwParam1 = IsSystemFlagSeted(SF_FONTCHANGED) ? GetSystemFlagParam(SF_FONTCHANGED) : 0;
		dwParam1 = (dwParam1 << 8) | m_btChatColor;
		for (VISIBLE_OBJECT* pVo = obj.first(); pVo != nullptr; pVo = obj.next())
		{
			if (pVo->pObject == nullptr || pVo->pObject->GetType() != OBJ_PLAYER)
				continue;
			CHumanPlayer* pObject = (CHumanPlayer*)pVo->pObject;
			if (pObject->IsChannelDisabled(CCH_NORMAL))
				continue;
			pObject->ChannelHearDirectly(CCH_NORMAL, GetId(), szText, dwParam1);
		}
		this->ChannelHearDirectly(CCH_NORMAL, GetId(), szText, dwParam1);
	}
	break;
	case CCH_WISPER:
	{
		char* pszName = (char*)pszParam;
		if (pszName == nullptr)pszName = this->m_szCurWisperTarget;
		if (pszName[0] == 0)
		{
			SaySystem("当前密谈对象为空, 无法密谈!");
			return;
		}
		CHumanPlayer* player = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
		if (player == nullptr)
			SaySystem("%s 目前不在线, 无法密谈!", pszName);
		else
		{
			if (player == this)
				player->SaySystem("你干吗要自言自语呢?");
			else
			{
				if (player->IsChannelDisabled(CCH_WISPER))
					SaySystem("对方关闭了密谈频道, 请稍候再试!");
				else
				{
					player->ChannelHear(CCH_WISPER, GetId(), "%s: %s", GetName(), szBuff);
					SetWisperTarget(pszName);
				}
			}
		}
		return;
	}
	break;
	case CCH_CRY:
	{
		xListHelper<CMapObject> objlist;
		m_pMap->GetObjList(objlist);
		char szText[512];
		sprintf(szText, "(!)%s: %s", GetName(), szBuff);
		for (CMapObject* pObj = objlist.first(); pObj != nullptr; pObj = objlist.next())
		{
			if (pObj->GetType() == OBJ_PLAYER)
			{
				if (!((CHumanPlayer*)pObj)->IsChannelDisabled(CCH_CRY))
					((CHumanPlayer*)pObj)->ChannelHearDirectly(CCH_CRY, GetId(), szText);
			}
		}
	}
	break;
	case CCH_GM:
	{
	}
	break;
	case CCH_GROUP:
	{
		CGroupObject* pGroupObj = GetGroupObject();
		if (pGroupObj == nullptr)
			SaySystem("没有在编组内, 组队频道发言无效!");
		else
		{
			char szText[512];
			sprintf(szText, "_%s: %s", GetName(), szBuff);
			xAutoPtrArray<CHumanPlayer>& array = pGroupObj->GetMemberArray();
			CHumanPlayer* pPlayer = nullptr;
			for (UINT i = 0; i < array.GetCount(); i++)
			{
				pPlayer = array[i];
				if (pPlayer && !pPlayer->IsChannelDisabled(CCH_GROUP))
					pPlayer->ChannelHearDirectly(CCH_GROUP, GetId(), szText);
			}
		}
	}
	break;
	case CCH_GUILD:
	{
		if (m_pGuild)
		{
			if (m_pGuild->IsAllNoSay())
			{
				SaySystem("会长已经禁止全行会聊天!");
				break;
			}
			if (m_pGuild->IsNoSay(GetName()))
			{
				SaySystem("会长已经把你禁止行会聊天!");
				break;
			}
			char szText[512];
			sprintf(szText, "%s[%s]: %s", GetName(), m_pGuild->GetName(), szBuff);
			m_pGuild->SendWords(szText);
		}
		else
			SaySystem( "没有加入行会, 行会频道发言无效!" );
	}
	break;
	default:
		SaySystem("系统尚未启动!");
		break;
	}
}

BOOL CHumanPlayer::ChannelHear(e_chatchannel channel, DWORD dwParam, const char* pszWords, ...)
{
	if (channel < 0 || channel >= CCH_MAX) channel = GetChatChannel();
	if (channel < 0 || channel >= CCH_MAX) return FALSE;
	if (m_bChatChannelDisabled[channel])return FALSE; // 被关闭的频道听不到东西
	char szBuff[248];
	va_list	vl;
	va_start(vl, pszWords);
	_vsnprintf(szBuff, sizeof(szBuff) - 1, pszWords, vl);
	va_end(vl);
	szBuff[120] = '\0';
	return ChannelHearDirectly(channel, dwParam, szBuff);
}

BOOL CHumanPlayer::ChannelHearDirectly(e_chatchannel channel, DWORD dwParam, const char* pszWords, DWORD dwParam1, DWORD dwParam2)
{
	if (channel < 0 || channel >= CCH_MAX) channel = GetChatChannel();
	if (channel < 0 || channel >= CCH_MAX) return FALSE;
	WORD wMsg = g_ChatChannelMsg[channel];
	DWORD dwAttrib = g_ChatChannelAttrib[channel];
	WORD wFlag = (channel == CCH_NORMAL);
	if (wFlag)
		wFlag = (dwParam1 & 0xffff);
	SendMsg(dwParam, wMsg, LOWORD(dwAttrib), HIWORD(dwAttrib), wFlag, (LPVOID)pszWords);
	return TRUE;
}

BOOL CHumanPlayer::IsProperFriend(CAliveObject* pObject)
{
	if (pObject == nullptr || (pObject && pObject->IsDeath())) return FALSE;
	if (pObject == this) return TRUE;
	e_object_type type = pObject->GetType();
	if (type == OBJ_NPC) return FALSE;
	if (type == OBJ_MONSTER) return FALSE;
	if (type == OBJ_PET)
	{
		if (pObject->GetOwner())
			return IsProperFriend(pObject->GetOwner());
		else
			return FALSE;
	}
	else if (type == OBJ_PLAYER)
	{
		e_humanattackmode mode = GetAttackMode();
		switch (mode)
		{
		case HAM_ALL:
			return TRUE;
		case HAM_PEACE:
			return FALSE;
		case HAM_GROUP:
			return (m_pGroupObject != nullptr && m_pGroupObject->IsMember((CHumanPlayer*)pObject));
		case HAM_GUILD:
			return (m_pGuild != nullptr && m_pGuild == ((CHumanPlayer*)pObject)->GetGuild());
		break;
		case HAM_COUPLE:
			return (m_sWife[0] != 0 && strcmp(m_sWife, pObject->GetName()) == 0);
		break;
		case HAM_CRIME:
			return (!((CHumanPlayer*)pObject)->NoLawProtect());
		break;
		case HAM_MASTER:
		{
			if (m_sMaster[0] != 0 && strcmp(m_sMaster, pObject->GetName()) == 0)
				return TRUE;
			if (GetStudentCount() > 0 && IsMyStudent(pObject->GetName()) == 0)
				return TRUE;
		}
		break;
		}
	}
	return FALSE;
}

BOOL CHumanPlayer::IsProperTarget(CAliveObject* pObject)
{
	if (!pObject) return FALSE;
	if (pObject == this) return FALSE;
	e_object_type type = pObject->GetType();
	if (type == OBJ_NPC) return FALSE;
	if (type == OBJ_MONSTER) return TRUE;
	if (!pObject->IsStatusSet(SI_ITEMTRACED))
	{
		if (InSafeArea() || pObject->InSafeArea())
			return FALSE;
	}
	e_humanattackmode mode = GetAttackMode();
	if (type == OBJ_PET && mode != HAM_ALL)
	{
		if (pObject->GetOwner())
			return IsProperTarget(pObject->GetOwner());
		else
			return FALSE;
	}
	switch (mode)
	{
	case HAM_CRIME: // 善恶
	{
		if (((CHumanPlayer*)pObject)->NoLawProtect()) return TRUE;
		return FALSE;
	}
	break;
	case HAM_ALL: // 全体
		return TRUE;
	case HAM_PEACE: // 和平
		return FALSE;
	case HAM_GROUP: // 组队
	{
		if (m_pGroupObject == nullptr) return TRUE;
		return (!m_pGroupObject->IsMember((CHumanPlayer*)pObject));
	}
	break;
	case HAM_GUILD: // 行会
	{
		if (m_pGuild == nullptr) return TRUE;
		if (m_pGuild != ((CHumanPlayer*)pObject)->GetGuild()) return TRUE;
		return FALSE;
	}
	break;
	case HAM_MASTER: // 师徒
	{
		if (m_sMaster[0] != 0 && strcmp(m_sMaster, pObject->GetName()) == 0)
			return FALSE;
		if (GetStudentCount() > 0 && IsMyStudent(pObject->GetName()) == 0)
			return FALSE;
	}
	break;
	case HAM_COUPLE: // 夫妻
	{
		if (m_sWife[0] != 0 && strcmp(m_sWife, pObject->GetName()) == 0)
			return FALSE;
	}
	break;
	}
	return FALSE;
}

VOID CHumanPlayer::SetGuild(CGuildEx* pGuild, const char* pszTitle, int iLevel)
{
	m_pGuild = pGuild;
	if (pGuild == nullptr)
	{
		m_szGuildTitle[0] = 0;
		m_iGuildTitleLevel = 0;
	}
	else
	{
		if (pszTitle == nullptr)
		{
			m_pGuild->MemberLogon(this);
			return;
		}
		else
		{
			o_strncpy(m_szGuildTitle, pszTitle, 63);
			m_iGuildTitleLevel = iLevel;
			m_pGuild = pGuild;
		}
	}
	char GuildTitle[200] = "";
	if (m_pGuild)
	{
		sprintf(GuildTitle, "%s/%s", m_pGuild->GetName(), m_szGuildTitle);
		o_strncpy(m_Humandesc.dbinfo.szGuildName, m_pGuild->GetName(), 31);
	}
	else
		m_Humandesc.dbinfo.szGuildName[0] = 0;
	SendMsg(0, 0x2ee, 0, 0, 0, (LPVOID)GuildTitle);
}

static int appendstring(char* pszString, int maxlength, int curptr, const char* pszAppend)
{
	int len = (int)strlen(pszAppend);
	int leftlen = maxlength - curptr - 1;
	if (leftlen <= 0)return 0;
	if (len > leftlen)len = leftlen;
	o_strncpy(pszString + curptr, pszAppend, len);
	*(pszString + curptr + len) = 0;
	return len;
}

static int appendstringblock(char* pszString, int maxlength, int curptr, const char* pszAppend)
{
	int len = (int)strlen(pszAppend);
	int leftlen = maxlength - curptr - 1;
	if (leftlen <= 0)return 0;
	if (len > leftlen)return 0;
	o_strncpy(pszString + curptr, pszAppend, len);
	*(pszString + curptr + len) = 0;
	return len;
}

int	CHumanPlayer::GetGuildFrontPage(char* pszBuffer, int buffersize)
{
	return 0;
}

BOOL CHumanPlayer::IsMyFriend(CHumanPlayer* pPlayer)const
{
	if (pPlayer == nullptr)return FALSE;
	char* pName = (char*)pPlayer->GetName();
	for (int i = 0; i < 32; i++)
	{
		if (m_sFriends[i][0] == 0)continue;
		if (strcmp(m_sFriends[i], pName) == 0)return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::IsMyFriend(const char* pszName)const
{
	if (pszName == nullptr)return FALSE;
	for (int i = 0; i < 32; i++)
	{
		if (m_sFriends[i][0] == 0)continue;
		if (strcmp(m_sFriends[i], pszName) == 0)return TRUE;
	}
	return FALSE;
}

BOOL CHumanPlayer::PostAddFriendRequest(CHumanPlayer* poster)
{
	if (poster == nullptr)return FALSE;
	if (IsMyFriend(poster) || poster->IsMyFriend(this))
	{
		poster->SendFriendSystemError(FE_ADD_ALREADYFRIEND, GetName());
		return FALSE;
	}
	if (m_bRefuseAddFriend)
	{
		poster->SendFriendSystemError(FE_ADD_TARGETNOTALOW, GetName());
		return FALSE;
	}
	int level = CGameWorld::GetInstance()->GetVar(EVI_ADDFRIENDLEVEL);
	if (poster->GetPropValue(PI_LEVEL) < level)
	{
		poster->SendFriendSystemError(FE_ADD_LOWLEVEL, "");
		return FALSE;
	}
	else if (GetPropValue(PI_LEVEL) < level)
	{
		poster->SendFriendSystemError(FE_ADD_TARGETLOWLEVEL, GetName());
		return FALSE;
	}
	SendMsg(0, 0x1c6, 0, 0, 0, (LPVOID)poster->GetName());
	return TRUE;
}

VOID CHumanPlayer::ReplyAddFriendRequest(BOOL bAccept, const char* pszName)
{
	CHumanPlayer* pRequester = CHumanPlayerMgr::GetInstance()->FindbyName(pszName);
	if (pRequester == nullptr)return;
	if (bAccept > 1)bAccept = FALSE;
	if (bAccept)
		pRequester->AcceptAddFriendRequest(this);
	else
		pRequester->SendFriendSystemError(FE_ADD_REFUSED, GetName());
}

VOID CHumanPlayer::AcceptAddFriendRequest(CHumanPlayer* pFriend)
{
	AddFriend(pFriend);
}

BOOL CHumanPlayer::PostAddToGuildRequest(CHumanPlayer* poster)
{
	if (m_pAddToGuildRequester != nullptr)
	{
		if (m_pAddToGuildRequester == poster)
			poster->SaySystem("此人正在决定你的加入行会请求!");
		else
			poster->SaySystem("此人正在决定别人的加入行会请求!");
		return FALSE;
	}
	m_pAddToGuildRequester = poster;
	if (poster != 0)
	{
		m_dwAddToGuildRequesterInstanceKey = poster->GetInstanceKey();
		SendMsg(0, 0xc7ff, 0, 0, 0, (LPVOID)poster->GetName());
		m_AddToGuildTimer.Savetime();
	}
	return TRUE;
}

VOID CHumanPlayer::ReplyAddToGuildRequest(BOOL bAccept)
{
	if (bAccept)
	{
		if (m_pAddToGuildRequester == nullptr)
			return;
		if (m_pAddToGuildRequester->GetInstanceKey() != m_dwAddToGuildRequesterInstanceKey)
		{
			SaySystem("请求者下线, 本次加入操作作废!");
			return;
		}
		m_pAddToGuildRequester->AcceptAddToGuildRequest(this);
	}
	else
	{
		if (m_pAddToGuildRequester == nullptr)
			return;
		if (m_pAddToGuildRequester->GetInstanceKey() == m_dwAddToGuildRequesterInstanceKey)
			m_pAddToGuildRequester->SaySystem("对方拒绝你的加入行会请求");
	}
	m_pAddToGuildRequester = nullptr;
	m_dwAddToGuildRequesterInstanceKey = 0;
}

VOID CHumanPlayer::AcceptAddToGuildRequest(CHumanPlayer* pMember)
{
	CGuildEx* pGuild = pMember->GetGuild();
	if (pGuild != nullptr) return;
	pGuild = GetGuild();
	if (pGuild->AddMember(pMember))
	{
		pMember->UpdateViewName();
		SaySystemAttrib(CC_GREEN, "加入行会成功");
		pMember->SaySystemAttrib(CC_GREEN, "加入行会成功");
	}
}

ITEM* CHumanPlayer::GetWeapon()
{
	return m_Equipments.GetEquipment(_U_WEAPON);
}

ITEM* CHumanPlayer::GetDress()
{
	return m_Equipments.GetEquipment(_U_DRESS);
}

ITEM* CHumanPlayer::GetEquipment(int pos)
{
	return m_Equipments.GetEquipment(pos);
}

BOOL CHumanPlayer::RideHorse()
{
	if (m_pHorse != nullptr)
	{
		if (m_bRideHorse)
		{
			POINT	pt;
			int x = getX();
			int y = getY();
			if (m_pMap->GetValidPoint(x, y, &pt, 1))
			{
				m_bRideHorse = FALSE;
				ForceMove(pt.x, pt.y);
				m_pHorse->setXY(x, y);
				m_pHorse->SetMapId(GetMapId());
				CGameWorld::GetInstance()->AddMapObject((CMapObject*)m_pHorse);
				m_HorseTimer.Savetime();
				return TRUE;
			}
		}
		else
		{
			//马牌1 212枣红马
			//马牌2 209雪龙
			//马牌3 206乌骓
			//马牌4 187黄金宝马
			//马牌5 211迎亲宝马
			//马牌6 184高头大马
			MonsterClass* pDesc = m_pHorse->GetDesc();
			char* szHorseName = pDesc->base.szClassName;
			if (szHorseName && (szHorseName[0] & 0xff) == 206)
			{
				CGuildEx* pGuild = GetGuild();
				if (!pGuild || !pGuild->IsMaster(this))
				{
					SaySystem("乌骓只能行会会长骑!");
					return FALSE;
				}
			}
			if (szHorseName && (szHorseName[0] & 0xff) == 209)
			{
				CGuildEx* pGuild = GetGuild();
				if (!pGuild || !pGuild->IsMaster(this) || pGuild != CSandCity::GetInstance()->GetOwnerGuild())
				{
					SaySystem("雪龙只能沙城行会会长骑!");
					return FALSE;
				}
			}
			if (m_ActionType != AT_STAND || IsSystemFlagSeted(SF_TRANSFORMED))
			{
				SaySystem("您现在的状态不能骑马!");
				return FALSE;
			}
			if (m_pHorse->GetMap() != this->GetMap() ||
				DISTANCE(getX(), getY(), m_pHorse->getX(), m_pHorse->getY()) > 2)
			{
				SaySystem("您的马不在身边!");
				return FALSE;
			}
			if (!m_HorseTimer.IsTimeOut(3000))
			{
				SaySystem("三秒钟之后才能骑马!");
				return FALSE;
			}
			m_HorseTimer.Savetime();
			int x = m_pHorse->getX();
			int y = m_pHorse->getY();
			//m_pHorse->GetMap()->RemoveObject(m_pHorse);
			CGameWorld::GetInstance()->RemoveMapObject((CMapObject*)m_pHorse);
			m_bRideHorse = TRUE;
			ForceMove(x, y);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CHumanPlayer::NoLawProtect()
{
	if (this->IsStatusSet(SI_ITEMTRACED))return TRUE;
	if (m_bJustPk || GetPkValue() >= CGameWorld::GetInstance()->GetVar(EVI_REDPKPOINT))
		return TRUE;
	return FALSE;
}

VOID CHumanPlayer::CheckPk(CAliveObject* pTarget)
{
	if (pTarget == this)return;
	if (CSandCity::GetInstance()->IsWarStarted() && InWarArea())
		return;
	if (pTarget->GetType() == OBJ_PLAYER && GetGuild() && GetGuild()->IsKillGuildMember((CHumanPlayer*)pTarget))
		return;
	//战斗地图
	if (m_pMap && m_pMap->IsFlagSeted(MF_FIGHTMAP))
		return;
	//如果目标不是玩家或者受保护
	if (pTarget->GetType() != OBJ_PLAYER || ((CHumanPlayer*)pTarget)->NoLawProtect())
		return;
	if (m_bJustPk)
		m_tmrJustPk.Savetime();
	else if (GetPkValue() < CGameWorld::GetInstance()->GetVar(EVI_REDPKPOINT))
	{
		m_bJustPk = TRUE;
		m_tmrJustPk.Savetime();
		SendChangeName();
	}
}

VOID CHumanPlayer::OnEnterCityArea()
{
	if (m_pMap != nullptr)
		m_pMap->CheckEnterCity(this);
	if (GetGuild() && GetGuild()->GetKillGuildCount() > 0)
		this->ReviewAroundNameColor();
}

VOID CHumanPlayer::OnLeaveCityArea()
{
	if (GetGuild() && GetGuild()->GetKillGuildCount() > 0)
		this->ReviewAroundNameColor();
}

constexpr BYTE NCOLOR_GRAY = 61;
constexpr BYTE NCOLOR_RED = 58;
constexpr BYTE NCOLOR_YELLOW = 251;
constexpr BYTE NCOLOR_NORMAL = 255;
BYTE CHumanPlayer::GetNameColor(CMapObject* pViewer)
{
	if (CSandCity::GetInstance()->IsWarStarted() && InWarArea() && pViewer != nullptr && pViewer->GetType() == OBJ_PLAYER)
	{
		CGuildEx* pGuild = GetGuild();
		CGuildEx* pViewerGuild = ((CHumanPlayer*)pViewer)->GetGuild();
		BYTE btNormal = CGameWorld::GetInstance()->GetVar(EVI_WARNORMALCOLOR);
		BYTE btEnemy = CGameWorld::GetInstance()->GetVar(EVI_WARENEMYCOLOR);
		BYTE btAlly = CGameWorld::GetInstance()->GetVar(EVI_WARALLYCOLOR);
		if (pViewerGuild == nullptr) // 普通人看来
		{
			return btNormal;
		}
		else if (pViewerGuild == CSandCity::GetInstance()->GetOwnerGuild()) // 沙城人来看
		{
			if (pGuild == nullptr)return btNormal;
			if (pGuild->IsAttackSabuk())return btEnemy;
			if (pGuild == pViewerGuild)return btAlly;
			return btNormal;
		}
		else if (pViewerGuild->IsAttackSabuk())
		{
			if (pGuild == nullptr)return btNormal;
			if (pGuild->IsAttackSabuk())return btAlly;
			if (pGuild == CSandCity::GetInstance()->GetOwnerGuild())return btEnemy;
			return btNormal;
		}
		else
			return btNormal;
	}
	else if (GetGuild() != nullptr &&
		pViewer != nullptr && pViewer->GetType() == OBJ_PLAYER && !((CHumanPlayer*)pViewer)->InCityArea() &&
		((CHumanPlayer*)pViewer)->GetGuild() && ((CHumanPlayer*)pViewer)->GetGuild()->GetKillGuildCount() > 0)
	{
		if (((CHumanPlayer*)pViewer)->GetGuild()->IsAllyGuild(GetGuild()) || ((CHumanPlayer*)pViewer)->GetGuild() == GetGuild())
			return 0xfc;
		else if (((CHumanPlayer*)pViewer)->GetGuild()->IsKillGuild(GetGuild()))
			return 0x45;
	}
	if (IsSystemFlagSeted(SF_SPECIALNAMECOLOR))
		return (BYTE)m_SystemFlag.GetParam(SF_SPECIALNAMECOLOR);
	if (GetPkValue() >= CGameWorld::GetInstance()->GetVar(EVI_REDPKPOINT))
		return NCOLOR_RED;
	if (m_bJustPk)
		return NCOLOR_GRAY;
	if (GetPkValue() >= CGameWorld::GetInstance()->GetVar(EVI_YELLOWPKPOINT))
		return NCOLOR_YELLOW;
	return NCOLOR_NORMAL;
}

VOID CHumanPlayer::AddPkPoint(DWORD btPoint)
{
	BYTE btColor = GetNameColor(this);
	this->m_dwPkValue += btPoint;
	if (GetPkValue() >= CGameWorld::GetInstance()->GetVar(EVI_REDPKPOINT))
	{
		if (m_bJustPk)
			m_bJustPk = FALSE;
	}
	else if (!m_bJustPk)
		m_bJustPk = TRUE;
	if (m_bJustPk)
		m_tmrJustPk.Savetime();
	BYTE btColor2 = GetNameColor(this);
	if (btColor != btColor2)
		SendChangeName();
}

VOID CHumanPlayer::DecPkPoint(DWORD btPoint)
{
	BYTE btColor = GetNameColor(this);
	if (this->m_dwPkValue < btPoint)
		this->m_dwPkValue = 0;
	else
		this->m_dwPkValue -= btPoint;
	BYTE btColor2 = GetNameColor(this);
	if (btColor != btColor2)
		SendChangeName();
}

static char g_szTempString[65536];
static ITEM g_items[100];
typedef struct tagDropEquipment
{
	ITEM* pItem;
	int	pos;
}DropEquipment;

VOID CHumanPlayer::OnDeath(DWORD dwKiller)
{
	CleanPets();
	CSystemScript::GetInstance()->Execute(GetScriptTarget(), "PlayerEnv.OnDeath", FALSE); // 执行死亡触发脚本
	CAliveObject* pObj = CGameWorld::GetInstance()->GetAliveObjectById(dwKiller);
	BOOL bPersonKill = TRUE;
	if (m_pMap && m_pMap->IsFlagSeted(MF_FIGHTMAP))
		bPersonKill = FALSE;
	if (bPersonKill)
	{
		xPacket packet(g_szTempString, 65536);
		int bagitemcount = 0;
		int dropcount = 0;
		ITEM* Items[100] = { 0 };
		ITEM item;
		char szText[1024];
		DropEquipment equipments[20] = { 0 };
		int equipmentcount = 0;
		//	获取所有合法物品的指针
		for (int i = 0; i < m_ItemBox.GetCount(); i++)
		{
			Items[bagitemcount] = m_ItemBox.GetItem(i);
			if (Items[bagitemcount])
			{
				if (CItemManager::GetInstance()->ItemLimited(*Items[bagitemcount], IL_NODEADDROP))
					continue;
				bagitemcount++;
			}
		}
		for (int i = 0; i < _U_MAX; i++)
		{
			equipments[equipmentcount].pItem = m_Equipments.GetEquipment(i);
			if (equipments[equipmentcount].pItem)
			{
				if (CItemManager::GetInstance()->ItemLimited(*equipments[equipmentcount].pItem, IL_NODEADDROP))
					continue;
				equipments[equipmentcount].pos = i;
				equipmentcount++;
			}
		}
		//	获取掉罗个数
		int dropbagtotal = 0;
		int dropequipmenttotal = 0;
		if (GetPkValue() < CGameWorld::GetInstance()->GetVar(EVI_YELLOWPKPOINT))
		{
			dropbagtotal = bagitemcount / 4;
			if (equipmentcount > 0 && Getrand(10) == 0)dropequipmenttotal = 1;
		}
		else if (GetPkValue() < CGameWorld::GetInstance()->GetVar(EVI_REDPKPOINT))
		{
			dropbagtotal = bagitemcount / 4;
			if (equipmentcount > 0 && Getrand(5) == 0)dropequipmenttotal = 1;
		}
		else
		{
			dropbagtotal = bagitemcount / 4;
			if (equipmentcount > 0)dropequipmenttotal = 1;
		}
		//	乱续
		for (int i = 0; i < bagitemcount; i++)
		{
			int idx1 = Getrand(bagitemcount);
			int idx2 = Getrand(bagitemcount);
			if (idx1 == idx2)continue;
			ITEM* p = Items[idx1];
			Items[idx1] = Items[idx2];
			Items[idx2] = p;
		}
		//	对必掉和消失的下手
		for (int i = 0; i < bagitemcount; i++)
		{
			if (CItemManager::GetInstance()->ItemLimited(*Items[i], IL_DEADDELETE))
			{
				CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
				SendTakeBagItem(&item);
			}
			else if (CItemManager::GetInstance()->ItemLimited(*Items[i], IL_DEADDROP))
			{
				if (!DropItem(*Items[i]))continue;
			}
			else
				continue;
			this->m_ItemBox.RemoveItem(Items[i]->dwMakeIndex);
			o_strncpy(szText, item.baseitem.szName, 14);
			sprintf(szText + strlen(szText), "/%u/", item.dwMakeIndex);
			packet.push((LPVOID)szText, (int)strlen(szText));
			dropcount++;
			dropbagtotal--;
			Items[i] = nullptr;
		}
		//	第二轮掉落
		if (dropbagtotal > 0)
		{
			for (int i = 0; i < bagitemcount; i++)
			{
				if (Items[i] && DropItem(*Items[i]))
				{
					item = *Items[i];
					this->m_ItemBox.RemoveItem(Items[i]->dwMakeIndex);
					o_strncpy(szText, item.baseitem.szName, 14);
					sprintf(szText + strlen(szText), "/%u/", item.dwMakeIndex);
					packet.push((LPVOID)szText, (int)strlen(szText));
					dropcount++;
					Items[i] = nullptr;
					dropbagtotal--;
					if (dropbagtotal <= 0)break;
				}
			}
		}
		//	获取有装备的位置
		for (int i = 0; i < equipmentcount; i++)
		{
			int idx1 = Getrand(equipmentcount);
			int idx2 = Getrand(equipmentcount);
			if (idx1 == idx2)continue;
			DropEquipment a = equipments[idx1];
			equipments[idx1] = equipments[idx2];
			equipments[idx2] = a;
		}
		//	第一轮掉落
		int operation = 0;	//	drop
		for (int i = 0; i < equipmentcount; i++)
		{
			if (CItemManager::GetInstance()->ItemLimited(*equipments[i].pItem, IL_EQUDEADDELETE))
				operation = 1;	//	delete
			else if (CItemManager::GetInstance()->ItemLimited(*equipments[i].pItem, IL_DEADDROP))
				operation = 0;	//	drop
			else
				continue;
			if (m_Equipments.UnEquipItem(equipments[i].pos, item))
			{
				if (operation == 0)
					DropItem(item);
				else
				{
					CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
					SendTakeBagItem(&item);
				}
				o_strncpy(szText, item.baseitem.szName, 14);
				sprintf(szText + strlen(szText), "/%u/", item.dwMakeIndex);
				packet.push((LPVOID)szText, (int)strlen(szText));
				equipments[i].pItem = nullptr;
				dropcount++;
				dropequipmenttotal--;
			}
		}
		//	第二轮掉落
		if (dropequipmenttotal > 0)
		{
			for (int i = 0; i < equipmentcount; i++)
			{
				if (equipments[equipmentcount].pItem == nullptr)
					continue;
				if (m_Equipments.UnEquipItem(equipments[i].pos, item))
				{
					DropItem(item);
					o_strncpy(szText, item.baseitem.szName, 14);
					sprintf(szText + strlen(szText), "/%u/", item.dwMakeIndex);
					packet.push((LPVOID)szText, (int)strlen(szText));
					equipments[i].pItem = nullptr;
					dropcount++;
					dropequipmenttotal--;
					if (dropequipmenttotal <= 0)break;
				}
			}
		}
		//	发送消息
		if (packet.getsize() > 0)
			SendMsg(0, 0x2c5, 0, 0, dropcount, (LPVOID)packet.getbuf(), packet.getsize());
	}
	if (pObj && pObj->GetType() == OBJ_PET)
		pObj = pObj->GetOwner();
	if (pObj)
		pObj->OnKillTarget(this);
	if (pObj && pObj->GetType() == OBJ_PLAYER)
	{
		CHumanPlayer* pKiller = (CHumanPlayer*)pObj;
		if (pKiller->GetGuild() && pKiller->GetGuild()->IsKillGuildMember(this))
		{
			//GetTickCount64();
		}
		else
		{
			if (!this->NoLawProtect())
			{
				if (!(CSandCity::GetInstance()->IsWarStarted() && InWarArea()))
				{
					((CHumanPlayer*)pObj)->AddPkPoint(CGameWorld::GetInstance()->GetVar(EVI_ONCEPKPOINT));
					ITEM* pWeapon = ((CHumanPlayer*)pObj)->GetEquipment(_U_WEAPON);
					if (pWeapon && Getrand(100) < CGameWorld::GetInstance()->GetVar(EVI_PKCURSERATE))
					{
						if (pWeapon->baseitem.Ac1 > 0)
						{
							pWeapon->baseitem.Ac1--;
							((CHumanPlayer*)pObj)->SaySystem("你犯了谋杀罪, 武器被诅咒了");
							CItemManager::GetInstance()->AddItemModifyFlag(*pWeapon, ITEMMODIFY_FORGED);
							((CHumanPlayer*)pObj)->SendUpdateItem(*pWeapon);
							((CHumanPlayer*)pObj)->m_Equipments.ResetPropCache();
							((CHumanPlayer*)pObj)->UpdateProp();
							((CHumanPlayer*)pObj)->UpdateSubProp();
						}
						else if (pWeapon->baseitem.Mac1 < 10)
						{
							pWeapon->baseitem.Mac1++;
							((CHumanPlayer*)pObj)->SaySystem("你犯了谋杀罪, 武器被诅咒了");
							CItemManager::GetInstance()->AddItemModifyFlag(*pWeapon, ITEMMODIFY_FORGED);
							((CHumanPlayer*)pObj)->SendUpdateItem(*pWeapon);
							((CHumanPlayer*)pObj)->m_Equipments.ResetPropCache();
							((CHumanPlayer*)pObj)->UpdateProp();
							((CHumanPlayer*)pObj)->UpdateSubProp();
						}
						else
							((CHumanPlayer*)pObj)->SaySystem("你犯了谋杀罪");
					}
					else
						((CHumanPlayer*)pObj)->SaySystem("你犯了谋杀罪");
				}
			}
			else
				((CHumanPlayer*)pObj)->SaySystem("你实施了正当防卫");
		}
		SaySystem("您被 %s 杀死了!", pObj->GetName());
	}
}

VOID CHumanPlayer::CleanPets()
{
	if (m_iPetCount > 0)
	{
		CAliveObject* pPets[5];
		memcpy(pPets, m_pPets, sizeof(m_pPets));
		int petcount = m_iPetCount;
		for (int i = 0; i < petcount; i++)
		{
			pPets[i]->ToDeath();
		}
		m_iPetCount = 0;
	}
	if (m_pHorse)
	{
		if (m_bRideHorse)
		{
			CMonsterManagerEx::GetInstance()->DeleteMonsterImm(m_pHorse);
			m_bRideHorse = FALSE;
		}
		else
		{
			ITEM* pItem = nullptr;
			if ((pItem = GetEquipment(_U_CHARM)) && pItem->baseitem.btStdMode == 33)
			{
				CGameWorld::GetInstance()->RemoveMapObject(m_pHorse);
				CMonsterManagerEx::GetInstance()->DeleteMonster(m_pHorse);
			}
			else if ((pItem = GetEquipment(_U_POISON)) && pItem->baseitem.btStdMode == 33)
			{
				CGameWorld::GetInstance()->RemoveMapObject(m_pHorse);
				CMonsterManagerEx::GetInstance()->DeleteMonster(m_pHorse);
			}
			else
				m_pHorse->ToDeath();
		}
		m_pHorse = nullptr;
	}
	if (ISzhaohuan && m_pPet != nullptr)
	{
		//这里对豹子进行了释放操作
		CGameWorld::GetInstance()->RemoveMapObject(m_pPet);//从游戏世界移除这个对象
		CMonsterManagerEx::GetInstance()->DeleteMonster(m_pPet);//从怪物列表移除这怪物
		DelPet(m_pPet);
	}
}

VOID CHumanPlayer::SetPetTarget(CAliveObject* pObject)
{
	for (int i = 0; i < m_iPetCount; i++)
	{
		if (m_pPets[i] && !m_pPets[i]->IsDeath() && m_pPets[i]->GetTarget() == nullptr)
			m_pPets[i]->SetTarget(pObject);
	}
}

VOID CHumanPlayer::UpdateViewName()
{
	strcpy(m_szLongName, GetName());
	char szTemp[256];
	if (m_pGuild != nullptr) //	行会
	{
		const char* pGuildName = m_pGuild->GetName();
		if (m_pGuild == CSandCity::GetInstance()->GetOwnerGuild())
		{
			sprintf(szTemp, "\\%s(%s)", pGuildName, CSandCity::GetInstance()->GetName());
			strcat(m_szLongName, szTemp);
		}
		else if (CSandCity::GetInstance()->IsWarStarted() && InWarArea())
		{
			sprintf(szTemp, "\\%s(战争)", pGuildName);
			strcat(m_szLongName, szTemp);
		}
		else
		{
			CGuildMemberEx* pGuildMember = m_pGuild->GetMember(this);
			sprintf(szTemp, "\\%s(%s)", pGuildName, pGuildMember->GetGroup()->GetName());
			strcat(m_szLongName, szTemp);
		}
	}
	if (m_sMaster[0] != 0) //	师徒 
	{
		sprintf(szTemp, "\\(%s的徒弟)", m_sMaster);
		strcat(m_szLongName, szTemp);
	}
	if (m_sWife[0] != 0) //	夫妻
	{
		if (m_Humandesc.dbinfo.btSex == 0)
			sprintf(szTemp, "\\(%s的丈夫)", m_sWife);
		else
			sprintf(szTemp, "\\(%s的妻子)", m_sWife);
		strcat(m_szLongName, szTemp);
	}
	//时长封号系统
	CFengHaoGrowManager* pMgr = CFengHaoGrowManager::GetInstance();
	if (m_FenghaoInfo.btType1 > 0 ) // 普通封号
	{
		FengHaoGrowItem* pConfig = pMgr->GetItem(m_FenghaoInfo.btType1);
		sprintf(szTemp, "\\%s$%u", pConfig->szName, pConfig->btColorId);
		strcat(m_szLongName, szTemp);
	}
	SendChangeName();
}

const char* CHumanPlayer::GetScriptVarValue(const char* pszName)
{
	//<$USERNAME> 人物名称 
	GetVariableStruct var;
	if (CScriptVariableManager::GetInstance()->GetVariable(pszName, this, var))
	{
		if (var.nType == 0)
		{
			sprintf(m_szTempScriptVarValue, "%u", var.nValue);
			return m_szTempScriptVarValue;
		}
		else
			return var.pszValue;
	}
	else
		return "";
}

VOID CHumanPlayer::OnEnterSafeArea()
{
	if (CGameWorld::GetInstance()->GetVar(EVI_ENABLESAFEAREANOTICE))
		SaySystem("您进入了安全区～");
}

VOID CHumanPlayer::OnLeaveSafeArea()
{
	if (!IsDeath() && CGameWorld::GetInstance()->GetVar(EVI_ENABLESAFEAREANOTICE))
		SaySystem("您离开了安全区～");
}

VOID CHumanPlayer::OnEnterWarArea()
{
	if (CSandCity::GetInstance()->IsWarStarted())
	{
		SaySystem("您进入了攻城区域～");
		UpdateViewName();
	}
}

VOID CHumanPlayer::OnLeaveWarArea()
{
	if (!IsDeath() && CSandCity::GetInstance()->IsWarStarted())
	{
		SaySystem("您离开了攻城区域～");
		UpdateViewName();
	}
}

VOID CHumanPlayer::OnSystemFlagCleared(int index, DWORD dwParam)
{
	if (index == SF_SPECIALNAMECOLOR) // 聊天名字变色
		SendChangeName();
	else if (index == SF_BANED)
		SaySystem("您的聊天功能被恢复!");
	else if (index == SF_GODBLESS) // 附体效果
	{
		SaySystem("上古神魔的力量慢慢弱去, 你的身体恢复正常.");
		SendSpecialStatusChanged();
	}
	else if (index == SF_WINDSHIELD) // 风影盾
	{
		USERMAGIC* pMagic = GetMagic(70);
		if (pMagic == nullptr) return;
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		DecProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value3);//恢复躲避
		DecProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value4);//恢复魔法躲避
		DecProp(PI_POISONESCAPE, magicskill.skills[pMagic->magic.btLevel].value5);//恢复中毒躲避
		UpdateProp();
		UpdateSubProp();
		SendSpecialStatusChanged();
		SaySystemAttrib(CC_GREENS, "你的风影盾已经消散!");
	}
	else if (index == SF_STRONGSHIELD) // 金刚护体
	{
		USERMAGIC* pMagic = GetMagic(61);
		if (pMagic == nullptr) return;
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		DecProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value7);//恢复躲避
		DecProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value8);//恢复魔法躲避
		UpdateProp();
		UpdateSubProp();
		SendSpecialStatusChanged();
		SaySystemAttrib(CC_GREENS, "你的金刚护体已经被击碎!");

	}
	else if (index == SF_TRANSFORMED)
	{
		SendAroundMsg(GetId(), 0x532c, 0x40, 0, 0);
		SendMsg(GetId(), 0x532c, 0x40, 0, 0);
	}
}

VOID CHumanPlayer::OnSystemFlagSeted(int index, DWORD dwParam)
{
	if (index == SF_GODBLESS)
	{
		int type = m_SystemFlag.GetParam(index) & 0xffff;
		DWORD dwSecond = m_SystemFlag.GetTimeOut(index) / 1000;
		switch (type)
		{
		case 1:
		{
			SaySystem("上古的神魔赋予了你一股神奇的力量,");
			SaySystem("你获得了秒杀的神力,");
			SaySystem("神力将持续%u分钟……", dwSecond / 60);
		}
		break;
		case 2:
		{
			SaySystem("上古的神魔赋予了你一股神奇的力量,");
			SaySystem("你获得了重击的神力,");
			SaySystem("神力将持续%u分钟…… ", dwSecond / 60);
		}
		break;
		case 3:
		{
			SaySystem("来自上古的神魔赐予了你一股神奇的力量,");
			SaySystem("你获得了神御的神力,");
			SaySystem("神力将持续%u分钟……", dwSecond / 60);
		}
		break;
		case 4:
		{
			SaySystem("来自上古的神魔赐予了你一股神奇的力量");
			SaySystem("你获得了神佑的神力");
			SaySystem("神魔的眷顾可能让你在%u分钟内获得双倍的经验", dwSecond / 60);
			SaySystem("进入炼狱更有可能获得三倍……");
		}
		break;
		}
		SendSpecialStatusChanged();
	}
	else if (index == SF_WINDSHIELD)
	{
		USERMAGIC* pMagic = GetMagic(70);
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		AddProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value3);//增加躲避
		AddProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value4);//增加魔法躲避
		AddProp(PI_POISONESCAPE, magicskill.skills[pMagic->magic.btLevel].value5);//增加中毒躲避
		UpdateProp();
		UpdateSubProp();
		SendSpecialStatusChanged();
	}
	else if (index == SF_STRONGSHIELD)
	{
		USERMAGIC* pMagic = GetMagic(61);
		Magic magicskill = CMagicManager::GetInstance()->GetMagic(pMagic->magic.wId);
		AddProp(PI_ESCAPE, magicskill.skills[pMagic->magic.btLevel].value7);//增加躲避
		AddProp(PI_MAGESCAPE, magicskill.skills[pMagic->magic.btLevel].value8);//增加魔法躲避
		UpdateProp();
		UpdateSubProp();
		SendSpecialStatusChanged();
	}
	else if (index == SF_TRANSFORMED)
	{
		SendAroundMsg(GetId(), 0x532c, 0x40, (dwParam & 0xffff0000) >> 16, dwParam & 0xffff);
		SendMsg(GetId(), 0x532c, 0x40, (dwParam & 0xffff0000) >> 16, dwParam & 0xffff);
	}
}

BOOL CHumanPlayer::EscapeMap()
{
	int	mapid, x, y;
	if (m_pMap == nullptr)return FALSE;
	if (m_pMap->IsFlagSeted(MF_NOESCAPE))
		return FALSE;
	if (CGameWorld::GetInstance()->GetStartPoint(m_Humandesc.dbinfo.szStartPoint, mapid, x, y))
		AddProcess(EP_RANDOMTELEPORT, mapid, 0, 0, 0, 1);
	else
		AddProcess(EP_RANDOMTELEPORT, 0, 0, 0, 0, 1);
	return TRUE;
}

BOOL CHumanPlayer::SetPrivateShop(int iCount, PRIVATESHOPQUERY* pQuery)
{
	if (GetActionType() != AT_PRIVATESHOP && !CanDoAction(AT_PRIVATESHOP))return FALSE;
	if (iCount > 10)return FALSE;
	o_strncpy(m_szPrivateShopName, pQuery->szName, 52);
	m_iPrivateShopItemCount = 0;
	for (int i = 0; i < iCount; i++)
	{
		m_PrivateShopCache[m_iPrivateShopItemCount].pItem = m_ItemBox.FindItem(pQuery->items[i].dwMakeIndex);
		if (m_PrivateShopCache[m_iPrivateShopItemCount].pItem == nullptr)continue;
		if (CItemManager::GetInstance()->ItemLimited(*m_PrivateShopCache[m_iPrivateShopItemCount].pItem, IL_NOPRIVATESHOP))
		{
			SaySystem("摊位中有不能出售的物品!");
			return FALSE;
		}
		m_PrivateShopCache[m_iPrivateShopItemCount].dwPrice = pQuery->items[i].dwPrice;
		m_PrivateShopCache[m_iPrivateShopItemCount].pricetype = (money_type)(pQuery->items[i].wPriceType & 1);
		m_iPrivateShopItemCount++;
	}
	if (m_iPrivateShopItemCount == 0)return FALSE;
	int olddir = (int)GetDirection();
	int newdir = olddir;
	if ((newdir & 1) == 0)
	{
		if (Getrand(2) == 0)
			newdir++;
		else
			newdir--;
	}
	newdir = (newdir + 8) % 8;
	SetDirection((e_direction)newdir);
	if (!SetAction(AT_PRIVATESHOP, GetDirection(), getX(), getY(), 0xffffffff))
	{
		SetDirection((e_direction)olddir);
		return FALSE;
	}
	return TRUE;
}

BOOL CHumanPlayer::StopPrivateShop()
{
	SetAction(AT_STAND, GetDirection(), getX(), getY(), 0);
	return TRUE;
}

BOOL CHumanPlayer::SendPrivateShopPage(CHumanPlayer* pQueryer, WORD wFlag)
{
	if (pQueryer == nullptr)return FALSE;
	if (this->m_ActionType != AT_PRIVATESHOP)return FALSE;
	PRIVATESHOPSHOW	psshow;
	this->GetPrivateShopView(psshow.header);
	psshow.header.wCount = (WORD)m_iPrivateShopItemCount;
	psshow.header.w2 = wFlag;
	if (wFlag == 1)
		pQueryer->SetCurPrivateShopView(this);
	o_strncpy(psshow.header.szName, m_szPrivateShopName, 51);
	int ptr = 0;
	for (int i = 0; i < m_iPrivateShopItemCount; i++)
	{
		if (m_PrivateShopCache[i].pItem == nullptr)continue;
		psshow.items[ptr] = *(ITEMCLIENT*)m_PrivateShopCache[i].pItem;
		psshow.items[ptr].baseitem.btPriceType = (BYTE)m_PrivateShopCache[i].pricetype;
		psshow.items[ptr].baseitem.nPrice = m_PrivateShopCache[i].dwPrice;
		ptr++;
	}
	psshow.header.wCount = (WORD)ptr;
	if (ptr == 0)return FALSE;
	pQueryer->SendMsg(GetId(), 0xfca0, getX(), getY(), (WORD)GetDirection(),
		&psshow, sizeof(PRIVATESHOPHEADER) + sizeof(ITEMCLIENT) * ptr);
	return TRUE;
}

static thread_local char g_szCodedMsgBuffer[65536];
VOID CHumanPlayer::UpdatePrivateShopToAround()
{
	DWORD dwParam[2] = { 0, 0 };
	SendMsg(GetId(), 0x80d7, getX(), getY(), (WORD)GetDirection(), (LPVOID)dwParam, sizeof(dwParam));

	if (m_xVisibleObjectList.getCount() == 0) return;

	PRIVATESHOPSHOW	psshow;
	psshow.header.wCount = (WORD)m_iPrivateShopItemCount;
	psshow.header.w2 = 2;
	o_strncpy(psshow.header.szName, m_szPrivateShopName, 51);

	int ptr = 0;
	for (int i = 0; i < m_iPrivateShopItemCount; i++)
	{
		if (m_PrivateShopCache[i].pItem == nullptr)continue;
		psshow.items[ptr] = *(ITEMCLIENT*)m_PrivateShopCache[i].pItem;
		psshow.items[ptr].baseitem.btPriceType = (BYTE)m_PrivateShopCache[i].pricetype;
		psshow.items[ptr].baseitem.nPrice = m_PrivateShopCache[i].dwPrice;
		ptr++;
	}
	psshow.header.wCount = (WORD)ptr;

	if (ptr == 0) return;

	int size = 0;
	SmartEncodeMessage(g_szCodedMsgBuffer, size, GetId(), 0xfca0, getX(), getY(), (WORD)GetDirection(),
		&psshow, sizeof(PRIVATESHOPHEADER) + sizeof(ITEMCLIENT) * ptr);

	xListHost<VISIBLE_OBJECT>::xListNode* pNode = m_xVisibleObjectList.getHead();
	while (pNode)
	{
		CMapObject* pObject = pNode->getObject()->pObject;
		if (pObject && pObject->GetType() == OBJ_PLAYER && ((CHumanPlayer*)pObject)->GetCurPrivateShopView() == this)
			pObject->OnAroundMsg(this, g_szCodedMsgBuffer, size);
		pNode = pNode->getNext();
	}
}

BOOL CHumanPlayer::BuyPrivateShopItem(CHumanPlayer* pBuyer, DWORD dwItemId, const char* pszName)
{
	CItemBox& box = pBuyer->GetBag();
	if (box.GetFree() <= 0)
	{
		pBuyer->SaySystem("背包满了, 无法购买物品!");
		return FALSE;
	}

	for (int i = 0; i < m_iPrivateShopItemCount; i++)
	{
		if (m_PrivateShopCache[i].pItem)
		{
			if (m_PrivateShopCache[i].pItem->dwMakeIndex == dwItemId)
			{
				DWORD dwMoney = pBuyer->GetMoney(m_PrivateShopCache[i].pricetype);
				if (dwMoney < m_PrivateShopCache[i].dwPrice)
				{
					pBuyer->SaySystem("没有足够的金钱来购买此物品!");
					return FALSE;
				}
				if (!TestAddMoney(m_PrivateShopCache[i].pricetype, m_PrivateShopCache[i].dwPrice))
				{
					char szName[20];
					o_strncpy(szName, m_PrivateShopCache[i].pItem->baseitem.szName, 16);
					SaySystem("%s要购买您的%s物品, 但是你已经无法再携带更多的钱了.",
						pBuyer->GetName(), szName);
					pBuyer->SaySystem("卖家不能再携带更多的钱了!");
					return FALSE;
				}
				if (pBuyer->AddBagItem(*m_PrivateShopCache[i].pItem, FALSE, TRUE))
				{
					pBuyer->CostMoney(m_PrivateShopCache[i].pricetype, m_PrivateShopCache[i].dwPrice);
					AddMoney(m_PrivateShopCache[i].pricetype, m_PrivateShopCache[i].dwPrice);
					m_iPrivateShopItemCount--;
					ITEM item = *m_PrivateShopCache[i].pItem;
					if (m_iPrivateShopItemCount > 0)
					{
						for (int j = i; j < m_iPrivateShopItemCount; j++)
						{
							m_PrivateShopCache[j] = m_PrivateShopCache[j + 1];
						}
					}
					m_ItemBox.RemoveItem(dwItemId);
					SendTakeBagItem(&item);
					SendWeightChanged();
					return TRUE;
				}
				else
					return FALSE;
			}
		}
	}
	return FALSE;
}

BOOL CHumanPlayer::CanDoAction(actiontype action)
{
	if (m_bRideHorse)
	{
		if (action != AT_RUN && action != AT_WALK) // 骑马的时候不能作其他动作, 只能走路～!
			return FALSE;
	}
	else if (IsSystemFlagSeted(SF_TRANSFORMED))
	{
		if (action != AT_WALK && action != AT_PRIVATESHOP)
			return FALSE;
	}
	if (action == AT_SPECIALHIT)
	{
		return m_tmrSpecialAttackSkill.IsTimeOut(g_dwActionDelay[AT_ATTACK] - 80 * GetPropValue(PI_ATTACKSPEED));
	}
	return CAliveObject::CanDoAction(action);
}

BOOL CHumanPlayer::CanBearItem(ITEM& item)
{
	if (m_Humandesc.dbinfo.weight < m_ItemBox.CalcWeight())
		return FALSE;
	if (m_Humandesc.dbinfo.weight - m_ItemBox.CalcWeight() < item.baseitem.btWeight)
		return FALSE;
	return TRUE;
}

BOOL CHumanPlayer::SetMagicLevel(USERMAGIC* pMagic, int level)
{
	if (level < 0) level = 0;
	if (level > 3) level = 2;
	pMagic->magic.btLevel = level;
	pMagic->magic.dwCurTrain = 0;
	SendMagicExpChg(pMagic);
	return TRUE;
}

BOOL CHumanPlayer::SetMagicLevel(const char* pszName, int level)
{
	USERMAGIC* pMagic = this->GetMagicByName(pszName);
	if (pMagic == nullptr) return FALSE;
	if (level < 0) level = 0;
	if (level > 3) level = 2;
	pMagic->magic.btLevel = level;
	pMagic->magic.dwCurTrain = 0;
	SendMagicExpChg(pMagic);
	return TRUE;
}

VOID CHumanPlayer::SendMagicExpChg(USERMAGIC* pMagic)
{
	SendMsg(pMagic->magic.wId, SM_SPELLEXPCHG, pMagic->magic.btLevel, pMagic->magic.dwCurTrain & 0xffff,
		(pMagic->magic.dwCurTrain & 0xffff0000) >> 16, nullptr);
}

VOID CHumanPlayer::SendZhenBao(DWORD dwZhenBaoExp, DWORD dwZhenBaoExpMax, DWORD dwZhenBaoStar)
{
	xPacket packet(g_szTempString, 65536);
	packet.push("ZhenBaoExp");
	packet.push(1);
	packet.push((LPVOID)&dwZhenBaoExp, 8);
	SendMsg(GetId(), 0x9af, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());

	if (dwZhenBaoExpMax != -1 && dwZhenBaoExpMax != m_dwZhenBaoExpMax)
	{
		m_dwZhenBaoExpMax = dwZhenBaoExpMax;
		packet.clear();
		packet.push("ZhenBaoExpMax");
		packet.push(1);
		packet.push((LPVOID)&dwZhenBaoExpMax, 8);
		SendMsg(GetId(), 0x9af, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	}
	if (dwZhenBaoStar != -1 && dwZhenBaoStar != m_dwZhenBaoStar)
	{
		m_dwZhenBaoStar = dwZhenBaoStar;
		packet.clear();
		packet.push("ZhenBaoStar");
		packet.push(1);
		packet.push((LPVOID)&dwZhenBaoStar, 8);
		SendMsg(GetId(), 0x9af, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
	}
}

VOID CHumanPlayer::SendJingLiZhi(DWORD wStamina)
{
	m_wStamina = wStamina;
	xPacket packet(g_szTempString, 65536);
	packet.push("jinglizhi");
	packet.push(1);
	packet.push((LPVOID)&wStamina, 8);
	SendMsg(GetId(), 0x9af, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CHumanPlayer::SaveVars()
{
	char szPath[1024];
	_makepath(szPath, nullptr, ".\\Data\\Charvars", GetName(), "txt");
	if (GetScriptTarget())
		GetScriptTarget()->SaveVars(szPath);
}

VOID CHumanPlayer::LoadVars()
{
	char szPath[1024];
	_makepath(szPath, nullptr, ".\\Data\\Charvars", GetName(), "txt");
	if (GetScriptTarget())
		GetScriptTarget()->LoadVars(szPath);
}