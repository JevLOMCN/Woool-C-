#include "StdAfx.h"
#include <time.h>
#include <string.h>
#include <unordered_set>
#include "gsclientobj.h"
#include "server.h"
#include "humanplayermgr.h"
#include "gameworld.h"
#include "itemmanager.h"
#include "magicmanager.h"
#include "scriptnpc.h"
#include "npcmanager.h"
#include "scriptobjectmgr.h"
#include "gmmanager.h"
#include "visibleevent.h"
#include "logicmapmgr.h"
#include "downitemmgr.h"
#include "eventmanager.h"
#include "fireburnevent.h"
#include "guildmanager.h"
#include "guildex.h"
#include "monstermanagerex.h"
#include "monsterex.h"
#include "groupobject.h"
#include "sandcity.h"
#include "topmanager.h"
#include "SpecialEquipmentManager.h"
#include "cmdproc.h"
#include "scriptshell.h"
#include "scripttarget.h"
#include "scriptview.h"
#include "marketmanager.h"
#include "systemscript.h"
#include "BossTJ.h"
#include "TimeAchieve.h"
#include "GameStage.h"

CClientObj::CClientObj(void)
{
	Clean();
}

CClientObj::~CClientObj(void)
{
}

void CClientObj::Clean()
{
	CClientObject::Clean();
	m_State = GSUM_NOTVERIFIED;
	memset(&m_EnterInfo, 0, sizeof(m_EnterInfo));
	m_pPlayer = nullptr;
	m_iGmLevel = 0;
	m_bScrollTextMode = FALSE;
	m_bNoticeMode = FALSE;
	m_bCompetlyQuit = FALSE;
}

VOID CClientObj::Update()
{
	UpdateStarPing();
	CClientObject::Update();
}

VOID CClientObj::OnDBMsg(PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	switch (pMsg->wCmd)
	{
	case DM_QUERYTASKINFO:
	{
		if (m_pPlayer)
			m_pPlayer->OnTaskInfo((TASKINFO*)pMsg->data);
	}
	break;
	case DM_QUERYFENGHAO:
	{
		if (m_pPlayer)
			m_pPlayer->OnFengHaoInfo((FenghaoInfo*)pMsg->data);
	}
	break;
	case DM_QUERYUPGRADEITEM:
	{
		DWORD dwKey = *(DWORD*)&pMsg->wParam[0];
		if (dwKey == m_dwClientKey && pMsg->wParam[2] > 0)
		{
			if (m_pPlayer)
				m_pPlayer->SetUpgradeItem(((DBITEM*)pMsg->data)->item);
		}
	}
	break;
	case DM_QUERYMAGIC:
	{
		DWORD key = (pMsg->wParam[1] << 16) | pMsg->wParam[0];
		if (pMsg->wParam[2] & 0x8000 || key != m_dwClientKey)
		{
		}
		else
		{
			m_pPlayer->SetMagic((MAGICDB*)pMsg->data, pMsg->wParam[2]);
			m_pPlayer->SendMagicList();
		}
	}
	break;
	case DM_QUERYITEMS:
	{
		DWORD* pdwArray = (DWORD*)pMsg->data;
		ITEM item;
		if (pMsg->wParam[0] == SE_OK && pdwArray[0] == m_dwClientKey)
		{
			DBITEM* pItems = (DBITEM*)(pMsg->data + sizeof(DWORD));
			OnDBItem(pItems, pMsg->wParam[2], (BYTE)pMsg->wParam[1]);
		}
	}
	break;
	case DM_GETCHARDBINFO:
	{
		CHARDBINFO* pInfo = (CHARDBINFO*)pMsg->data;
		if (pInfo->dwClientKey != m_dwClientKey)
			break;
		if (pMsg->wParam[0] != SE_OK)
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"读取数据库失败, 请联系管理员解决!");
			Disconnect(3000);
			break;
		}
		CREATEHUMANDESC	desc;
		desc.dbinfo = *pInfo;
		desc.pClientObj = this;
		m_pPlayer = CHumanPlayerMgr::GetInstance()->NewPlayer();
		if (m_pPlayer == nullptr)
		{
			Disconnect();
			break;
		}
		if (!m_pPlayer->Init(desc) || !CHumanPlayerMgr::GetInstance()->AddPlayerNameList(m_pPlayer, desc.dbinfo.szName))
		{
			CHumanPlayerMgr::GetInstance()->DeletePlayer(m_pPlayer);
			m_pPlayer = nullptr;
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"初始化失败!");
			Disconnect(3000);
			LG2("玩家 %s 初始化失败\n", desc.dbinfo.szName);
			break;
		}
		m_pPlayer->LoadVars();
		CDBClientObj* pDI = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
		if (pDI)
		{
			// 加载数据
			pDI->SendQueryMagic(getId(), m_dwClientKey, m_pPlayer->GetDBId());
			pDI->SendQueryUpgradeItem(getId(), m_dwClientKey, m_pPlayer->GetDBId());
			pDI->SendQueryItem(getId(), m_dwClientKey, m_pPlayer->GetDBId(), IDF_BAG, m_pPlayer->GetBag().GetCountLimit());
			pDI->SendQueryItem(getId(), m_dwClientKey, m_pPlayer->GetDBId(), IDF_BANK, 100);
			pDI->SendQueryItem(getId(), m_dwClientKey, m_pPlayer->GetDBId(), IDF_PETBANK, 10);
			pDI->QueryTaskInfo(getId(), m_dwClientKey, m_pPlayer->GetDBId());
			pDI->QueryFengHaoInfo(getId(), m_dwClientKey, m_pPlayer->GetDBId());
		}
		else
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"查询数据失败!");
			Disconnect(3000);
			return;
		}
		m_State = GSUM_WAITINGCONFIRM;
	}
	break;
	case DM_CREATEITEM:
	{
		CREATEITEM* pInfo = (CREATEITEM*)pMsg->data;
		if (pInfo->dwClientKey != m_dwClientKey)
			break;
		OnCreateItem(pInfo->item, pInfo->wPos, pInfo->btFlag);
	}
	break;
	case DM_QUERYCOMMUNITY:
	{
		DWORD dwKey = pMsg->wParam[0] | (pMsg->wParam[1] << 16);
		if (dwKey != m_dwClientKey)
			break;
		if (m_pPlayer)
			m_pPlayer->OnCommunityInfo(pMsg->data);
	}
	break;
	default:
#ifdef _DEBUG
	{
		pServer->OnUnknownMsg(pMsg, datasize);
	}
#endif
	break;
	}
}

VOID CClientObj::OnCreateItem(ITEM& item, int pos, BYTE btFlag)
{
	if (btFlag == IDF_BAG)
		m_pPlayer->AddBagItem(item);
	else
		m_pPlayer->DropItem(item);
}

VOID CClientObj::OnVerifyString(const char* pszString)
{
	//***loginid/角色名字/selectcharid/20161104/0
	//***382198915/天斩/12941735/20161104/0
	char* p = (char*)pszString;
	if (p[0] == '*' && p[1] == '*' && p[2] == '*')
		p += 3;
	else
		return;
	UINT nLoginId = 0;
	UINT nSelCharId = 0;
	char* Params[5];
	int nParam = SearchParam(p, Params, 5, '/');
	if (nParam == 5)
	{
		if (_stricmp(Params[3], "20161104") != 0) // 这个判断在登录网关也判断了一次,  客户端版本时间
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"你的客户端版本不支持!");
			Disconnect(3000);
			return;
		}
		nLoginId = (UINT)atoi(Params[0]);
		nSelCharId = (UINT)atoi(Params[2]);
		if (CServer::GetInstance()->GetEnterInfo(nLoginId, nSelCharId, nullptr, m_EnterInfo))
		{
			if (strncmp(m_EnterInfo.szName, Params[1], 16) != 0 || CHumanPlayerMgr::GetInstance()->FindbyName(m_EnterInfo.szName) != nullptr)
			{
				SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"您登陆的角色已经登陆该服务器!");
				Disconnect(3000);
				LG2("您登陆的 %s 角色已经登陆该服务器\n", m_EnterInfo.szName);
				return;
			}
			CDBClientObj* pDI = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
			if (!pDI)
			{
				SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"查询数据失败!");
				Disconnect(3000);
				return;
			}
			else
				pDI->SendQueryDbInfo(getId(), m_dwClientKey, m_EnterInfo.szAccount, CServer::GetInstance()->GetNickName(), Params[1]);
			m_State = GSUM_WAITINGDBINFO;
			return;
		}
	}
	close();
}

