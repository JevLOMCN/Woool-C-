#include "StdAfx.h"
#include ".\server.h"
#include "gameworld.h"
#include "NpcManager.h"
#include "ScriptNpc.h"
#include "LogicMapMgr.h"
#include "ItemManager.h"
#include "DownItemMgr.h"
#include "magicmanager.h"
#include "monstermanagerex.h"
#include <vector>
CServer* CServer::m_pInstance = nullptr;

CServer::CServer(void)
{
	m_pGameWorld = nullptr;
	m_pInstance = this;
	m_bWillClose = FALSE;
}

CServer::~CServer(void)
{
}

CClientObject* CServer::NewClientObject()
{
	return newObject();
}

VOID CServer::DeleteClientObject(CClientObject* pObject)
{
	deleteObject((CClientObj*)pObject);
}

BOOL CServer::InitServer(CSettingFile& s)
{
	int maxconnection = s.GetInteger(m_pServerName, "MaxConnection", 100);
	create(maxconnection);
	m_EnterObjects.Create(maxconnection);
	PRINT(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);
	m_pGameWorld = CGameWorld::GetInstance();
	return m_pGameWorld->Init();
}

VOID CServer::CleanServer()
{
	if (m_pGameWorld)
	{
		delete m_pGameWorld;
		m_pGameWorld = nullptr;
	}
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

VOID CServer::Update()
{
	CClientObj* pObject = m_ObjectPool.First();
	while (pObject)
	{
		if (pObject->IsConnected())
		{
			int processCount = 0; // 每帧只处理5个包
			while (processCount < 5 && pObject->GetPacketQueueCount() > 0)
			{
				pObject->Update();
				processCount++;
			}
		}
		pObject = m_ObjectPool.Next();
	}
	// 更新游戏世界
	const DWORD dwUpdateKey = GetUpdateKey();
	m_pGameWorld->SetUpdateKey(dwUpdateKey);
	m_pGameWorld->Update();
	if ((dwUpdateKey & 1) == 0)// 偶数帧：处理客户端消息
		UpdateSCServer(); // 处理服务中心消息
	else // 奇数帧：处理服务器通信
		UpdateDBServer(); // 处理数据中心消息
}

VOID CServer::OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize)
{
	switch (wCmd)
	{
	case MAS_ENTERGAMESERVER:
	{
		ENTERGAMESERVER* pEnterInfo = (ENTERGAMESERVER*)pszData;
		UINT id = pEnterInfo->nClientId;
		if (m_EnterObjects.GetFreeCount() == 0)
			pEnterInfo->result = SE_SERVERFULL; // 游戏服务器人数已满
		else
			pEnterInfo->result = AddEnterAccount(pEnterInfo->nLoginId, pEnterInfo->nSelCharId, pEnterInfo->szAccount, pEnterInfo->szName, wIndex);
		m_SCClientObj.SendMsgAcrossServer(id, MAS_ENTERGAMESERVER, MST_SINGLE, wIndex, pszData, datasize);
	}
	break;
	}
}

VOID CServer::OnSCMsg(PMIRMSG pMsg, int datasize)
{
	switch (pMsg->wCmd)
	{
	case SCM_FINDSERVER:
	{
		if (pMsg->wParam[0] == SE_OK)
		{
			SERVERADDR* pAddr = (SERVERADDR*)pMsg->data;
			PRINT(SUCCESS_GREEN, "找到了服务器地址 %s:%d\n", pAddr->addr, pAddr->nPort);
		}
		else
			PRINT(ERROR_RED, "没有找到服务器 %s 的地址!\n", pMsg->data);
	}
	break;
	default:
		CBaseServer::OnSCMsg(pMsg, datasize);
		break;
	}
}

SERVER_ERROR CServer::AddEnterAccount(UINT nLoginId, UINT nSelCharId, const char* pszAccount, const char* pszName, WORD wIndex)
{
	ENTERGAMESERVER* pEnterInfo = nullptr;
	UINT id = m_EnterObjects.New(&pEnterInfo);
	if (id == 0 || pEnterInfo == nullptr)return SE_SERVERFULL;
	if (!m_Inthash.HAdd(nLoginId, id))
	{
		m_EnterObjects.Del(id);
		return SE_SERVERFULL;
	}
	pEnterInfo->nClientId = id;
	pEnterInfo->nLoginId = nLoginId;
	pEnterInfo->nSelCharId = nSelCharId;
	pEnterInfo->dwEnterTime = timeGetTime();
	strncpy(pEnterInfo->szAccount, pszAccount, 10);
	o_strncpy(pEnterInfo->szName, pszName, 20);
	pEnterInfo->szAccount[10] = 0;
	pEnterInfo->dwSelectCharServerId = wIndex;
	return SE_OK;
}

BOOL CServer::GetEnterInfo(UINT nLoginId, UINT nSelCharId, const char* pszAccount, ENTERGAMESERVER& enterinfo)
{
	UINT id = (UINT)m_Inthash.HGet(nLoginId);
	if (id == 0)return FALSE;
	ENTERGAMESERVER* pEnterInfo = m_EnterObjects.Get(id);
	if (pEnterInfo == nullptr)return FALSE;
	if (pEnterInfo->nLoginId != nLoginId || pEnterInfo->nSelCharId != nSelCharId)return FALSE;
	enterinfo = *pEnterInfo;
	m_EnterObjects.Del(id);
	m_Inthash.HDel(nLoginId);
	return TRUE;
}

