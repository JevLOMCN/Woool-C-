#include "StdAfx.h"
#include ".\scriptfunction.h"
#include ".\cmdproc.h"
#include ".\scriptview.h"
#include ".\scripttarget.h"
#include ".\scriptshell.h"
#include ".\Humanplayer.h"
#include ".\ScriptElement.h"
#include ".\ScriptObject.h"
#include "sandcity.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "StringListManager.h"
#include "guildex.h"
#include "LogicMapMgr.h"
#include "logicmap.h"
#include "monstergenmanager.h"
#include "GroupObject.h"
#include "topmanager.h"
#include "gameworld.h"
#include "humanplayermgr.h"
#include "npcManager.h"
#include "taskmanager.h"
#include "autoscriptmanager.h"
#include "scripttimermanager.h"
#include "monstermanagerex.h"
#include "systemscript.h"

extern BOOL	g_bDebugScript;
CFunctionRegisterAgent::CFunctionRegisterAgent(const char* pszName, fnCommandProc proc)
{
	if (!CCommandManager::GetInstance()->AddCommand(pszName, proc))
		PRINT(ERROR_RED, "注册命令 %s 的时候出现问题!\n", pszName);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	检查命令, 支持返回真
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TRUE) {
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	随机是否为真
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RANDOM){
	if (nParam == 0)
		return (DWORD)Getrand();
	else if (nParam == 1)
		return (DWORD)Getrand(Params[0].nParam);
	else /*if( nParam == 2 )*/
		return (DWORD)GetRangeRand(Params[0].nParam, Params[1].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：	给玩家物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GIVE){
//dwResult = 0; 
if (nParam == 1)
	return (DWORD)pPlayer->CreateBagItem(Params[0].pszParam);
else if (nParam == 2)
{
	UINT count = Params[1].nParam;
	DWORD dwResult = 0;
	for (UINT i = 0; i < count; i++)
	{
		if (pPlayer->CreateBagItem(Params[0].pszParam))
			dwResult++;
	}
	return dwResult;
}
else if (nParam == 3)
{
	UINT count = Params[1].nParam;
	BOOL IsBing = Params[2].nParam;
	DWORD dwResult = 0;
	for (UINT i = 0; i < count; i++)
	{
		if (pPlayer->CreateBagItem(Params[0].pszParam, IsBing))
			dwResult++;
	}
	return dwResult;
}
return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查玩家变量值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECK){
	if (nParam < 1)return 0;
	int value = pPlayer->GetVarValue(Params[0].pszParam);
	if (value == -1)
		value = StringToInteger(Params[0].pszParam);
	if (nParam > 1)
	{
		int compvalue = (int)(Params[1].nParam);
		return (value == compvalue);
	}
	else
		return (value > 0);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查玩家变量值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKEX){
	if (nParam != 3) return 0;
	int value = pPlayer->GetVarValue(Params[0].pszParam);
	if (value == -1)
		value = StringToInteger(Params[0].pszParam);
	int compvalue = (int)(Params[2].nParam);
	return CompareValue(value, Params[1].pszParam, compvalue);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：判断时间是否是之前时间
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(BEFORE){
	if (nParam == 0)return 0;
	char* pDateTime = Params[0].pszParam;
	CSystemTime timenow;
	CSystemTime timetest(pDateTime);
	if (timetest < timenow)
		return TRUE;
	else
		return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：判断时间是否是之后时间
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(AFTER){
	if (nParam == 0)return 0;
	char* pDateTime = Params[0].pszParam;
	CSystemTime timenow;
	CSystemTime timetest(pDateTime);
	if (timetest > timenow)
		return TRUE;
	else
		return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送当前时间消息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(NOWDATE) {
	CSystemTime timenow;
	pPlayer->SaySystem("%u-%u-%u,%u:%u:%u", timenow.GetYear(), timenow.GetMonth(), timenow.GetDay(), timenow.GetHour(), timenow.GetMinute(), timenow.GetSecond());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：延时调用脚本
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DELAY){
	if (nParam < 2)return 0;
	int seconds = Params[0].nParam;
	char szPage[256];
	char* p = strchr(Params[1].pszParam, '.');
	if (p == nullptr && pShell->GetTitleId() == (UINT)-1)
		sprintf(szPage, "@%s.%s", pTarget->getCurScriptObject()->getName(), Params[1].pszParam);
	else
		o_strncpy(szPage, Params[1].pszParam, 250);
	pPlayer->AddProcess(EP_OPENSCRIPTPAGE, pShell->GetTitleId(), 0, 0, 0, seconds * 1000, 1, szPage);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：跳转调用脚本
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GOTO){
	if (pTarget->getCurScriptObject() == nullptr)return FALSE;
	CSe_Page* pPage = pTarget->getCurScriptObject()->GetPage(Params[0].pszParam);
	if (pPage)
	{
		char szPage[256];
		if (pShell->GetTitleId() == (UINT)-1)
		{
			char* p = strchr(Params[0].pszParam, '.');
			if (p == nullptr)
				sprintf(szPage, "@%s.%s", pTarget->getCurScriptObject()->getName(), Params[0].pszParam);
			else
				o_strncpy(szPage, Params[0].pszParam, 250);
		}
		else
			o_strncpy(szPage, Params[0].pszParam, 250);
		return pPlayer->AddProcess(EP_OPENSCRIPTPAGE, pShell->GetTitleId(), 0, 0, 0, Params[1].nParam * 1000, 1, szPage);
	}
	else
		return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：关闭PAGE
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLOSE){
	pShell->setExecuteResult(ER_CLOSE);
	bContinue = FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：地图切换或者移动
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MOVE){
	if (nParam == 0)
		return pPlayer->RandomTeleport();
	else if (nParam == 1)
		return pPlayer->RandomTeleport(Params[0].nParam);
	else if (nParam == 2)
		return pPlayer->FlyTo(pPlayer->GetMap(), Params[0].nParam, Params[1].nParam);
	else
		return pPlayer->FlyTo(Params[0].nParam, Params[1].nParam, Params[2].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查玩家是否在账号列表中
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKACCOUNTLIST){
	return CStringListManager::GetInstance()->InStringList(Params[0].pszParam, pPlayer->GetAccount());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查玩家是否在名字列表中
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKCHARNAMELIST){
	return CStringListManager::GetInstance()->InStringList(Params[0].pszParam, pPlayer->GetName());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查背包物品是否有指定数量物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKBAGITEM){
	if (nParam == 0) return 0;
	int count = 1;
	char* pszItemName = "";
	pszItemName = Params[0].pszParam;
	if (nParam > 1)
		count = Params[1].nParam;
	CItemBox& bag = pPlayer->GetBag();
	return (count <= bag.GetItemCount(pszItemName));
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：拿走玩家背包中的指定数量物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TAKEBAGITEM){
	if (nParam == 0)return 0;
	UINT count = 1;
	if (nParam > 1)
	count = Params[1].nParam == 0 ? 1 : Params[1].nParam;
	CItemBox& bag = pPlayer->GetBag();
	ITEM items[BIGBAG_SLOT];
	if (bag.TakeItemCount(Params[0].pszParam, count, items))
	{
		for (UINT i = 0; i < count; i++)
		{
			if (CItemManager::GetInstance()->ItemLimited(items[i], IL_TRACEDITEM) &&
			pPlayer->GetStatusParam(SI_ITEMTRACED) == items[i].dwMakeIndex)
			{
				pPlayer->ClrStatus(SI_ITEMTRACED);
			}
			CItemManager::GetInstance()->DeleteItem(items[i].dwMakeIndex);
			pPlayer->SendTakeBagItem(&items[i]);
		}
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：拿走玩家背包中所有物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TAKEITEMALL){
	if (nParam == 0)return 0;
	CItemBox& bag = pPlayer->GetBag();
	UINT count = bag.GetItemCount(Params[0].pszParam);
	if (count == 0)return 0;
	ITEM items[BIGBAG_SLOT];
	if (bag.TakeItemCount(Params[0].pszParam, count, items))
	{
		for (UINT i = 0; i < count; i++)
		{
			if (CItemManager::GetInstance()->ItemLimited(items[i], IL_TRACEDITEM) &&
			pPlayer->GetStatusParam(SI_ITEMTRACED) == items[i].dwMakeIndex)
			{
				pPlayer->ClrStatus(SI_ITEMTRACED);
			}
			CItemManager::GetInstance()->DeleteItem(items[i].dwMakeIndex);
			pPlayer->SendTakeBagItem(&items[i]);
		}
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置玩家自定义标识
//		注释：0-63 共64个标识
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETFLAG){
	pPlayer->setSelfFlag(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：删除玩家自定义标识
//		注释：0-63 共64个标识
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLRFLAG){
	pPlayer->clrSelfFlag(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查玩家自定义标识
//		注释：0-63 共64个标识
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKFLAG){
	return pPlayer->getSelfFlag(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：地图随机传送
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MAPMOVE){
	if (nParam == 0)
		return pPlayer->RandomTeleport();
	if (nParam == 1)
		return pPlayer->RandomTeleport(Params[0].nParam);
	if (nParam == 2)
		return pPlayer->FlyTo(pPlayer->GetMap(), Params[0].nParam, Params[1].nParam);
	if (nParam == 3)
		return pPlayer->FlyTo(Params[0].nParam, Params[1].nParam, Params[2].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：指定玩家传送
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ARROWMOVE) {
	if (nParam < 4) return FALSE;
	CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	return p->FlyTo(Params[1].nParam, Params[2].nParam, Params[3].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：消费金币
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TAKEGOLD){
	return pPlayer->CostMoney(MT_GOLD, Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：消费元宝
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TAKEYUANBAO){
	return pPlayer->CostMoney(MT_YUANBAO, Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加金币
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GIVEGOLD){
	if (nParam == 1)
		return pPlayer->AddMoney(MT_GOLD, Params[0].nParam);
	else 
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p) return p->AddMoney(MT_GOLD, Params[1].nParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：设置为大金币包
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETBIGGOLD) {
	if (nParam == 1)
		pPlayer->SetBigGold(Params[0].nParam);
	else
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p) p->SetBigGold(Params[1].nParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：曾加元宝
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GIVEYUANBAO){
	if (nParam == 1)
		return pPlayer->AddMoney(MT_YUANBAO, Params[0].nParam);
	else 
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p) return p->AddMoney(MT_YUANBAO, Params[1].nParam);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否是沙城城主
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISSABUKOWNER){
	if (pPlayer->GetGuild() && pPlayer->GetGuild() == CSandCity::GetInstance()->GetOwnerGuild())
	{
		if (pPlayer->GetGuild()->IsMaster(pPlayer))
			return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否玩家是沙城成员
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISSABUKMEMBER){
	if (pPlayer->GetGuild() && pPlayer->GetGuild() == CSandCity::GetInstance()->GetOwnerGuild())
		return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：攻城申请
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REQUESTATTACKSABUK){
	if (!CSandCity::GetInstance()->AddAttackRequest(pPlayer->GetGuild()))
	{
		pPlayer->SaySystem(CSandCity::GetInstance()->getErrorMsg());
		return FALSE;
	}
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：改变玩家名字颜色
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGENAMECOLOR){
	pPlayer->SetSystemFlag(SF_SPECIALNAMECOLOR, TRUE, (BYTE)Params[0].nParam, Params[1].nParam * 1000);
	pPlayer->SendChangeName();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：改变玩家衣服颜色
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEDRESSCOLOR){
	ITEM* pDress = pPlayer->GetDress();
	if (pDress)
	{
		pDress->baseitem.wFeature = (pDress->baseitem.wFeature & 0xfff0) | (Params[0].nParam & 0xf);
		CItemManager::GetInstance()->AddItemModifyFlag(*pDress, ITEMMODIFY_COLORCHANGED);
		pPlayer->SendFeatureChanged();
		return TRUE;
	}
	else
		return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：修理沙城大门
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REPAIRMAINDOOR){
	CSandCity::GetInstance()->RepairGate();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：修理沙城皇宫墙
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REPAIRWALL){
	CSandCity::GetInstance()->RepairWall(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否沙城战开始
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISSABUKWARSTARTED){
	return CSandCity::GetInstance()->IsWarStarted();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查指定位置是否穿戴某装备
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKEQUIPMENT){
	if (nParam == 0)return 0;
	int pos = Params[0].nParam;
	ITEM* pItem = pPlayer->GetEquipment(pos);
	if (pItem == 0)return FALSE;
	if (nParam > 1)
	{
		char szName[20];
		o_strncpy(szName, pItem->baseitem.szName, 14);
		if (strcmp(szName, Params[1].pszParam) == 0)
			return TRUE;
		return FALSE;
	}
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查所有位置是否穿戴某装备
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKEQUIPMENTEX){
	if (nParam == 0)return 0;
	ITEM item;
	if (!CItemManager::GetInstance()->MakeupTemplateItem(Params[0].pszParam, item))return FALSE;
	int out = 0;
	return pPlayer->CheckEquipment(-1, item.baseitem.btStdMode, item.baseitem.wImageIndex, out);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：修理穿戴装备
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REPAIREQUIPMENT) {
	if (nParam == 0) return FALSE;
	if (nParam == 1)
		return pPlayer->RepairEquipment(Params[0].nParam);
	if (nParam == 2)
		return pPlayer->RepairEquipment(Params[0].nParam, Params[1].nParam);
	return FALSE;
}END_SCRIPT_FUNCTION


//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否玩家行会是进攻沙城的行会
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISATTACKSABUKGUILD){
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (pGuild && pGuild->IsAttackSabuk())
		return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否玩家行会的成员
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISGUILDMEMBER) {
	if (nParam == 0) return FALSE;
	CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (p)
	{
		if (pPlayer->GetGuild() && pPlayer->GetGuild()->IsMember(p))
			return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否玩家是行会掌门人
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISGUILDMASTER){
	if (pPlayer->GetGuild() && pPlayer->GetGuild()->IsMaster(pPlayer))
		return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否玩家有行会
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HASGUILD){
	if (pPlayer->GetGuild())
		return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加和减少玩家对行会的贡献值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(PGUILDDEVOTED) {
	if (nParam < 2) return FALSE;
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (pGuild)
	{
		CGuildMemberEx* pGuildMember = pGuild->GetMember(pPlayer);
		if (pGuildMember)
		{
			switch (Params[0].pszParam[0])
			{
			case '+':
				pGuildMember->AddExp(Params[1].nParam);
			case '-':
				pGuildMember->DecExp(Params[1].nParam);
			case '=':
				pGuildMember->SetExp(Params[1].nParam);
			default:
				return FALSE;
			}
			return TRUE;
		}
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加和减少玩家在行会拥有的能量值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(PGUILDPOWER) {
	if (nParam < 2) return FALSE;
	CGuildEx* pGuild = pPlayer->GetGuild();
	if (pGuild)
	{
		CGuildMemberEx* pGuildMember = pGuild->GetMember(pPlayer);
		if (pGuildMember)
		{
			switch (Params[0].pszParam[0])
			{
			case '+':
				pGuildMember->AddPower(Params[1].nParam);
			case '-':
				pGuildMember->DecPower(Params[1].nParam);
			case '=':
				pGuildMember->SetPower(Params[1].nParam);
			default:
				return FALSE;
			}
			return TRUE;
		}
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否玩家有豹子
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISHASBAOZI){
	return pPlayer->IsHasPet();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加经验
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GIVEEXP){
	if (nParam == 1)
	{
		pPlayer->WinExp(Params[0].nParam, TRUE);
		return TRUE;
	}
	if (nParam == 2)
	{
		pPlayer->WinExp(Params[0].nParam * Params[1].nParam, TRUE);
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加贡献值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GIVECREDIT){
	if (nParam == 1)
	{
		pPlayer->GiveCredit(Params[0].nParam);
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：减少贡献值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TAKECREDIT){
	return pPlayer->TakeCredit(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否在指定小时段内
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HOUR){
	SYSTEMTIME st;
	GetLocalTime(&st);
	WORD wStart = Params[0].nParam;
	WORD wEnd = Params[1].nParam;
	if (st.wHour >= wStart && st.wHour <= wEnd)return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否在指定分钟段内
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MINUTE){
	SYSTEMTIME st;
	GetLocalTime(&st);
	WORD wStart = Params[0].nParam;
	WORD wEnd = Params[1].nParam;
	if (st.wMinute >= wStart && st.wMinute <= wEnd)return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否在指定一周段内
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DAYOFWEEK){
	SYSTEMTIME st;
	GetLocalTime(&st);
	WORD wStart = Params[0].nParam;
	if (wStart == st.wDayOfWeek) return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表中检查IP
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKIPLIST){
	return CStringListManager::GetInstance()->InStringList(Params[0].pszParam, pPlayer->GetClientObject() ? pPlayer->GetClientObject()->getAddress() : "0.0.0.0");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表中添加玩家名字
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDNAMELIST){
	CStringListManager::GetInstance()->AddToStringList(Params[0].pszParam, pPlayer->GetName());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表中添加账号
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDACCOUNTLIST){
	CStringListManager::GetInstance()->AddToStringList(Params[0].pszParam, pPlayer->GetAccount());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表中添加IP
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDIPLIST){
	CStringListManager::GetInstance()->AddToStringList(Params[0].pszParam, pPlayer->GetClientObject() ? pPlayer->GetClientObject()->getAddress() : "0.0.0.0");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表中删除玩家名字
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DELNAMELIST){
	CStringListManager::GetInstance()->DelFromStringList(Params[0].pszParam, pPlayer->GetName());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表中删除账号
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DELACCOUNTLIST){
	CStringListManager::GetInstance()->DelFromStringList(Params[0].pszParam, pPlayer->GetAccount());
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表中删除IP
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DELIPLIST){
	CStringListManager::GetInstance()->DelFromStringList(Params[0].pszParam, pPlayer->GetClientObject() ? pPlayer->GetClientObject()->getAddress() : "0.0.0.0");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：提升玩家多少等级
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(LEVELUP){
	if (nParam == 0) return FALSE;
	UINT level = Params[0].nParam + pPlayer->GetPropValue(PI_LEVEL);
	pPlayer->LevelUp(level);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加护身-4种类型：神佑、神御、重击、秒杀
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDGODBLESS){
	DWORD dwType = Params[0].nParam;
	DWORD dwRate = Params[1].nParam;
	DWORD dwTime = Params[2].nParam;
	DWORD dwParam = (dwType & 0xffff) | (dwRate << 16);
	pPlayer->SetSystemFlag(SF_GODBLESS, TRUE, dwParam, dwTime * 1000);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：清理护身-4种类型：神佑、神御、重击、秒杀
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLRGODBLESS){
	pPlayer->SetSystemFlag(SF_GODBLESS, FALSE);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查玩家是否在指定地图
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKMAP) {
	int mapid = Params[0].nParam;
	return (pPlayer->GetMapId() == mapid);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查指定地图玩家数量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKMAPHUM){
	int mapid = Params[0].nParam;
	int count = Params[2].nParam;
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
	if (pMap == nullptr)return FALSE;
	int value = pMap->GetObjectCount(OBJ_PLAYER);
	return CompareValue(value, Params[1].pszParam, count);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查指定地图怪物数量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKMAPMON){
	if (nParam < 3)return 0;
	int mapid = Params[0].nParam;
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
	if (pMap == nullptr)return FALSE;
	int count = 0;
	int value = 0;
	char* sign;
	if (nParam > 3)
	{
		count = Params[3].nParam;
		sign = Params[2].pszParam;
		value = pMap->CountAllMonsters(Params[1].pszParam);
	}
	else
	{
		count = Params[2].nParam;
		sign = Params[1].pszParam;
		value = pMap->CountAllMonsters();
	}
	return CompareValue(value, sign, count);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：刷怪
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MONGEN){
    if (nParam < 1)return 0;
	char* pMonsterName = Params[0].pszParam;
	int mapid = pPlayer->GetMapId();
	int x = pPlayer->getX() + rand() % 10 - 5;
	int y = pPlayer->getY() + rand() % 10 - 5;
	int r = 0;
	int c = 1;
	if (nParam == 2)
	{
		mapid = Params[1].nParam;
		CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
		auto randomCoord = pMap->GetRandomCoordinate();
		x = randomCoord.first;
		y = randomCoord.second;
	}
	else if (nParam == 4) 
	{
		mapid = Params[1].nParam;
		x = Params[2].nParam;
		y = Params[3].nParam;
	}
	else if (nParam == 5)
	{
		mapid = Params[1].nParam;
		x = Params[2].nParam;
		y = Params[3].nParam;
		r = Params[4].nParam;
	}
	else if (nParam == 6)
	{
		mapid = Params[1].nParam;
		x = Params[2].nParam;
		y = Params[3].nParam;
		r = Params[4].nParam;
		c = Params[5].nParam;
	}
	CMonsterGenManager* monGenManager = CMonsterGenManager::GetInstance();
	char szTemp[128];
	sprintf_s(szTemp, sizeof(szTemp), "%s,%u,%u,%u,%u,%u,%u,%s", pMonsterName, mapid, x, y, r, c, 0, Params[6].pszParam);
	MONSTERGEN* monGen = monGenManager->AddMonsterGen(szTemp); // 这个需要一个常驻刷怪配置, 因为特殊刷怪需要管理怪列表指针.
	if (r <= 1000)
	{
		static CRandomRangeSpawnStrategy strategy;
		return monGenManager->UpdateGenEx(monGen, &strategy, c, TRUE);
	}
	else
	{ 
		static CSpecialFormationSpawnStrategy strategy;
		return monGenManager->UpdateGenEx(monGen, &strategy, c, TRUE);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否是玩家小组成员
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISGROUPMEMBER){
	if (pPlayer->GetGroupObject() == nullptr)return FALSE;
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否是小组组长
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISGROUPLEADER){
	CGroupObject* pGroupObject = pPlayer->GetGroupObject();
	if (pGroupObject && pGroupObject->IsLeader(pPlayer))
		return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：滚动消息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SCROLLMSG){
	if (nParam == 1)
	{
		char szText[1024];
		ProcFmtText(Params[0].pszParam, szText, 1024, pTarget);
		CGameWorld::GetInstance()->AddGlobeProcess(EP_SCROLLTEXT, 0, 0, 0, 0, 50, 1, szText);
	}
	else if (nParam > 1)
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p)
		{
			char szText[1024];
			ProcFmtText(Params[1].pszParam, szText, 1024, pTarget);
			p->SendScrollText(szText);
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：屏幕中大字系统消息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SYSTEM10MSG) {
	if (nParam == 1)
	{
		char szText[1024];
		ProcFmtText(Params[0].pszParam, szText, 1024, pTarget);
		CGameWorld::GetInstance()->PostSystem10Message(szText);
	}
	else if (nParam > 1)
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p)
		{
			char szText[1024];
			ProcFmtText(Params[1].pszParam, szText, 1024, pTarget);
			p->SaySystem10(szText);
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：系统消息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SYSTEMMSG){
	if (nParam == 1)
	{
		char szText[1024];
		ProcFmtText(Params[0].pszParam, szText, 1024, pTarget);
		CGameWorld::GetInstance()->PostSystemMessage(szText);
	}
	else if (nParam > 1)
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p)
		{
			char szText[1024];
			ProcFmtText(Params[1].pszParam, szText, 1024, pTarget);
			p->SaySystem(szText);
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：玩家进入天下第一判断
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ENTERTOPLIST) {
	return CTopManager::GetInstance()->EnterProfessionTop(pPlayer);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：把自己设置为沙城城主
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETSABUKMASTER){
	return CSandCity::GetInstance()->SetSabukMaster(pPlayer);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加PK值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(INCPKPOINT){
	DWORD pkpoint = Params[0].nParam;
	pPlayer->AddPkPoint(pkpoint);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：减少PK值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DECPKPOINT){
	DWORD pkpoint = Params[0].nParam;
	pPlayer->DecPkPoint(pkpoint);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：清除PK值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLRPKPOINT){
	DWORD pkpoint = pPlayer->GetPkValue();
	pPlayer->DecPkPoint(pkpoint);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：武器升级
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DOUPGRADEWEAPON){
	return pPlayer->DoUpgradeWeapon();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：取出武器升级
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TAKEUPGRADEWEAPON){
	return pPlayer->TakeUpgradeWeapon();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否有武器升级
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HASUPGRADEWEAPON){
	return pPlayer->HasUpgradeWeapon();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：定义变量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(VAR){
	if (nParam == 0)return 0;
	char* pVal = "";
	if (nParam > 1) pVal = Params[1].pszParam;
	pTarget->AddVariable(Params[0].pszParam, pVal);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：调用指定PAGE
//		注释：如果调用的是跨脚本页面（格式：脚本名.页面名）, 则自动切换当前脚本对象
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CALL){
	if (nParam == 0)return 0;
	CScriptObject* pScriptObject = pTarget->getCurScriptObject();
	if (pScriptObject == nullptr)return 0;

	CSe_Page* page = pScriptObject->GetPage(Params[0].pszParam);
	if (page == nullptr)return 0;

	// 检查是否是跨脚本调用（参数中包含.表示跨脚本）
	CScriptObject* pOriginalScriptObject = pTarget->getCurScriptObject();
	CScriptObject* pCalledScriptObject = page->getObject();
	// 如果调用的是不同脚本中的页面, 切换当前脚本对象
	if (pCalledScriptObject != nullptr && pCalledScriptObject != pOriginalScriptObject)
		pTarget->setCurScriptObject(pCalledScriptObject);

	CallParamEx* pParamsArray = pShell->getCallParams();
	pShell->setCallParams(Params + 1);

	page->Execute(pShell, pTarget, pView);
	pShell->setCallParams(pParamsArray);

	if (pShell->getExecuteResult() != ER_OK && pShell->getExecuteResult() != ER_RETURN)
		bContinue = FALSE;
	return pShell->getExecuteResultValue().nParam;	// 这里返回值可用于 #if 或者 #switch
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：清除变量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLRVAR){
	if (nParam == 0)return 0;
	pTarget->ClrVariable(Params[0].pszParam);
}END_SCRIPT_FUNCTION

////----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：给变量赋值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MOVR){
	if (nParam == 0)return 0;
	if (*Params[0].pszParam == '_') return 0;
	else if (*Params[0].pszParam == '%') return 0;
	char* pVal = "";
	if (nParam > 1) pVal = Params[1].pszParam;
	pTarget->SetVariableValue(Params[0].pszParam, pVal);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：给变量值做加法
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(INC){
	if (nParam == 0)return 0;
	if (*Params[0].pszParam == '_') return 0;
	else if (*Params[0].pszParam == '%') return 0;
	char* pVal = pTarget->GetVariableValue(Params[0].pszParam);
	UINT nValue = pVal == nullptr ? 0 : StringToInteger(pVal);
	if (nParam > 1)
		nValue += Params[1].nParam;
	else
		nValue++;
	char szBuffer[20];
	sprintf(szBuffer, "%u", nValue);
	pTarget->SetVariableValue(Params[0].pszParam, szBuffer);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：给变量值做减法
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DEC){
	if (nParam == 0)return 0;
	if (*Params[0].pszParam == '_') return 0;
	else if (*Params[0].pszParam == '%') return 0;
	char* pVal = pTarget->GetVariableValue(Params[0].pszParam);
	UINT nValue = pVal == nullptr ? 0 : StringToInteger(pVal);
	if (nParam > 1)
		nValue -= Params[1].nParam;
	else
		nValue--;
	char szBuffer[20];
	sprintf(szBuffer, "%u", nValue);
	pTarget->SetVariableValue(Params[0].pszParam, szBuffer);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：给变量值乘法
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MUL) {
	if (nParam == 0)return 0;
	if (*Params[0].pszParam == '_') return 0;
	else if (*Params[0].pszParam == '%') return 0;
	char* pVal = pTarget->GetVariableValue(Params[0].pszParam);
	UINT nValue = pVal == nullptr ? 0 : StringToInteger(pVal);
	if (nParam > 1)
		nValue *= Params[1].nParam;
	else
		nValue *= 2;
	char szBuffer[20];
	sprintf(szBuffer, "%u", nValue);
	pTarget->SetVariableValue(Params[0].pszParam, szBuffer);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：给变量值除法
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DIV) {
	if (nParam == 0)return 0;
	if (*Params[0].pszParam == '_') return 0;
	else if (*Params[0].pszParam == '%') return 0;
	char* pVal = pTarget->GetVariableValue(Params[0].pszParam);
	UINT nValue = pVal == nullptr ? 0 : StringToInteger(pVal);
	if (nParam > 1)
		nValue /= Params[1].nParam;
	else
		nValue /= 2;
	char szBuffer[20];
	sprintf(szBuffer, "%u", nValue);
	pTarget->SetVariableValue(Params[0].pszParam, szBuffer);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：直接返回
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RETURN){
	pShell->setExecuteResult(ER_RETURN, nParam > 0 ? (Params[0].pszParam) : nullptr);
	bContinue = FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示指定玩家PAGE
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWPAGE){
	if (nParam < 2) return 0;
	CHumanPlayer* pp = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (pp == nullptr)return 0;
	CScriptTarget* pTarget1 = pp->GetScriptTarget();
	if (pTarget1 == nullptr)
		return 0;
	CallParamEx* pCallParam = pShell->getCallParams();
	CScriptObject* pScriptObject1 = pTarget1->getCurScriptObject();
	CScriptShell* pShell1 = pTarget1->getCurShell();
	pTarget1->setCurScriptObject(pTarget->getCurScriptObject());
	pTarget1->setCurShell(pShell);
	pShell->setCallParams(Params + 2);
	BOOL b = pShell->Execute(pTarget1, Params[1].pszParam, FALSE);
	pShell->setCallParams(pCallParam);
	pTarget1->setCurScriptObject(pScriptObject1);
	pTarget1->setCurShell(pShell1);
	return b;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：指定玩家是否在线
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HUMONLINE){
	if (nParam == 0)return 0;
	return (CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam) != nullptr);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：输入字符串
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(INPUTTEXT){
	if (nParam < 3)return 0;
	//	p0 = tips, p1 = length, p2 = page
	char	szPage[256];
	if (pShell->GetTitleId() == (UINT)-1)
	{
		char* p = strchr(Params[2].pszParam, '.');
		if (p == nullptr)
			sprintf(szPage, "@%s.%s", pTarget->getCurScriptObject()->getName(), Params[2].pszParam);
		else
			o_strncpy(szPage, Params[2].pszParam, 250);
	}
	else
		o_strncpy(szPage, Params[2].pszParam, 250);
	pTarget->SetInputPage(szPage, pShell, Params[1].nParam);
	WORD w1 = ((Params[1].nParam & 0xff) << 8) | 2;
	pPlayer->SendMsg(0, 0x1213, w1, 0, 0, (LPVOID)Params[0].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：玩家是否有老师
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HASTEACHER){
	return pPlayer->HasMaster();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：玩家是否有师傅
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HASMASTER){
	return pPlayer->HasMaster();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加学生
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDSTUDENT){
	if (nParam < 1)return FALSE;
	CHumanPlayer* pStudent = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (pStudent == nullptr)return FALSE;
	return pPlayer->AddStudent(pStudent);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：删除学生
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DELETESTUDENT){
	if (nParam < 1)return FALSE;
	return pPlayer->DeleteStudent(Params[0].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CANTAKESTUDENT){
	return (pPlayer->GetStudentCount() < 3);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否已经结婚
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISMARRIED){
	return pPlayer->IsMarried();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：远程调用指定玩家脚本命令
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REMOTECALL){
	if (nParam < 2)return 0;
	CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
	if (p == nullptr)return 0;
	fnCommandProc proc = CCommandManager::GetInstance()->GetCommandProc(Params[1].pszParam);
	if (proc == nullptr)return 0;
	BOOL bContinued;
	return proc(pShell, pTarget, pView, Params + 2, nParam - 2, bContinued);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：取下玩家穿戴物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TAKEEQUIPMENT){
	if (nParam == 0)return 0;
	ITEM item;
	if (!pPlayer->TakeEquipment(Params[0].nParam, item))return FALSE;
	pPlayer->SendTakeBagItem(&item);
	CItemManager::GetInstance()->DeleteItem(item.dwMakeIndex);
	return TRUE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加老师的声望值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDTEACHERCREDIT){
	if (nParam == 0)return 0;
	if (!pPlayer->HasMaster())return 0;
	return pPlayer->AddTeacherCredit(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：获取变量的值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(VALUEOF){
	if (nParam == 0)return 0;
	return Params[0].nParam;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：开关调试模式
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DEBUGMODE){
	g_bDebugScript = !g_bDebugScript;
	if (g_bDebugScript)
		pPlayer->SaySystem("进入脚本调试模式!");
	else
		pPlayer->SaySystem("离开脚本调试模式!");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否有学生
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HASSTUDENT){
	if (nParam == 0)
		return (pPlayer->GetStudentCount() != 0);
	return (pPlayer->HasStudent(Params[0].nParam));
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发系统消息
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SYSTEMMSGEX){
	if (nParam < 2)return 0;
	if (nParam == 2)
	{
		char szText[1024];
		ProcFmtText(Params[1].pszParam, szText, 1024, pTarget);
		CGameWorld::GetInstance()->PostSystemMessage(Params[0].nParam, szText);
	}
	else if (nParam > 2)
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[1].pszParam);
		if (p)
		{
			char szText[1024];
			ProcFmtText(Params[2].pszParam, szText, 1024, pTarget);
			p->SaySystemAttrib(Params[0].nParam, szText);
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：打开PAGE
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(OPENPAGE){
	if (nParam == 0)return 0;
	return CSystemScript::GetInstance()->Execute(pTarget, Params[0].pszParam, FALSE);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：显示当前更新怪物数量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SHOWMONSTER){
	UINT i = CGameWorld::GetInstance()->GetUpdateMonsterListCount();
	pPlayer->SaySystem("当前的更新怪物数量: %u\n", i);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：玩家变身
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TRANSFORM){
	if (nParam == 0)
	{
		pPlayer->TransformInto(0, 0);
		return 0;
	}
	MonsterClass* pClass = CMonsterManagerEx::GetInstance()->GetClassByName(Params[0].pszParam);
	if (pClass)
	{
		DWORD dwTime = 0xffffffff;
		if (nParam > 1)
			dwTime = Params[1].nParam * 1000;
		pPlayer->TransformInto(0x12, pClass->base.btImage, dwTime);
	}
	else
		pPlayer->TransformInto(0, 0);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：目标刷怪
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TARGETMONGEN){
	if (nParam < 8)return 0;
	char* pMonsterName = Params[0].pszParam;
	int mapid = Params[1].nParam;
	int x = Params[2].nParam;
	int y = Params[3].nParam;
	int r = Params[4].nParam;
	int c = Params[5].nParam;
	int tx = Params[6].nParam;
	int ty = Params[7].nParam;
	MONSTERGEN gen;
	gen.count = c;
	gen.curcount = 0;
	gen.tmrRefresh.Savetime();
	gen.dwRefreshDelay = 0;
	gen.mapid = mapid;
	gen.x = x;
	gen.y = y;
	gen.range = r;
	o_strncpy(gen.szName, pMonsterName, 31);
	if (nParam > 8)
		gen.pszScriptPage = Params[8].pszParam;
	if (gen.range <= 1000)
	{
		static CRandomRangeSpawnStrategy strategy;
		return CMonsterGenManager::GetInstance()->UpdateGenEx(&gen, &strategy, gen.count, FALSE, TRUE, tx, ty);
	}
	else
	{
		static CSpecialFormationSpawnStrategy strategy;
		return CMonsterGenManager::GetInstance()->UpdateGenEx(&gen, &strategy, gen.count, FALSE, TRUE, tx, ty);
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：比较字符串是否相同
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(STREQU){
	if (nParam < 2)return 0;
	return (strcmp(Params[0].pszParam, Params[1].pszParam) == 0);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：比较字符串是否相同, 不区分大小
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(STREQUNOCASE){
	if (nParam < 2)return 0;
	return (_stricmp(Params[0].pszParam, Params[1].pszParam) == 0);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：比较字符串指定长度内是否相同
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(STREQULENGTH){
	if (nParam < 3)return 0;
	return (strncmp(Params[0].pszParam, Params[1].pszParam, Params[2].nParam) == 0);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：比较字符串指定长度内是否相同, 不区分大小
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(STREQULENGTHNOCASE){
	if (nParam < 3)return 0;
	return (_strnicmp(Params[0].pszParam, Params[1].pszParam, Params[2].nParam) == 0);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：保存玩家变量到数据库
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SAVEVARTODB){
	pPlayer->SaveVars();//保存变量
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查多个地图中指定玩家数量
//		注释：multicheckmaphum retmapid 380 500 = 1 其中retmapid是定义的变量, 380是地图ID, 500是最大地图ID, 其中380到500之间的地图中玩家是否有1个
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MULTICHECKMAPHUM){
	if (nParam < 5)return 0;
	UINT nMapId = 0;
	CLogicMap* pMap = nullptr;
	for (nMapId = Params[1].nParam; nMapId <= Params[2].nParam; nMapId++)
	{
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(nMapId);
		if (pMap == nullptr)continue;
		UINT nCount = pMap->GetObjectCount(OBJ_PLAYER);
		if (CompareValue(nCount, Params[3].pszParam, Params[4].nParam))
		{
			char szValue[20];
			sprintf(szValue, "%u", nMapId);
			pTarget->SetVariableValue(Params[0].pszParam, szValue);
			return 1;
		}
	}
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：变更聊天字体颜色
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHANGEFONTCOLOR){
	if (nParam < 1)return 0;
	pPlayer->SetChatColor(Params[0].nParam & 0xff);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：小组传送
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(GROUPMOVE){
	CGroupObject* pObject = pPlayer->GetGroupObject();
	if (pObject == nullptr)return 0;
	if (nParam == 0)return 0;
	if (pPlayer->GetMap() == nullptr || pPlayer->GetMap()->IsFlagSeted(MF_NOGROUPMOVE))return 0;
	UINT nMapId = Params[0].nParam;
	UINT x = 0, y = 0;
	if (nParam >= 3)
	{
		x = Params[1].nParam;
		y = Params[2].nParam;
	}
	if (pObject)
	{
		xAutoPtrArray<CHumanPlayer>* parray = &pObject->GetMemberArray();
		if (parray)
		{
			for (UINT i = 0; i < (*parray).GetCount(); i++)
			{
				if ((*parray)[i])
				{
					if (x == 0 && y == 0)
						(*parray)[i]->RandomTeleport(nMapId);
					else
						(*parray)[i]->FlyTo(nMapId, x, y);
				}
			}
			return 1;
		}
	}
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查多个地图, 如果在范围内就传送
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MULTIMAPHUMTELEPORT){
	if (nParam < 3)return 0;
	UINT nMapId1 = Params[0].nParam;//最小地图ID
	UINT nMapId2 = Params[1].nParam;//最大地图ID
	UINT nMapId3 = Params[2].nParam;//传送的地图ID
	UINT x = 0, y = 0;
	if (nParam >= 5)
	{
		x = Params[3].nParam;//坐标X
		y = Params[4].nParam;//坐标Y
	}
	return CGameWorld::GetInstance()->AddGlobeProcess(EP_MAPTELEPORT, nMapId1, nMapId2, nMapId3, (y << 16) | (x & 0xffff));
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：消息弹窗
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MSGBOX){
	if (nParam == 1)
	{
		char szText[1024];
		ProcFmtText(Params[0].pszParam, szText, 1024, pTarget);
		pPlayer->SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)szText);
	}
	else if (nParam > 1)
	{
		CHumanPlayer* p = CHumanPlayerMgr::GetInstance()->FindbyName(Params[0].pszParam);
		if (p)
		{
			char szText[1024];
			ProcFmtText(Params[1].pszParam, szText, 1024, pTarget);
			p->SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)szText);
		}
	}
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：启动一个定时器
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TIMERSTART){
	if (nParam == 0)return 0;
	return CScriptTimerManager::GetInstance()->StartTimer(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：停止一个定时器
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TIMERSTOP){
	if (nParam == 0)return 0;
	return CScriptTimerManager::GetInstance()->StopTimer(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测一个定时器是否超时
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TIMERTIMEOUT){
	if (nParam < 2)return 0;
	CScriptTimerManager::GetInstance()->StopTimer(Params[0].nParam);
	DWORD dwTime = CScriptTimerManager::GetInstance()->GetTimerTime(Params[0].nParam);
	return (dwTime >= Params[1].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加动态NPC
//		注释：adddynamicnpc id, 名字, view, mapid, x, y, scriptpage
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDDYNAMICNPC){
	if (nParam < 7)return 0;
	return CNpcManager::GetInstance()->AddDynamicNpc(Params[0].nParam, Params[1].pszParam, Params[2].nParam,
		Params[3].nParam, Params[4].nParam, Params[5].nParam, Params[6].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：删除动态NPC
//		注释：removedynamicnpc id
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REMOVEDYNAMICNPC){
	if (nParam == 0)return 0;
	return CNpcManager::GetInstance()->RemoveDynamicNpc(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：系统延时打开PAGE
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SYSTEMDELAY){
	if (nParam < 2)return 0;
	int seconds = Params[0].nParam;
	char szPage[256];
	char* p = strchr(Params[1].pszParam, '.');
	if (p == nullptr && pShell->GetTitleId() == (UINT)-1)
		sprintf(szPage, "@%s.%s", pTarget->getCurScriptObject()->getName(), Params[1].pszParam);
	else
		o_strncpy(szPage, Params[1].pszParam, 250);
	CHumanPlayer* pTargetP = CAutoScriptManager::GetInstance()->GetScriptTarget();
	if (pTargetP == nullptr)return 0;
	return pTargetP->AddProcess(EP_OPENSCRIPTPAGE, pShell->GetTitleId(), 0, 0, 0, seconds * 1000, 1, szPage);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测玩家是否有某ID的任务
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HASTASK){
	return pPlayer->HasTask(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：添加任务
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDTASK){
	if (nParam == 0)return 0;
	return pPlayer->AddTask(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：删除任务
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(REMOVETASK){
	if (nParam == 0)return 0;
	return pPlayer->DeleteTask(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查指定任务的变量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKTASKVALUE) {
	if (nParam == 0)return FALSE;
	char sign1 = Params[1].pszParam[0]; // 变量类型
	int sign2 = StringToInteger(&Params[1].pszParam[1]); // 变量序号
	if (sign2 > 9) return FALSE;
	if (sign1 == 'I') // 数值变量
	{
		int nValue = pPlayer->GetTaskValue(Params[0].nParam, sign2);
		return CompareValue(nValue, Params[2].pszParam, Params[3].nParam); // 比较
	}
	else if (sign1 == 'S') // 字符变量不能比较
		return FALSE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：操作指定任务的变量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SETTASKVALUE) {
	if (nParam == 0)return FALSE;
	char sign1 = Params[1].pszParam[0]; // 变量类型
	int sign2 = StringToInteger(&Params[1].pszParam[1]); // 变量序号
	if (sign2 > 9) return FALSE;
	if (sign1 == 'I') // 数值变量
	{
		int nValue = pPlayer->GetTaskValue(Params[0].nParam, sign2);
		int iValue = CalcValue(nValue, Params[2].pszParam, Params[3].nParam); // 进行加减乘除计算
		pPlayer->SetTaskValue(Params[0].nParam, sign2, iValue);
		return TRUE;
	}
	else if (sign1 == 'S') // 字符变量
	{
		pPlayer->SetTaskString(Params[0].nParam, sign2, Params[2].pszParam);
		return TRUE;
	}
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载任务包括新增加的
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADTASK){
	CTaskManager::GetInstance()->Load(".\\Data\\Task");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：修改任务
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(MODIFYTASK){
	if (nParam < 2)return 0;
	return pPlayer->UpdateTask(Params[0].nParam, Params[1].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：完成任务
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(COMPLETETASK){
	if (nParam < 1)return 0;
	return pPlayer->DeleteTask(Params[0].nParam, TRUE);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检查任务进度
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKTASKSTEP){
	if (nParam < 3)return 0;
	UINT value = pPlayer->GetTaskStep(Params[0].nParam);
	return CompareValue(value, Params[1].pszParam, Params[2].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：给玩家加血
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDHP){
	if (nParam == 0)return 0;
	pPlayer->AddHp(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载限制物品列表
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADITEMLIMIT){
	CItemManager::GetInstance()->LoadLimit(".\\Data\\Config\\ItemLimit.txt");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：重新加载物品触发脚本
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(RELOADITEMSCRIPT){
	CItemManager::GetInstance()->LoadScriptLink(".\\Data\\ItemScript.txt");
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：是否是行会掌门人
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ISFIRSTGUILDMASTER){
	if (pPlayer->GetGuild() && pPlayer->GetGuild()->IsFirstMaster(pPlayer))
		return 1;
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：管理中设置目标变量
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(FORMATSTRING) {
	if (nParam < 2)return 0;
	char szText[1024];
	ProcFmtText(Params[1].pszParam, szText, 1024, pTarget);
	pTarget->SetVariableValue(Params[0].pszParam, szText);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测时间
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKDATETIME) {
	if (nParam < 3)return 0;
	CSystemTime t1(Params[0].pszParam);
	CSystemTime t2(Params[2].pszParam);
	char sign1 = Params[1].pszParam[0];
	char sign2 = Params[1].pszParam[1];
	switch (sign1)
	{
	case '>':
	{
		if (sign2 == '=')
			return (t1 >= t2);
		return (t1 > t2);
	}
	break;
	case '<':
	{
		if (sign2 == '=')
			return (t1 <= t2);
		return (t1 < t2);
	}
	break;
	case '=':
	{
		if (sign2 == '>')
			return (t1 >= t2);
		if (sign2 == '<')
			return (t1 <= t2);
		return (t1 == t2);
	}
	break;
	case '!':
		return (t1 != t2);
		break;
	}
	return 0;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表增加字符串
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDSTRINGLIST) {
	if (nParam < 2)return 0;
	CStringListManager::GetInstance()->AddToStringList(Params[0].pszParam, Params[1].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表删除字符串
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DELSTRINGLIST) {
	if (nParam < 2)return 0;
	CStringListManager::GetInstance()->DelFromStringList(Params[0].pszParam, Params[1].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测列表是否有某字符串
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CHECKSTRINGLIST) {
	if (nParam < 2)return 0;
	CStringListManager::GetInstance()->InStringList(Params[0].pszParam, Params[1].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：从列表清除字符串
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(CLEARSTRINGLIST) {
	if (nParam < 1)return 0;
	CStringListManager::GetInstance()->ClearStringList(Params[0].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：创建物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(BUILDITEM){
	if (nParam == 0)return 0;
	ITEM item;
	if (!CItemManager::GetInstance()->MakeupTemplateItem(Params[0].pszParam, item))
		return 0;
	if (nParam > 1)
	{
		if (!CItemManager::GetInstance()->UpgradeItem(item, Params[1].pszParam))return 0;
	}
	CItemManager::GetInstance()->IdentItem(item);
	return pPlayer->AddBagItem(item);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：地图做触发脚本
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(DOMAPSCRIPT){
	if (nParam == 0)return 0;
	CLogicMap* pMap = nullptr;
	char* pszPage = nullptr;
	if (nParam > 1)
	{
		pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(Params[0].nParam);
		pszPage = Params[1].pszParam;
	}
	else
	{
		pMap = pPlayer->GetMap();
		pszPage = Params[0].pszParam;
	}
	if (pMap == nullptr || pszPage == nullptr)return 0;
	pMap->AddAllProcess(OBJ_PLAYER, EP_OPENSCRIPTPAGE, -1, 0, 0, 0, 0, 0, Params[0].pszParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测玩家是否有夺宝追踪物品
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(HASTRACEDITEM){
	if (pPlayer->IsStatusSet(SI_ITEMTRACED))
		return TRUE;
	return FALSE;
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：发送行会急救
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(SENDGUILDSOS){
	//0x229d1d38 0064 38ff 0038 0400
	if (nParam < 1)return 0;
	if (pPlayer->GetGuild() == nullptr)return 0;
	char szText[1024];
	ProcFmtText(Params[0].pszParam, szText, 1024, pTarget);
	pPlayer->GetGuild()->SendMsg(pPlayer->GetId(), 0x64, 0x38ff, 0x38, 0x400, (LPVOID)szText);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测玩家是否在安全区
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(INSAFEAREA){
	return pPlayer->InSafeArea();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测玩家是否在城市安全区
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(INCITYAREA){
	return pPlayer->InCityArea();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：检测玩家是否在战争安全区
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(INWARAREA){
	return pPlayer->InWarArea();
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：拿走玩家的极品修练值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(TAKEFORGERATE){
	if (nParam < 1)return 0;
	return pPlayer->TakeForgePoint(Params[0].nParam);
}END_SCRIPT_FUNCTION

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：增加玩家的极品修练值
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
DEFINE_SCRIPT_FUNCTION(ADDFORGERATE){
	if (nParam < 1)return 0;
	pPlayer->GiveForgePoint(Params[0].nParam);
}END_SCRIPT_FUNCTION