static thread_local char g_szTempBuffer[65536];
VOID CClientObj::OnCodedMsg(xClientObject* pObject, PMIRMSG pMsg, int datasize)
{
	memcpy(g_szTempBuffer, pMsg, sizeof(MIRMSGHEADER) + datasize + 1);
	pMsg = (PMIRMSG)g_szTempBuffer;
	CServer* pServer = CServer::GetInstance();
	if (m_State != GSUM_VERIFIED)
	{
		switch (m_State)
		{
		case GSUM_NOTVERIFIED:
		{
			OnVerifyString((char*)pMsg);
		}
		break;
		case GSUM_WAITINGCONFIRM:
		{
			if (pMsg->wCmd == CM_CONFIRMFIRSTDIALOG)
			{
				CDBClientObj* pDI = CServer::GetInstance()->GetDBConnection(DI_CHARINFO);
				if (pDI)
				{
					// 加载数据
					pDI->SendQueryItem(getId(), m_dwClientKey, m_pPlayer->GetDBId(), IDF_EQUIPMENT, 20);
					pDI->QueryCommunity(getId(), m_dwClientKey, m_pPlayer->GetDBId());
				}
				else
				{
					m_pPlayer->SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"查询数据失败!");
					Disconnect(3000);
					return;
				}
				if (!CGameWorld::GetInstance()->AddMapObject(m_pPlayer))
				{
					m_pPlayer->SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"进入地图失败!");
					Disconnect(3000);
					LG2("玩家 %s 进入地图失败\n", m_pPlayer->GetName());
					return;
				}
				if (m_pPlayer->IsFirstLogin())
					m_pPlayer->AddProcess(EP_FIRSTLOGINPROCESS);
				else
					CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "LoginEnv.Login", FALSE);
				m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9609, 0, 0, 0, (LPVOID)m_pPlayer->GetName()); //确认玩家上线
				m_pPlayer->SendClientPluginInfo();// 发送客户端插件信息
				m_pPlayer->SendClientKeyConfig(); // 发送自定义快捷键
				CBossTJ::GetInstance()->SendBossList(m_pPlayer); //登录完成后, 发送加载BOSS图鉴列表数据
				m_State = GSUM_VERIFIED;
			}
		}
		break;
		}
		return;
	}
	if (m_pPlayer->IsDeath())
	{
		if (pMsg->wCmd != CM_SOFTCLOSE && pMsg->wCmd != CM_SAY && pMsg->wCmd != CM_SETITEMPOSITION && pMsg->wCmd != CM_LEAVESERVER)
		{
			m_pPlayer->SaySystem(getstrings(SD_STDERROR_YOUHAVEALREADYDEAD));
			return;
		}
	}
	ProcClientMsg(pMsg, datasize);
}

VOID CClientObj::SendActionResult(int x, int y, BOOL bSuccess)
{
	char szMsg[64];
	if (bSuccess)
		sprintf(szMsg, "#+G/%d/%d!", x, y);
	else
		sprintf(szMsg, "#+FL/%d/%d!", x, y);
	PostMsg(szMsg, (int)strlen(szMsg));
}

VOID CClientObj::OnDisconnect()
{
	xPacket* pPacket = nullptr;
	if (pPacket = m_xPacketQueue.pop())
	{
		OnDataPacket(pPacket);
		getServer()->releasePacket(pPacket);
	}
	if (m_pPlayer != nullptr)
	{
		CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "LogoutEnv.Logout", FALSE);
		if (m_pPlayer->GetGuild()) // 行会处理
			m_pPlayer->GetGuild()->MemberLogoff(m_pPlayer);
		if (m_pPlayer->GetExchangeObject() != nullptr) // 交易处理
			m_pPlayer->GetExchangeObject()->End(m_pPlayer, ET_CANCEL);
		if (m_pPlayer->GetGroupObject()) // 组队处理
			m_pPlayer->GetGroupObject()->DelMember(m_pPlayer);
		if (m_pPlayer->GetMap() != nullptr) // 地图处理
		{
			DWORD dwParam = 0;
			if (m_pPlayer->GetMap()->IsFlagSeted(MF_NORECONNECT, dwParam))
			{
				if (!m_pPlayer->RandomTeleport(dwParam))
					m_pPlayer->Home();
			}
		}
		CTopManager::GetInstance()->UpdateTopInfo(m_pPlayer); // 更新排行榜
		m_pPlayer->CleanPets();//清除宠物
		m_pPlayer->NoticeFriendOffline();//朋友上下线通知
		m_pPlayer->SaveVars();//保存变量
		m_pPlayer->UpdateToDB();//更新到数据库
		m_pPlayer->UpdateTaskToDB();//更新任务到数据库
		m_pPlayer->UpdateFengHaoToDB();//更新时长封号到数据库
		m_pPlayer->UpdateItemsToDB();//更新物品到数据库
		CGameWorld::GetInstance()->RemoveMapObject(m_pPlayer);//从地图中移除玩家
		CHumanPlayerMgr::GetInstance()->DeletePlayer(m_pPlayer);//释放玩家, 
	}

	if (!m_bCompetlyQuit && this->m_State == GSUM_VERIFIED)
	{
		CSCClientObj* pObj = CServer::GetInstance()->GetSCConnection();
		if (pObj != nullptr)
			pObj->SendMsgAcrossServer(0, MAS_RESTARTGAME, MST_SINGLE, (WORD)m_EnterInfo.dwSelectCharServerId, (char*)&m_EnterInfo, sizeof(m_EnterInfo));
	}
}

VOID CClientObj::OnCommand(const char* pszCommand)
{
	if (m_pPlayer == nullptr) return;
	CGmManager::GetInstance()->ExecGameCmd(pszCommand, m_pPlayer);
	return;
}

//增加包裹物品需要转换为客户端物品
VOID CClientObj::SendAddItem(ITEM& item)
{
	ITEMCLIENT clientItem{};
	memcpy(&clientItem, &item, sizeof(ITEMCLIENT));
	m_pPlayer->SendMsg(m_pPlayer->GetId(), SM_ADDBAGITEM, 0, 0, 0, &clientItem, sizeof(ITEMCLIENT));
}