void CServer::OnInput(const char* pString)
{
	char szLine[256];
	o_strncpy(szLine, pString, 250);
	xStringsExtracter<16> cmd(szLine, " \t,", " \t");
	if (static_cast<int>(cmd.getCount()) > 0)
	{
		if (_stricmp(cmd[0], "reloadserverconfig") == 0)
		{
			CGameWorld::GetInstance()->LoadServerConfig();
			this->GetIoConsole()->OutPut(SUCCESS_GREEN, "服务器配置文件已重新加载!\n");
		}
		else if (_stricmp(cmd[0], "reloaditem") == 0)
		{
			CItemManager::GetInstance()->ClearItemData();
			CItemManager::GetInstance()->Load(".\\Data\\Config\\BaseItem.csv");
			this->GetIoConsole()->OutPut(SUCCESS_GREEN, "物品配置文件BaseItem.csv已重新加载!\n");
		}
		else if (_stricmp(cmd[0], "reloadmonster") == 0)
		{
			CMonsterManagerEx::GetInstance()->ClearMonsterData();
			CMonsterManagerEx::GetInstance()->LoadMonsters(".\\Data\\Monsters");
			this->GetIoConsole()->OutPut(SUCCESS_GREEN, "怪物配置Monsters下的所有文件已重新加载!\n");
		}
		else if (_stricmp(cmd[0], "reloadskill") == 0) {
			CMagicManager::GetInstance()->ClearMagicData();
			CMagicManager::GetInstance()->LoadMaigc(".\\Data\\Config\\BaseMagic.csv");
			CMagicManager::GetInstance()->LoadMagicExt(".\\Data\\Config\\MagicExt.csv", TRUE);
			CMagicManager::GetInstance()->LoadMaigcskill(".\\Data\\Config\\MagicSkill.xml");
			// 重新加载技能数据后, 需要更新所有在线玩家的技能指针
			CMagicManager::GetInstance()->ReloadAllPlayerSkills();
			this->GetIoConsole()->OutPut(SUCCESS_GREEN, "技能配置文件BaseMagic.csv、MagicExt.csv、MagicSkill.xml已重新加载!\n");
		}
	}
	CBaseServer::OnInput(pString);
}

VOID CServer::KickAll()
{
	CClientObj* pObject = m_ObjectPool.First();
	while (pObject)
	{
		pObject->Disconnect(2000);
		pObject = m_ObjectPool.Next();
	}
}

#include "humanplayermgr.h"
#include "humanplayer.h"
#include "guildmanagerex.h"
#include "sandcity.h"
#include "simpledbconnection.h"

VOID CServer::OnTerminated(BOOL bExcepted)
{
	// 直接使用当前线程（主线程/终端）保存数据, 不创建新线程
	PRINT(STRING_GREEN, "开始存储行会信息...\n");
	CGuildManagerEx::GetInstance()->SaveAll();

	PRINT(STRING_GREEN, "开始存储沙城信息...\n");
	CSandCity::GetInstance()->Save();

	PRINT(STRING_GREEN, "开始存储角色信息...\n");
	CIndexListEx<CHumanPlayer>* pList = CHumanPlayerMgr::GetInstance()->GetPlayerList();
	if (pList == nullptr || pList->GetCount() == 0)
	{
		PRINT(STRING_GREEN, "没有玩家在线, 无需存储.\n");
		return;
	}

	// 获取DBServer地址
	SERVERADDR* pAddr = GetDBAddr(0);
	if (pAddr == nullptr)
	{
		PRINT(ERROR_RED, "无法取得DB服务器的地址, 无法完成数据备份.\n");
		return;
	}

	// 创建临时DB连接（同步方式）
	CSimpleDBConnection dbConn;
	if (!dbConn.Connect(pAddr->addr, pAddr->nPort))
	{
		PRINT(ERROR_RED, "无法连接到数据库服务 %s:%u, 数据备份失败.\n", pAddr->addr, pAddr->nPort);
		return;
	}

	PRINT(SUCCESS_GREEN, "连接到数据库服务, 开始数据备份\n");

	CHumanPlayer* pPlayer = pList->First();
	int nSavedCount = 0;
	alignas(64) static char szBackupBuffer[1024 * 64];

	while (pPlayer)
	{
		// 保存角色变量
		pPlayer->SaveVars();

		// 构建保存数据包
		memset(szBackupBuffer, 0, sizeof(szBackupBuffer));
		xPacket msgpacket(szBackupBuffer, 1024 * 64);
		pPlayer->GetDBInfoPacket(msgpacket);

		// 同步发送到DBServer
		int size = msgpacket.getsize();
		if (size > 0)
		{
			dbConn.Send((LPVOID)msgpacket.getbuf(), size);
			// 等待发送完成（简单处理：短暂延时）
			Sleep(10);
		}

		PRINT(STRING_GREEN, "%s 角色信息已经保存到数据库\n", pPlayer->GetName());
		nSavedCount++;
		pPlayer = pList->Next();
	}

	PRINT(SUCCESS_GREEN, "数据已备份完毕, 共保存 %d 个角色.\n", nSavedCount);
}