//处理客户端的消息
VOID CClientObj::ProcClientMsg(PMIRMSG pMsg, int datasize)
{
	if (!m_pPlayer) return;
#ifdef _DEBUG
	CServer* pServer = CServer::GetInstance();
	// 排除一些请求消息不打印
	static const std::unordered_set<int> ignoreCmds = { 
		CM_WALK, CM_RUN, CM_TURN, CM_ATTACK, CM_SPELLSKILL,
		0xbd2, 0xbd0, 0xbd1, 0xbcb, 0xbca, 
		0x0c00
	};
	if (ignoreCmds.find(pMsg->wCmd) == ignoreCmds.end())
		pServer->OnUnknownMsg(pMsg, datasize, 1);
#endif
	switch (pMsg->wCmd)
	{
	case 0x6893:// 个性化签名和临时封号设置消息
	{
		WORD wPersonCode = pMsg->wParam[1];
		if (wPersonCode > 0)
		{
			m_pPlayer->SetPersonCode(wPersonCode);
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9593, 3, 1, 0); // 同步确认
		}
		PersonSetting* pPersonSetting = (PersonSetting*)pMsg->data;
		if (pPersonSetting->szPersonSign[0] != 0) // 个性化签名
		{ 
			m_pPlayer->SetPersonSign(pPersonSetting->szPersonSign);
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9593, 1, 0, wPersonCode, pPersonSetting->szPersonSign);
		}
		if (pPersonSetting->szTempBan[0] != 0) // 临时称号
		{
			m_pPlayer->SetTempRank(pPersonSetting->szTempBan);
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9593, 2, 0, 0, pPersonSetting->szTempBan);
		}
	}
	break;
	case 0xa06: // 头像框开启
	{
		if (pMsg->wParam[0] == 0)
		{
			char szBuffer[32]{};
			xPacket packet(szBuffer, 32);
			int nValue = 1;// 获得头像数量
			packet.push((LPVOID)&nValue, 4);
			nValue = 0; // 头像序号：0初始头像、1镖头、2镖师、3镖王、4镖神
			packet.push((LPVOID)&nValue, 4);
			nValue = 0;
			packet.push((LPVOID)&nValue, 4);
			nValue = 1;
			packet.push((LPVOID)&nValue, 4);
			nValue = 0;
			packet.push((LPVOID)&nValue, 4);
			nValue = 1;
			packet.push((LPVOID)&nValue, 4);
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa06, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
		}
		else
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa06, pMsg->wParam[0], 0, 0);
	}
	break;
	case 0x310: // 点头像框请求拜师
	{
		CHumanPlayer* pObject = (CHumanPlayer*)CGameWorld::GetInstance()->GetAliveObjectById(pMsg->dwFlag);//师傅或徒弟
		if (pObject == nullptr) break;
		switch (pMsg->wParam[0])
		{
		case 1://师傅-招收徒弟 - pObject徒弟/this师傅
		{
			if (m_pPlayer->GetStudentCount() == 3)
				m_pPlayer->SaySystem("你已收满徒弟!");
			else if (m_pPlayer->GetPropValue(PI_LEVEL) < 28)
				m_pPlayer->SaySystem("你的等级不够, 不能收徒!");
			else if (pObject->GetPropValue(PI_LEVEL) >= 28)
				m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x310, 258, 12, 0, "对方等级太高");
			else if (pObject->HasMaster())
				m_pPlayer->SaySystem("对方已经有师傅!");
			else
				pObject->SendMsg(m_pPlayer->GetId(), 0x310, 4, 0, 0);
		}
		break;
		case 2://徒弟-请求拜师 - pObject师傅/this徒弟
		{
			if (pObject->GetStudentCount() == 3)
				m_pPlayer->SaySystem("对方已收满徒弟!");
			else if (pObject->GetPropValue(PI_LEVEL) < 28)
				m_pPlayer->SaySystem("对方等级不够!");
			else if (m_pPlayer->GetPropValue(PI_LEVEL) >= 28)
				m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x310, 258, 12, 0, "你的等级太高");
			else if (m_pPlayer->HasMaster())
				m_pPlayer->SaySystem("你已经有师傅!");
			else
				pObject->SendMsg(m_pPlayer->GetId(), 0x310, 3, 0, 0);
		}
		break;
		case 3://师傅-同意拜师 - pObject徒弟/this师傅
		{
			if (m_pPlayer->GetStudentCount() < 3 && m_pPlayer->GetPropValue(PI_LEVEL) >= 28 && pObject->GetPropValue(PI_LEVEL) < 28)
			{
				if (m_pPlayer->AddStudent(pObject))
					pObject->SetMaster(m_pPlayer);
			}
		}
		break;
		case 4://同意收徒 - pObject师傅/this徒弟
		{
			if (pObject->GetStudentCount() < 3 && pObject->GetPropValue(PI_LEVEL) >= 28 && m_pPlayer->GetPropValue(PI_LEVEL) < 28)
			{
				if (pObject->AddStudent(m_pPlayer))
					m_pPlayer->SetMaster(pObject);
			}
		}
		break;
		case 259:
			pObject->SendMsg(m_pPlayer->GetId(), 0x312, 258, 16, 0, "对方拒绝收你为徒");
		break;
		case 260:
			pObject->SendMsg(m_pPlayer->GetId(), 0x312, 258, 18, 0, "对方拒绝拜你为师傅");
		break;
		}
	}
	break;
	case 0x312: // 点头像框请求交易
	{
		CHumanPlayer* pObject = (CHumanPlayer*)CGameWorld::GetInstance()->GetAliveObjectById(pMsg->dwFlag);//请求者
		if (pObject == nullptr) break;
		switch (pMsg->wParam[0])
		{
		case 0: //请求交易
		{
			if (pObject->GetAttackMode() != HAM_PEACE)
			{
				m_pPlayer->SaySystem("%s 不是和平模式", pObject->GetName());
				break;
			}
			m_pPlayer->ChangeAttackMode(HAM_PEACE);//自己切换到和平模式
			char szContent[64];
			sprintf_s(szContent, "%s 申请和你交易", m_pPlayer->GetName());
			pObject->SendMsg(m_pPlayer->GetId(), 0x312, 1, 20, 0, szContent);
		}
		break;
		case 1: //开始交易
		{
			m_pPlayer->ChangeAttackMode(HAM_PEACE);//切换到和平模式
			if (!m_pPlayer->Trade(pObject))
				m_pPlayer->CancelTrade();
		}
		break;
		default: //拒绝交易
			pObject->SendMsg(m_pPlayer->GetId(), 0x312, 256, 16, 0, "对方拒绝与你交易");
		break;
		}

	}
	break;
	case 0x9701: //打开英雄榜
	{
		if (pMsg->wParam[0] == 0) // 0、自己点的; 1、升级时的
		{
			//p1 值是按照位计算，开启不同排行榜
			//p1排名类型：英雄等级榜、天下名师榜、元神封神榜、天地灵兽榜、行会风云榜、宗族权势榜、天地仙兽榜、天地神龙榜
			//p2特殊类型：军衔等级榜、镇魔排名榜、万兽谱排名、战斗力榜、恶人排名榜、骑术排行榜
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9702, 35, 0, 0); //35的意思是只开启英雄排行、名师排行、行会排行
		}
	}
	case 0x9703: //查询英雄排行榜数据
	{
		CTopManager::GetInstance()->SendRank(m_pPlayer, pMsg->wParam[0], pMsg->wParam[2]);
	}
	break;
	case 0xc02://放到宠物背包中
	{
		m_pPlayer->PutItemToPetBag(pMsg->dwFlag);
	}
	break;
	case 0xc03://物品放回包裹中
	{
		m_pPlayer->GetItemFromPetBag(pMsg->dwFlag);
	}
	break;
	case 0x5eb2:// 删除任务
	{
		m_pPlayer->DeleteTask(pMsg->dwFlag);
	}
	break;
	case CM_LEAVESERVER:// 大退
	{
		m_bCompetlyQuit = TRUE;
	}
	break;
	case 0x3ef: // 挖尸体
	{
		if (m_pPlayer->CutBody(pMsg->dwFlag, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2]))
			m_pPlayer->SendAroundMsg(m_pPlayer->GetId(), 0x27d, m_pPlayer->getX(), m_pPlayer->getY(), pMsg->wParam[2]);
	}
	break;
	case 0x8810:// 发送一个物品MAKEINDEX
	{
		m_pPlayer->OnPutItem(pMsg->dwFlag, *(UINT*)&pMsg->wParam[0]);
	}
	break;
	case 0xc5: // 刻录回城神石 回城神石(大)
	{
		int nMapId = HIBYTE(pMsg->wParam[0]); // 地图ID
		char* szName{};
		switch (nMapId)
		{
		case 0:
			szName = "落霞岛";
			break;
		case 1:
			szName = "中州";
			break;
		case 2:
			szName = "热砂荒漠";
			break;
		case 3:
			szName = "禁地";
			break;
		default:
			szName = "落霞岛";
			break;
		}
		CItemBox& box = m_pPlayer->GetBag();
		ITEM* pItem = box.FindItem(pMsg->dwFlag); // 回城神石物品
		m_pPlayer->SetUsingItem(pItem); // 设置为使用物品
		m_pPlayer->RecordHomeXY(szName); // 记录回城石
		m_pPlayer->SendUpdateItem(*pItem); // 更新物品
	}
	break;
	case 0x8897:// 宠物属性
	{
		m_pPlayer->ShowPetInfo();
	}
	break;
	case 0x959: //成就系统 TimeAchieve.xml
	{
		CTimeAchieve::GetInstance()->SendAchieveData(m_pPlayer);
	}
	break;
	case 0x88a6: // 灵兽弹窗-时长区
	{
		//豹子宠物
		CItemBox& box = m_pPlayer->GetBag();
		ITEM* pPetStones[5];
		int nCount = box.FindItems("豹魔石", pPetStones, 5, TRUE);
		for (int i = 0; i < nCount; i++)
			m_pPlayer->SendOutPetInfo(pPetStones[i], 1);
		//骑乘宠物
		ITEM* pItem = m_pPlayer->GetEquipedHorseItem();
		if (pItem) m_pPlayer->SendOutPetInfo(pItem, 2);
	}
	break;
	case 0x331: // 灵兽弹窗-喂食-时长区
	{
		m_pPlayer->PetEatItem(pMsg->wParam[0], pMsg->dwFlag);
	}
	break;
	case 0x892: // 灵兽弹窗-回馈经验领取-时长区
	{
		CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "灵兽.BackExp");
	}
	break;
	case 0x0c00://客户端发送时间
	{
		SendMsg(m_pPlayer->GetTimeStamp(), 0x9600, 0, 0, 0);
	}
	break;
	case 0x1000://打开商城
	{
		CMarketManager::GetInstance()->OnClientMsg(m_pPlayer, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
	}
	break;
	case 0x502:
	{
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x502, 0, 0, 0);
	}
	break;
	case 0x97a: // 自定义快捷键-时长区
	{
		if (pMsg->wParam[0] == 1) // 设置快捷键, 暂时不去保存个人玩家自定义的快捷键
			break;
		m_pPlayer->SendClientKeyConfig(); //发送自定义快捷键
	}
	break;
	case 0x9b0: // 时长版封号系统
	{
		if (pMsg->wParam[0] == 0) // 打开封号弹窗
			m_pPlayer->SendFengHaoGrowInfo();
		else // 装备或者卸下
			m_pPlayer->SendFengHaoEquip(pMsg->wParam[1]);
	}
	break;
	case 0x9a2: // 新邮件系统
	{
		SendClientNewMail(pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2]);
	}
	break;
	case 0xa02: //客户端时长区功能集合
	{
		if (_stricmp(pMsg->data, "GameTimeMgr") == 0) // 充值游戏时长
		{
			GameTimeMgr* pGameTimeMgr = (GameTimeMgr*)pMsg->data;
			switch (pGameTimeMgr->btCode)
			{
			case 1: // 打开界面
			{
				CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "游戏时长.Open");
			}
			break;
			case 2: // 充值游戏时长
			{
				m_pPlayer->setVParam(0, pGameTimeMgr->nValue[2]); // 充值金额
				CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "游戏时长.Refill");
			}
			break;
			case 3: // 点击自动续费
			{
				m_pPlayer->setVParam(0, pGameTimeMgr->nValue[2]); // 状态
				CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "游戏时长.Renewal");
			}
			break;
			}
		}
		else if (_stricmp(pMsg->data, "guildmgr") == 0) // 行会
		{
			Guildmgr* pGuildmgr = (Guildmgr*)pMsg->data;
			switch (pGuildmgr->btCode)
			{
			case 0: // 帮助
			{
				CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "行会.Help");
			}
			break;
			case 1: // 行会列表
			{
				xPacket packet(g_szTempBuffer, 65535);
				const char* s1C = "guildmgr";
				packet.push(s1C);
				packet.push(1);
				int nValue = 1;
				packet.push((LPVOID)&nValue, 1);
				nValue = 0x0B;
				packet.push((LPVOID)&nValue, 4);
				std::string szRecruitStateList = CGuildManagerEx::GetInstance()->GetRecruitStateList();
				if (szRecruitStateList == "") break;
				packet.push(szRecruitStateList.c_str());
				packet.push(1);
				m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
			}
			break;
			case 2: // 申请加入
			{
				if (m_pPlayer->GetPropValue(PI_LEVEL) < 11)
				{
					m_pPlayer->SaySystem("你的等级还未超过10级, 能力还不够, 无法报名应征行会.");
					break;
				}
				const char* pszGuildName = pMsg->data + sizeof(Guildmgr);
				CGuildEx* pGuild = CGuildManagerEx::GetInstance()->FindGuild(pszGuildName);
				if (pGuild)
				{
					if (pGuild->AddApplyPlayer(m_pPlayer))
						m_pPlayer->SaySystem("你成功申请了加入行会, 会长将会择优录取.");
					else
						m_pPlayer->SaySystem("你已经申请过该行会了, 请耐心等候.");
				}
			}
			break;
			case 3: // 申请列表-this会长
			{
				CGuildEx* pGuild = m_pPlayer->GetGuild();
				if (!pGuild) break;
				if (!pGuild->IsMaster(m_pPlayer))break;
				pGuild->SendApplyList(m_pPlayer);
			}
			break;
			case 4: // 申请审核-this会长
			{
				CGuildEx* pGuild = m_pPlayer->GetGuild();
				if (!pGuild)
				{
					m_pPlayer->SaySystem("你还没有加入任何行会, 不能招募会员.");
					break;
				}
				if (!pGuild->IsMaster(m_pPlayer))
				{
					m_pPlayer->SaySystem("你不是行会会长, 不能招募行会成员, 前往皇宫找寻国王可创建行会.");
					break;
				}
				if (pGuild->IsFull())
				{
					m_pPlayer->SaySystem("你当前行会成员已经达到上限, 前往皇宫找寻国王可提高上限成员.");
					break;
				}
				const char* pszCharName = pMsg->data + sizeof(Guildmgr);
				int* boAllow = (int*)(pszCharName + strlen(pszCharName) + 1);
				pGuild->DelApplyPlayer(pszCharName); // 从列表中删除
				if (*boAllow == 0) // 同意
				{
					pGuild->SendDurationMemberList();
					CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(pszCharName);
					if (pPlayer) // 如果玩家在线
					{
						if (pPlayer->GetGuild() != nullptr)
							m_pPlayer->SaySystem("%s 已经加入了其他行会, 招募失败.", pszCharName);
						else if (pPlayer->GetPropValue(PI_LEVEL) > 0 && pGuild->AddMember(pPlayer))
						{
							m_pPlayer->SaySystem("成功招募了行会成员 %s.", pszCharName);
							pGuild->SendDurationMemberList();
							pPlayer->UpdateViewName();//招募者的显示名字
						}
						else
							m_pPlayer->SaySystem("%s 等级不足, 招募失败.", pszCharName);
					}
					else // 如果玩家不在线，就直接添加到列表即可
					{
						if (pGuild->AddMember(pszCharName))
							m_pPlayer->SaySystem("成功招募了行会成员 %s.", pszCharName);
					}
				}
				else
					m_pPlayer->SaySystem("你拒绝了 %s 的行会申请.", pszCharName);
			}
			break;
			case 5: // 允许申请加入
			{
				int* nSelect = (int*)(pMsg->data + sizeof(Guildmgr));
				int* boRecruitState = (int*)((char*)nSelect + 4);
				CGuildEx* pGuild = m_pPlayer->GetGuild();
				if (*nSelect == 1) //设置允许申请入会
					pGuild->SetRecruitState(*boRecruitState);
				else
				{
					xPacket packet(g_szTempBuffer, 65535);
					const char* s1C = "guildmgr";
					packet.push(s1C);
					packet.push(1);
					int nValue = 0x05;
					packet.push((LPVOID)&nValue, 4);
					packet.push(1);
					nValue = 0x01;
					packet.push((LPVOID)&nValue, 4);
					nValue = (pGuild && pGuild->GetRecruitState()) ? 0x01 : 0x00;
					packet.push((LPVOID)&nValue, 4);
					packet.push(12); // 插入12字节空
					nValue = 0x01;
					packet.push((LPVOID)&nValue, 4);
					nValue = (pGuild && pGuild->GetRecruitState()) ? 0x01 : 0x00;
					packet.push((LPVOID)&nValue, 4);
					packet.push(4); // 插入4字节空
					m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
				}
			}
			break;
			case 6: // 编辑宣言
			{
				CGuildEx* pGuild = m_pPlayer->GetGuild();
				if (!pGuild) break;
				if (!pGuild->IsMaster(m_pPlayer))break;
				if (pGuildmgr->nType == 0)
				{
					xPacket packet(g_szTempBuffer, 65535);
					const char* s1C = "guildmgr";
					packet.push(s1C);
					packet.push(1);
					int nValue = 0x06;
					packet.push((LPVOID)&nValue, 1);
					nValue = 0x01;
					packet.push((LPVOID)&nValue, 4);
					std::string szRecruitStateList = pGuild->GetDeclarationList(TRUE);
					packet.push(szRecruitStateList.c_str());
					packet.push(1);
					nValue = 0x01;
					packet.push((LPVOID)&nValue, 4);
					packet.push(12); // 插入12字节空
					nValue = 0x01;
					packet.push((LPVOID)&nValue, 4);
					nValue = 0x01;
					packet.push((LPVOID)&nValue, 4);
					nValue = 0x00;
					packet.push((LPVOID)&nValue, 4);
					m_pPlayer->SendMsg(m_pPlayer->GetId(), 0xa02, 0, 0, 0, (LPVOID)packet.getbuf(), packet.getsize());
				}
				else
				{
					if (pGuild == nullptr) break;
					const char* pszDeclaration = pMsg->data + sizeof(Guildmgr);
					pGuild->SetDeclarationList(pszDeclaration);
				}
			}
			break;
			}
		}
		else if (_stricmp(pMsg->data, "expback2020") == 0) // 每日经验
		{
			ExpBack* pExpBack = (ExpBack*)pMsg->data;
			switch (pExpBack->btCode)
			{
			case 0:
			{
				CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "每日经验.Main");
			}
			break;
			case 2:
			{
				CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "每日经验.Help");
			}
			break;
			}
		}
		else if (_stricmp(pMsg->data, "ActivityScore2014") == 0) // 活动
		{
			ActivityScore2014* pActivityScore2014 = (ActivityScore2014*)pMsg->data;
			switch (pActivityScore2014->btCode)
			{
			case 1: // 活动日历
			{
				if (m_pPlayer->GetPropValue(PI_LEVEL) >= 7)
				{
					CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), "活动日历.Main");
				}
			}
			break;
			}
		}
		else if (_stricmp(pMsg->data, "CheckPlayerMapJump") == 0) // 服务器阶段
		{
			MapJump* pMapJump = (MapJump*)pMsg->data;
			switch (pMapJump->btCode)
			{
			case 0x62: // 点击分页
			{
				CGameStage::GetInstance()->SendPlayerMapJumpPage(m_pPlayer, pMapJump->szPage);
			}
			break;
			case 0x63: // 打开首页
			{
				CGameStage::GetInstance()->SendPlayerMapJumpHome(m_pPlayer);
			}
			break;
			}
		}
		else if (_stricmp(pMsg->data, "LianYu18") == 0) // BOOS图解
		{
			BossTJ* pBossTJ = (BossTJ*)pMsg->data;
			if (pBossTJ->btCode == 2 && pBossTJ->nNum == 1)//单个BOSS数据
				CBossTJ::GetInstance()->SendBoss(m_pPlayer, pBossTJ->sName);
		}
		else if (_stricmp(pMsg->data, "chatglog2023") == 0) // 聊天记录
		{

		}
		else//自定义
		{
			char szPage[32];
			sprintf(szPage, "自定义界面.%s", pMsg->data);

			const BYTE* p = (const BYTE*)pMsg->data;
			int pos = 0;
			auto ReadInt = [&](int& n) { 
				memcpy(&n, p + pos, 4);
				pos += 4;
			};
			m_pPlayer->GetScriptTarget()->SetVariableValue("CustomActName", (char*)p + pos);
			// 跳过 ActName
			pos += strlen((char*)p) + 1;
			// ActType
			int nActType;
			char szActType[16];
			ReadInt(nActType);
			sprintf(szActType, "%d", nActType);
			m_pPlayer->GetScriptTarget()->SetVariableValue("CustomActType", szActType);
			// StrList
			BOOL IsHaveData = FALSE;
			memcpy(&IsHaveData, p + pos, 1);
			if (IsHaveData)
			{
				int nStrCount;
				char szStrCount[16];
				ReadInt(nStrCount);
				sprintf(szStrCount, "%d", nStrCount);
				m_pPlayer->GetScriptTarget()->SetVariableValue("CustomStrCount", szStrCount);
				for (int i = 0; i < nStrCount; i++) {
					char szVar[32];
					sprintf(szVar, "CustomS%d", i + 1);
					m_pPlayer->GetScriptTarget()->SetVariableValue(szVar, (char*)p + pos);
					pos += strlen((char*)p + pos) + 1;
				}
			}
			else
				pos += 1;
			// IntList
			memcpy(&IsHaveData, p + pos, 1);
			if (IsHaveData)
			{
				int nIntCount;
				char szIntCount[16];
				ReadInt(nIntCount);
				sprintf(szIntCount, "%d", nIntCount);
				m_pPlayer->GetScriptTarget()->SetVariableValue("CustomIntCount", szIntCount);
				for (int i = 0; i < nIntCount; i++) {
					int nVal;
					ReadInt(nVal);
					char szVar[32], szVal[16];
					sprintf(szVar, "CustomN%d", i + 1);
					sprintf(szVal, "%d", nVal);
					m_pPlayer->GetScriptTarget()->SetVariableValue(szVar, szVal);
					pos += 4;
				}
			}
			else
				pos += 1;
			CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), szPage);
		}
	}
	break;
	case 0x345: // 时长区-行会成员列表
	{
		//SendMsg(0, 0x2f2, 0, 0, 0);//弹窗提示玩家没有加行会
		CGuildEx* pGuild = m_pPlayer->GetGuild();
		if (pGuild)
			pGuild->SendDurationMemberList(m_pPlayer);
	}
	break;
	case 0xbc7: // 强行攻击(shift+左健)
	{
		//int	x = pMsg->dwFlag & 0xffff;
		//int	y = (pMsg->dwFlag & 0xffff0000) >> 16;
		int dir = pMsg->wParam[1] & 0xff;
		if (m_pPlayer->CheckItemInfo(_U_WEAPON, 6, 19)) // 是否穿戴 鹤嘴锄 武器
		{
			if (m_pPlayer->DoMine(dir)) // 挖矿
				SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), TRUE);
			else
				SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), FALSE);
		}
	}
	break;
	case 0x43://删除好友
	{
		m_pPlayer->DeleteFriend(pMsg->data);
	}
	break;
	case 0x44://确认加友
	{
		m_pPlayer->ReplyAddFriendRequest(pMsg->dwFlag, pMsg->data);
	}
	break;
	case 0x42://添加好友
	{
		if (CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(pMsg->data))
			pPlayer->PostAddFriendRequest(m_pPlayer);
		else
			m_pPlayer->SendFriendSystemError(FE_ADD_OFFONLINE, pMsg->data);
	}
	break;
	case CM_QUERYSTARTPRIVATESHOP: // 摆摊
	{
		PRIVATESHOPQUERY* pQuery = (PRIVATESHOPQUERY*)pMsg->data;
		BOOL bStarted = FALSE;
		if (pMsg->wParam[0] > 0)
		{
			if (m_pPlayer->GetActionType() == AT_PRIVATESHOP)
				bStarted = TRUE;
			if (m_pPlayer->SetPrivateShop(pMsg->wParam[0], pQuery))
			{
				if (bStarted)
					m_pPlayer->UpdatePrivateShopToAround();
				else
					m_pPlayer->SendStartPrivateShop();
			}
			else
			{
				m_pPlayer->StopPrivateShop();
				m_pPlayer->SendStopPrivateShop();
			}
		}
		else
		{
			m_pPlayer->StopPrivateShop();
			m_pPlayer->SendStopPrivateShop();
		}
	}
	break;
	case 0x5eb3: // 请求摆摊
	{
		if (pMsg->wParam[0] == 5)
			m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x80D5, 5, 0, 0);
	}
	break;
	case 0x6891://创建行会
	{
		m_pPlayer->GetScriptTarget()->OnInputConfirm(pMsg->data);
	}
	break;
	case 0x40://骑马 下马
	{
		if (m_pPlayer->RideHorse())
			m_pPlayer->SendMsg(1, 0xcd, 0, 0, 0); // 上马
		else
			m_pPlayer->SendMsg(0, 0xcd, 0, 0, 0); // 下马
	}
	break;
	case 0xaaa:
	{
		m_pPlayer->ReplyAddToGuildRequest((BOOL)pMsg->wParam[0]);
	}
	break;
	case 0x40e://加入行会
	{
		CGuildEx* pGuild = m_pPlayer->GetGuild();
		if (pGuild && pGuild->IsMaster(m_pPlayer))
		{
			CHumanPlayer* pMember = CHumanPlayerMgr::GetInstance()->FindbyName(pMsg->data);
			if (pMember)
			{
				if (pMember->GetGuild() != nullptr)
					m_pPlayer->SaySystem(getstrings(SD_ADDTOGUILD_ALREADYOTHERGUILDMEMBER));
				else
					pMember->PostAddToGuildRequest(m_pPlayer);
			}
		}
	}
	break;
	case 0xbcd:	//作揖
	{
		m_pPlayer->ChangeChatChannel();
	}
	break;
	case 0x408:	//从仓库取出物品
	{
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
		DWORD dwItemId = *(DWORD*)&pMsg->wParam[0];
		if (pNpc != nullptr)
		{
			if (m_pPlayer->TakeBankItem(dwItemId))
				m_pPlayer->SendMsg(dwItemId, 0x2c1, 0, 0, 0);
			else
				m_pPlayer->SendMsg(dwItemId, 0x2c2, 0, 0, 0);
		}
	}
	break;
	case 0x407:	//从仓库放入物品
	{
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
		DWORD dwItemId = *(DWORD*)&pMsg->wParam[0];
		if (pNpc != nullptr)
		{
			if (m_pPlayer->PutBankItem(dwItemId))
				m_pPlayer->SendMsg(dwItemId, 0x2bd, 0, 0, 0);
			else
				m_pPlayer->SendMsg(dwItemId, 0x2be, 0, 0, 0);
		}
	}
	break;
	case CM_DELETEGROUPMEMBER:
	{
		m_pPlayer->RemoveGroupMember(pMsg->data);
	}
	break;
	case CM_CHANGEGROUPMODE:
	{
		m_pPlayer->SetGroupMode((BOOL)pMsg->wParam[0]);
	}
	break;
	case 0x3f7: // 获取NPC贩卖物品的列表
	{
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
		if (pNpc != nullptr)
		{
			if (!pNpc->QueryItemList(m_pPlayer, pMsg->data, pMsg->wParam[0]))
				m_pPlayer->SendMsg(-1, 0x028B, 0, 0, 0);
		}
	}
	break;
	case 0x3f5: //点击NPC
	{
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
		if (pNpc != nullptr)
		{
			ITEM* pItem = m_pPlayer->FindBagItem(*(DWORD*)&pMsg->wParam[0]);
			if (pItem && pNpc->SellItem(m_pPlayer, *pItem))
			{
				m_pPlayer->SendMsg(m_pPlayer->GetMoney(MT_GOLD), 0x288, 0, 0, 0);
				m_pPlayer->SendWeightChanged();
				break;
			}
		}
		m_pPlayer->SendMsg(-1, 0x289, 0, 0, 0);//649
	}
	break;
	case 0x3f4:
	{
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
		if (pNpc != nullptr)
		{
			ITEM* pItem = m_pPlayer->FindBagItem(*(DWORD*)&pMsg->wParam[0]);
			if (pItem)
				m_pPlayer->SendMsg(pNpc->GetItemSellPrice(*pItem), 0x287, 0, 0, 0);
			else
				m_pPlayer->SendMsg(0, 0x287, 0, 0, 0);
		}
	}
	break;
	case 0x3fd://添加组队成员
	case CM_QUERYADDGROUPMEMBER:
	{
		m_pPlayer->AddGroupMember(pMsg->data);
	}
	break;
	case 0x041f:	//请求组队成员位置
	{
		m_pPlayer->UpdateGroupPosition();
	}
	break;
	case 0x40f://删除行会成员 
	{
		CGuildEx* pGuild = m_pPlayer->GetGuild();
		if (pGuild && pGuild->IsMaster(m_pPlayer))
		{
			const char* pszCharName = pMsg->data;
			if (_stricmp(pszCharName, m_pPlayer->GetName()) == 0)
			{
				m_pPlayer->SaySystem("掌门人不能被踢出.");
				break;
			}
			pGuild->RemoveMember(pszCharName);
			pGuild->SendDurationMemberList();
			CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyName(pszCharName);
			if (pPlayer) pPlayer->UpdateViewName();
		}
	}
	break;
	case 0x410://编辑行会公告
	{
		CGuildEx* pGuild = m_pPlayer->GetGuild();
		if (pGuild && pGuild->IsMaster(m_pPlayer))
		{
			pMsg->data[datasize++] = '\r';
			pMsg->data[datasize] = 0;
			m_pPlayer->GetGuild()->SetNotice(pMsg->data);
			char buffer[4096];
			int size = m_pPlayer->GetGuildFrontPage(buffer, 4096);
			m_pPlayer->SendMsg(0, 0x2f1, 0, 0, 0, (LPVOID)buffer, size);
			pGuild->SendFirstPage(m_pPlayer);
		}
		else
			m_pPlayer->SaySystem("你不是掌门人，不能编辑行会公告!");
	}
	break;
	case 0xa11: //行会封号编辑
	{
		CGuildEx* pGuild = m_pPlayer->GetGuild();
		if (pGuild == nullptr || !pGuild->IsMaster(m_pPlayer))
		{
			m_pPlayer->SaySystem("你不是掌门人，不能编辑行会封号!");
			break;
		}
		switch (pMsg->wParam[0])
		{
		case 1: // 添加新封号分组
		{
			CGuildGroupEx* pGroup = pGuild->NewGroup("成员");
			pGuild->AddGroupToList(pGroup);
			pGuild->Save();
			pGuild->SendGroupFengHaoData();
		}
		break;
		case 3: // 修改封号分组名
		{
			WORD nLevel = pMsg->wParam[1];//封号编号
			if (nLevel == 0 || nLevel > 99)
			{
				m_pPlayer->SendMsg(-7, 0x2f9, 0, 0, 0);	//职位重复或者出错
				break;
			}
			char* pszGroupName = (char*)pMsg->data; // 封号名
			if (!pGuild->IsProperName(pszGroupName))
			{
				m_pPlayer->SendMsg(-15, 0x2f9, 0, 0, 0);	//行会成员封号包含禁止字符
				break;
			}
			if (strlen(pszGroupName) > 16)
			{
				pszGroupName[16] = '\0';
			}
			CGuildGroupEx* pGroup = pGuild->GetGroupByLevel(nLevel);
			pGroup->SetName(pszGroupName);
			pGuild->Save();
			pGuild->SendGroupFengHaoData();
			pGuild->SendDurationMemberList(m_pPlayer);
		}
		break;
		case 4: // 任职、免职
		{
			char* pszCharName = (char*)pMsg->data; //任职玩家名
			if (strcmp(m_pPlayer->GetName(), pszCharName) == 0)
			{
				m_pPlayer->SaySystem("不能对自己进行免职和任职操作!");
				break;
			}
			if (strcmp(pGuild->GetFirstOwnerName(), pszCharName) == 0)
			{
				m_pPlayer->SaySystem("不能对掌门人进行免职和任职操作!");
				break;
			}
			WORD nLevel = pMsg->wParam[2];//任职的封号编号
			WORD nOldLevel = pMsg->wParam[1];//旧职的封号编号
			if (pGuild->ChangeGroup(pszCharName, nLevel, nOldLevel))
				pGuild->SendDurationMemberList();
			else
				m_pPlayer->SaySystem("免职或任职操作失败!");
		}
		break;
		}
	}
	break;
	case 0x412: //察看行会经验值
	{
		if (m_pPlayer->GetGuild())
			m_pPlayer->GetGuild()->SendExp(m_pPlayer);
	}
	break;
	case 0x40b: //察看行会信息
	case 0x40c: //察看行会主页
	{
		CGuildEx* pGuild = m_pPlayer->GetGuild();
		if (pGuild == nullptr)
			m_pPlayer->SendMsg(0, 0x2f2, 0, 0, 0);	//尚未加入门派弹窗
		else
		{
			pGuild->SendFirstPage(m_pPlayer);
			pGuild->SendGroupFengHaoData(m_pPlayer);
			pGuild->SendGuildTowerInfo(m_pPlayer);
		}
	}
	break;
	case 0x9a6:
	{
		switch (pMsg->wParam[0])
		{
		case 6: //打开社交弹窗-人脉选项卡刷新数据
		{
			m_pPlayer->UpdateCommunityInfoToClient();
		}
		break;
		}
	}
	break;
	case 0xbd2:	//破击剑法, 破盾
	case 0xbd0:	//半月弯刀
	case 0xbd1:	//烈火
	case 0xbcb:	//刺杀
	case 0xbca:	//攻杀
	{
		SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), m_pPlayer->SpecialHit(pMsg->wParam[1], pMsg->wParam[2]));
	}
	break;
	case 0x3f0://技能快捷键切换
	{
		USERMAGIC* pMagic = m_pPlayer->GetMagicByKey((BYTE)pMsg->wParam[0]);
		if (pMagic != nullptr)pMagic->magic.btKey = 0;
		pMagic = m_pPlayer->GetMagic(pMsg->dwFlag);
		if (pMagic != nullptr)pMagic->magic.btKey = (BYTE)pMsg->wParam[0];
	}
	break;
	case SM_DROPGOLD:
	{
		m_pPlayer->DropGold(pMsg->dwFlag);
	}
	break;
	case 0x3ee://使用物品
	{
		m_pPlayer->UseItem(pMsg->dwFlag, static_cast<DWORD>(MAKELONG(pMsg->wParam[0], pMsg->wParam[1])));
	}
	break;
	case 0x409: //走到地图连接点
	{
		CLogicMap* pMap = m_pPlayer->GetMap();
		int minimap = 0;
		if (pMap)
		{
			minimap = pMap->GetMiniMap();
		}
		m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x2c6, minimap & 0xffff, 0, 0, 0, 0);
	}
	break;
	case 0x3f6://购买摆摊物品
	{
		CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(pMsg->dwFlag);
		if (pObject == nullptr)break;
		if (pObject->GetType() == OBJ_NPC)
		{
			CScriptNpc* pNpc = (CScriptNpc*)pObject;
			DWORD dwError = 0;
			// 因为 pMsg->data 这里的数据前4个字节是空, 所以需要从第4个字节开始指针.
			if (pNpc != nullptr && pNpc->BuyItem(m_pPlayer, (char*)&pMsg->data[4], *(DWORD*)&pMsg->wParam[0], dwError))
				m_pPlayer->SendMsg(m_pPlayer->GetMoney(MT_GOLD), 0x28a, pMsg->wParam[0], pMsg->wParam[1], 0);
			else
				m_pPlayer->SendMsg(dwError, 0x28b, 0, 0, 0);
		}
		else if (pObject->GetType() == OBJ_PLAYER)
		{
			if (((CHumanPlayer*)pObject)->BuyPrivateShopItem(m_pPlayer, *(DWORD*)&pMsg->wParam[0], pMsg->data))
			{
				if (((CHumanPlayer*)pObject)->GetPrivateShopItemCount() > 0)
					((CHumanPlayer*)pObject)->UpdatePrivateShopToAround();
				else
				{
					((CHumanPlayer*)pObject)->StopPrivateShop();
					((CHumanPlayer*)pObject)->SendStopPrivateShop();
				}
			}
		}
	}
	break;
	case CM_SETITEMPOSITION://请求物品位置
	{
		m_pPlayer->SetBagItemPos((BAGITEMPOS*)pMsg->data, pMsg->wParam[2]);
	}
	break;
	case 0x3ff://修理物品
	{
		UINT npcid = pMsg->dwFlag;
		UINT nItemId = *(UINT*)&pMsg->wParam[0];
		ITEM* pItem = m_pPlayer->FindBagItem(nItemId);
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(npcid);
		if (pItem != nullptr && pNpc != nullptr && pNpc->RepairItem(m_pPlayer, *pItem))
		{
			if (m_pPlayer->IsSystemFlagSeted(SF_SPECIALREPAIR))
				pNpc->QuerySelectLink(m_pPlayer, "@~s_repair");
			else
				pNpc->QuerySelectLink(m_pPlayer, "@~repair");
		}
		else
			m_pPlayer->SendMsg(0, 0x029E, 0, 0, 0);
	}
	break;
	case 0x400:	//放入待修理物品
	{
		UINT npcid = pMsg->dwFlag;
		UINT nItemId = *(UINT*)&pMsg->wParam[0];
		ITEM* pItem = m_pPlayer->FindBagItem(nItemId);
		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(npcid);
		if (pItem != nullptr && pNpc != nullptr)
		{
			DWORD dwMoney = pNpc->GetItemRepairPrice(m_pPlayer, *pItem);
			m_pPlayer->SendMsg(dwMoney, 0x29f, 0, 0, 0, nullptr);
		}
	}
	break;
	case 0x3f3://NPC节点点击
	{
		if (pMsg->dwFlag == 0xffffffff)
		{
			if (!CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), pMsg->data))
				m_pPlayer->SendCloseScriptPage(0xffffffff);
			break;
		}
		if (pMsg->dwFlag == 0xfffffff0) // 变强攻略
		{
			char szPage[256];
			sprintf(szPage, "变强攻略.%s", (const char*)pMsg->data);
			CSystemScript::GetInstance()->Execute(m_pPlayer->GetScriptTarget(), szPage);
			break;
		}

		CScriptNpc* pNpc = CNpcManager::GetInstance()->GetScriptNpcById(pMsg->dwFlag);
		if (pNpc != nullptr)
			pNpc->QuerySelectLink(m_pPlayer, pMsg->data);
	}
	break;
	case 0x3f2://点击NPC, 或者玩家摆摊
	{
		CAliveObject* pObject = CGameWorld::GetInstance()->GetAliveObjectById(pMsg->dwFlag);
		if (pObject == nullptr)break;
		if (pObject->GetType() == OBJ_NPC)
		{
			CScriptNpc* pNpc = (CScriptNpc*)pObject;
			pNpc->QueryTalk(m_pPlayer);
		}
		else if (pObject->GetType() == OBJ_PLAYER)
		{
			((CHumanPlayer*)pObject)->SendPrivateShopPage(m_pPlayer, 1);
		}
	}
	break;
	case CM_SOFTCLOSE://小退游戏
	{
		CSCClientObj* pObj = CServer::GetInstance()->GetSCConnection();
		if (pObj != nullptr)
		{
			pObj->SendMsgAcrossServer(0, MAS_RESTARTGAME, MST_SINGLE, (WORD)m_EnterInfo.dwSelectCharServerId, (char*)&m_EnterInfo, sizeof(m_EnterInfo));
			m_bCompetlyQuit = TRUE;
		}
		m_pPlayer->UpdateToDB();//更新玩家数据
	}
	break;
	case 0x52://观察其他玩家装备
	{
		CHumanPlayer* pPlayer = CHumanPlayerMgr::GetInstance()->FindbyId(pMsg->dwFlag);
		if (pPlayer != nullptr)
		{
			xPacket packet(g_szTempBuffer, 65535);
			pPlayer->GetViewDetail(packet);
			//p1低位是行会图标,高位是物品长度、p2高位穿戴物品最大数量
			m_pPlayer->SendMsg(pPlayer->GetId(), 0x2ef, 0, MAKEWORD(0, _U_MAX), MAKEWORD(0, 1), (LPVOID)packet.getbuf(), packet.getsize());
		}
	}
	break;
	case CM_SPELLSKILL://使用技能3017 
	{
		DWORD dwId = MAKEDWORD2W(pMsg->wParam[0], pMsg->wParam[2]);
		int x, y;
		x = pMsg->dwFlag & 0xffff;
		y = (pMsg->dwFlag & 0xffff0000) >> 16;
		BOOL fRet = m_pPlayer->SpellCast(x, y, dwId, pMsg->wParam[1]);
		SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), fRet);
	}
	break;
	case CM_CANCELTRADE:
	{
		m_pPlayer->CancelTrade();
	}
	break;
	case CM_PUTTRADEGOLD:
	{
		if (m_pPlayer->PutTradeMoney((money_type)pMsg->wParam[0], pMsg->dwFlag))
		{
			DWORD dwCurMoney = m_pPlayer->GetMoney((money_type)pMsg->wParam[0]);
			m_pPlayer->SendMsg(pMsg->dwFlag, SM_PUTTRADEGOLDOK, dwCurMoney & 0xffff, (dwCurMoney >> 16), pMsg->wParam[0]);
		}
		else
			m_pPlayer->SendMsg(0, SM_PUTTRADEGOLDFAIL, 0, 0, 0);
	}
	break;
	case CM_QUERYTRADEEND:
	{
		m_pPlayer->ConfirmTrade();
	}
	break;
	case CM_PUTTRADEITEM:
	{
		if (m_pPlayer->PutTradeItem(pMsg->dwFlag))
			m_pPlayer->SendMsg(0, SM_PUTTRADEITEMOK, 0, 0, 0, 0);
		else
			m_pPlayer->SendMsg(0, SM_PUTTRADEITEMOK, 0, 0, 0, 0);
	}
	break;
	case CM_QUERYTRADE:
	{
		m_pPlayer->Trade();
	}
	break;
	case CM_TAKEONITEM: //穿戴物品
	{
		SendEquipItemResult(m_pPlayer->EquipItem((int)pMsg->wParam[0], pMsg->dwFlag), (int)pMsg->wParam[0], pMsg->dwFlag);
	}
	break;
	case CM_TAKEOFFITEM: //取下穿戴物品
	{
		SendUnEquipItemResult(m_pPlayer->UnEquipItem((int)pMsg->wParam[0], pMsg->dwFlag), (int)pMsg->wParam[0], pMsg->dwFlag);
	}
	break;
	case CM_DROPITEM:
	{
		if (m_pPlayer->DropBagItem(pMsg->dwFlag))
		{
			m_pPlayer->SendMsg(pMsg->dwFlag, 0x258, 0, 0, 0);
			m_pPlayer->SendWeightChanged();
		}
		else
			m_pPlayer->SendMsg(pMsg->dwFlag, 0x259, 0, 0, 0);
	}
	break;
	case CM_PICKUPITEM:
	{
		if (m_pPlayer->PickupItem())
			m_pPlayer->SendWeightChanged();
	}
	break;
	case 0xc01://宠物拾取
	{
		m_pPlayer->SetPetFindItem(pMsg->wParam[0], pMsg->wParam[1]);
	}
	break;
	case CM_STOP:
	{
		SendActionResult(m_pPlayer->GetActionX(), m_pPlayer->GetActionY(), TRUE);
		m_pPlayer->Stop();
	}
	break;
	case CM_SAY:
	{
		if (m_pPlayer->IsSystemFlagSeted(SF_BANED))
		{
			m_pPlayer->SaySystem(getstrings(SD_YOUAREBANED));//你被禁言
			break;
		}
		switch (pMsg->wParam[0])
		{
		case 0: case 1:
		{
			switch (pMsg->data[0])
			{
			case '@'://命令
			{
				OnCommand(pMsg->data + 1);
			}
			break;
			case '/'://密谈
			{
				char* p = pMsg->data + 1;
				while (*p && *p != ' ' && *p != '	')
					p++;
				if (*p != 0)
				{
					*p = 0;
					p++;
				}
				m_pPlayer->ChannelSay(CCH_WISPER, pMsg->data + 1, p);
			}
			break;
			case '!':
			{
				switch (pMsg->data[1])
				{
				case '!'://组队
					m_pPlayer->ChannelSay(CCH_GROUP, nullptr, pMsg->data + 2);
					break;
				case '~'://门派
					m_pPlayer->ChannelSay(CCH_GUILD, nullptr, pMsg->data + 2);
					break;
				default://喊话
					m_pPlayer->ChannelSay(CCH_CRY, nullptr, pMsg->data + 1);
					break;
				}
			}
			break;
			default:
			{
				if (m_pPlayer->IsSystemFlagSeted(SF_SCROLLTEXTMODE))
					CGameWorld::GetInstance()->AddGlobeProcess(EP_SCROLLTEXT, 0, 0, 0, 0, 50, 1, pMsg->data);
				else if (m_pPlayer->IsSystemFlagSeted(SF_NOTICEMODE))
					CGameWorld::GetInstance()->PostSystemMessage(pMsg->data);
				else
					m_pPlayer->ChannelSay(m_pPlayer->GetChatChannel(), nullptr, pMsg->data);
			}
			break;
			}
		}
		break;
		case 2: // 摆摊卖家留言
		case 3: // 摆摊买家留言
		{
			char* p = pMsg->data;
			while (*p && *p != ' ')
				p++;
			if (*p != 0)
			{
				*p = 0;
				p++;
			}
			CHumanPlayer* player = CHumanPlayerMgr::GetInstance()->FindbyName(pMsg->data);
			if (player)
			{
				char szBuff[248];
				sprintf(szBuff, "%s: %s", m_pPlayer->GetName(), p);
				szBuff[120] = '\0';
				WORD nType = pMsg->wParam[0];
				player->SendMsg(m_pPlayer->GetId(), 0x67, 0, 0, nType, szBuff);
			}
			else
				m_pPlayer->SaySystem("%s 玩家不在线, 无法收到信息!", pMsg->data);
		}
		break;
		}
	}
	break;
	case 0x3d3:
	{
		m_pPlayer->SendMsg(pMsg->dwFlag, 0x3d4, 0, 0, 0, nullptr);
	}
	break;
	case CM_TURN:
	{
		if (!m_pPlayer->IsEnterMap()) break;
		if (m_pPlayer->Turn(pMsg->wParam[1] & 0xff))
			SendActionResult(m_pPlayer->GetActionX(), m_pPlayer->GetActionY(), TRUE);
		else
			SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), FALSE);
	}
	break;
	case CM_WALK:
	{
		if (!m_pPlayer->IsEnterMap()) break;
		int	x = pMsg->dwFlag & 0xffff;
		int	y = (pMsg->dwFlag & 0xffff0000) >> 16;
		int dir = pMsg->wParam[1] & 0xff;
		if (!m_pPlayer->WalkXY(x, y, dir))
			SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), FALSE);
		else
		{
			SendActionResult(m_pPlayer->GetActionX(), m_pPlayer->GetActionY(), TRUE);
			CScriptTargetForPlayer* pSTarget = m_pPlayer->GetScriptTarget();
			if (pSTarget) pSTarget->CheckExceedDistance();
		}	
	}
	break;
	case CM_GETMEAL:
	{
		m_pPlayer->GetMeal(pMsg->wParam[1] & 0xff);
		SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), TRUE);
	}
	break;
	case 0xba0: // 马鞭抓马
	{
		m_pPlayer->DoTrainHorse(pMsg->wParam[1] & 0xff);
		SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), TRUE);
	}
	break;
	case CM_ATTACK://普通攻击
	{
		e_humanattackmode mode = m_pPlayer->GetAttackMode();
		if (m_pPlayer->Attack(pMsg->wParam[1] & 0xff, 0, mode))
			SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), TRUE);
		else
			SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), FALSE);
		if (m_pPlayer->CheckItemInfo(_U_WEAPON, 5, 7)) // 伐木斧
			m_pPlayer->SetCurAttackType(DT_CUTTREE);
	}
	break;
	case CM_RUN:
	{
		if (!m_pPlayer->IsEnterMap()) break;
		int	x = pMsg->dwFlag & 0xffff;
		int	y = (pMsg->dwFlag & 0xffff0000) >> 16;
		int dir = pMsg->wParam[1] & 0xff;
		if (!m_pPlayer->RunXY(x, y, dir))
			SendActionResult(m_pPlayer->getX(), m_pPlayer->getY(), FALSE);
		else
		{
			SendActionResult(m_pPlayer->GetActionX(), m_pPlayer->GetActionY(), TRUE);
			CScriptTargetForPlayer* pSTarget = m_pPlayer->GetScriptTarget();
			if (pSTarget) pSTarget->CheckExceedDistance();
		}	
	}
	break;
	default:
#ifdef _DEBUG
	{
		pServer->OnUnknownMsg(pMsg, datasize);
	}
#endif
	break;
	}
}

VOID CClientObj::SendEquipItemResult(BOOL bSuccess, int pos, DWORD dwMakeIndex)
{
	WORD wCmd = SM_TAKEON_OK;
	DWORD dwFeather = m_pPlayer->GetFeather();
	if (!bSuccess)
	{
		wCmd = SM_TAKEON_FAIL;
		dwFeather = 0xffffffff;
		m_pPlayer->SendMsg(dwFeather, wCmd, 0, 0, 0, nullptr);
		return;
	}
	m_pPlayer->SendMsg(dwFeather, wCmd, 0, 0, 0, nullptr);
	m_pPlayer->SendFeatureChanged();//41外观信息
	m_pPlayer->UpdateProp();  //更新52人物信息
	m_pPlayer->UpdateSubProp(); //更新人物附加属性752
	m_pPlayer->SendStatusChanged();//657
	CItemManager::GetInstance()->UpdateItemPos(dwMakeIndex, IDF_EQUIPMENT, pos);
}

VOID CClientObj::SendUnEquipItemResult(BOOL bSuccess, int pos, DWORD dwMakeIndex)
{
	WORD wCmd = SM_TAKEOFF_OK;
	DWORD dwFeather = m_pPlayer->GetFeather();
	if (!bSuccess)
	{
		wCmd = SM_TAKEOFF_FAIL;
		dwFeather = 0xffffffff;
		m_pPlayer->SendMsg(dwFeather, wCmd, 0, 0, 0, nullptr);
		return;
	}
	m_pPlayer->SendMsg(dwFeather, wCmd, 0, 0, 0, nullptr);
	m_pPlayer->SendMsg(0, 0x26c, 0, 0, 0);
	m_pPlayer->SendFeatureChanged();
	m_pPlayer->UpdateProp();
	m_pPlayer->UpdateSubProp();
	m_pPlayer->SendStatusChanged();
	CItemManager::GetInstance()->UpdateItemPos(dwMakeIndex, IDF_BAG, pos);
}

VOID CClientObj::SendClientNewMail(WORD Parm1, WORD Parm2, WORD Parm3)
{
	xPacket packet(g_szTempBuffer, 65535);
	switch (Parm2)
	{
	case 0:
	{
	}
	break;
	case 1:
	{
	}
	break;
	case 2:
	{
	}
	break;
	case 3:
	{
	}
	break;
	case 4:
	{
	}
	break;
	case 5:
	{
	}
	break;
	}
	m_pPlayer->SendMsg(m_pPlayer->GetId(), 0x9A2, Parm1, Parm2, Parm3, (LPVOID)packet.getbuf(), packet.getsize());
}

VOID CClientObj::SendBagItems(DBITEM* pItems, int count)
{
	// 背包物品数据
	static thread_local ITEMCLIENT items[BIGBAG_SLOT]{};
	if (count > BIGBAG_SLOT) count = BIGBAG_SLOT;
	for (int i = 0; i < count; i++)
	{
		items[i] = *(ITEMCLIENT*)&pItems[i].item;
	}
	m_pPlayer->SendMsg(m_pPlayer->GetId(), SM_BAGINFO, 0, 0, count, items, sizeof(ITEMCLIENT) * count);
	// 背包物品位置数据
	static thread_local BAGITEMPOS itempos[BIGBAG_SLOT]{};
	for (int i = 0; i < count; i++)
	{
		itempos[i].ItemId = pItems[i].item.dwMakeIndex;
		itempos[i].wPos = pItems[i].pos;
	}
	if (m_pPlayer) m_pPlayer->SetBagItemPos(itempos, count);
	m_pPlayer->SendMsg(0, SM_SETITEMPOSITION, 0, 0, count, (LPVOID)itempos, sizeof(BAGITEMPOS) * count);
}

VOID CClientObj::SendEquipments()
{
	EQUIPMENT equipments[20];
	int count = m_pPlayer->GetEquipments(equipments);
	m_pPlayer->SendMsg(0, SM_EQUIPMENTS, 0, 0, 0, (LPVOID)equipments, sizeof(EQUIPMENT) * count);
	m_pPlayer->SendFeatureChanged();
	m_pPlayer->UpdateProp();
	m_pPlayer->UpdateSubProp();
	m_pPlayer->SendStatusChanged();
}

static int qsort_comp_dbitem(const DBITEM* p1, const DBITEM* p2)
{
	if (p1->pos > p2->pos)return 1;
	if (p1->pos < p2->pos)return -1;
	return 0;
}

VOID CClientObj::OnDBItem(DBITEM* pItemArray, int nCount, BYTE btFlag)
{
	ITEM item;
	if (nCount > 1)
	{
		qsort((void*)pItemArray, nCount, sizeof(DBITEM), (int(*)(const void*, const void*))qsort_comp_dbitem);
	}
	switch (btFlag)
	{
	case IDF_BANK:
	{
		for (int i = 0; i < nCount; i++)
		{
			m_pPlayer->AddBankItem(pItemArray[i].item, FALSE);
		}
	}
	break;
	case IDF_BAG:
	{
		m_pPlayer->SetSystemFlag(SF_BAGLOADED, TRUE);
		for (int i = 0; i < nCount; i++)
		{
			item = pItemArray[i].item;
			m_pPlayer->ItemToClient(item); // 处理数据
			m_pPlayer->AddBagItem(pItemArray[i].item, TRUE, FALSE, FALSE);
			pItemArray[i].item = item;
		}
		//	发送背包信息和位置信息.
		SendBagItems(pItemArray, nCount);
		m_pPlayer->SendWeightChanged();
	}
	break;
	case IDF_EQUIPMENT:
	{
		m_pPlayer->SetSystemFlag(SF_EQUIPMENTLOADED, TRUE);
		for (int i = 0; i < nCount; i++)
		{
			item = pItemArray[i].item;
			m_pPlayer->ItemToClient(item); // 处理数据
			m_pPlayer->EquipItem(pItemArray[i].pos, pItemArray[i].item, TRUE);
			pItemArray[i].item = item;
		}
		//	发送装备信息
		//	发送属性更改消息
		SendEquipments();
	}
	break;
	case IDF_PETBANK:
	{
		m_pPlayer->OnPetBank(pItemArray, nCount);
	}
	break;
	}
}

VOID CClientObj::OnConnection()
{
}