#include "StdAfx.h"
#include ".\npcmanager.h"
#include "scriptnpc.h"
#include "scriptobjectmgr.h"
#include "gameworld.h"
#include "server.h"
#include "logicmap.h"
#include "logicmapmgr.h"

CNpcManager::CNpcManager(void)
{
	m_ScriptNpcs.Create(2048); // 2048个NPC
	m_xQNpcs.create(2048);
}

CNpcManager::~CNpcManager(void)
{
}

CScriptNpc* CNpcManager::AddNpc(const char* pszString)
{
	char szBuffer[1024];
	o_strncpy(szBuffer, pszString, 1023);
	//NPC名字/DataId/NPC外观/逻辑地图ID参照(正常地图与逻辑地图转换)/坐标X/坐标Y/是否可以对话(0/1)=1表示可以对话/与玩家距离/脚本对应的文本名字/购买百分比/出售百分比 
	char* Params[11];
	int	nParam = 0;
	nParam = SearchParam(szBuffer, Params, 11, ',');
	int x, y, dbid, mapid, istalk, nDistance, view;
	UINT id = 0;
	if (nParam < 9)return nullptr;
	CScriptObject* pObject = CScriptObjectMgr::GetInstance()->GetScriptObject(Params[8]);
	if (pObject == nullptr)return nullptr;
	dbid = StringToInteger(Params[1]);
	view = StringToInteger(Params[2]);
	mapid = StringToInteger(Params[3]);
	x = StringToInteger(Params[4]);
	y = StringToInteger(Params[5]);
	istalk = StringToInteger(Params[6]);
	nDistance = StringToInteger(Params[7]);
	if (nDistance == 0) nDistance = 8;
	CScriptNpc* pLoadingNpc = nullptr;
	id = m_ScriptNpcs.New(&pLoadingNpc);
	if (id == 0 || pLoadingNpc == nullptr) return nullptr;
	id = (id & 0xffffff) | (OBJ_NPC << 24);
	pLoadingNpc->SetId(id);
	if (nParam > 9 && Params[9][0] != '\0')
	{
		FLOAT fPercent = (float)abs(StringToInteger(Params[9])) / 100;
		pLoadingNpc->SetBuyPercent(fPercent);
		if (nParam > 10 && Params[10][0] != '\0')
		{
			FLOAT fPercent = (float)abs(StringToInteger(Params[10])) / 100;
			pLoadingNpc->SetSellPercent(fPercent);
		}
	}
	if (!pLoadingNpc->Init(dbid, Params[0], view, x, y, mapid, pObject))
	{
		m_ScriptNpcs.Del(id);
		return nullptr;
	}
	pLoadingNpc->SetTalk(istalk);
	pLoadingNpc->SetDistance(nDistance);
	m_xQNpcs.push(pLoadingNpc);
	CGameWorld::GetInstance()->AddMapObject(pLoadingNpc);
	DPRINT(KEYWORD_PINK, "NPC %s 进入世界在(%d)(%d,%d)\n", Params[0], mapid, x, y);
	return pLoadingNpc;
}

BOOL CNpcManager::Load(const char* pszFilename)
{
	CStringFile sf(pszFilename);
	sf.MakeDeflate();
	int i = 0;
	//#name/id/view/mapid/x/y/istalk/scriptfile
	//测试NPC/1/0/477/222/1/firstlogin
	for (i = 0; i < sf.GetLineCount(); i++)
	{
		if (*sf[i] == '#')continue;
		AddNpc(sf[i]);
	}
	return TRUE;
}

VOID CNpcManager::Update()
{
	CScriptNpc* pNpc = m_xQNpcs.pop();
	if (pNpc)
	{
		pNpc->Update();
		m_xQNpcs.push(pNpc);
	}
}

BOOL CNpcManager::AddDynamicNpc(UINT nIdent, const char* pszName, UINT nView, UINT mapid, UINT x, UINT y, const char* pszScript)
{
	CScriptObject* pObject = CScriptObjectMgr::GetInstance()->GetScriptObject(pszScript);
	if (pObject == nullptr)return FALSE;
	CLogicMap* pMap = CLogicMapMgr::GetInstance()->GetLogicMapById(mapid);
	if (pMap == nullptr)return FALSE;

	CScriptNpc* npc = newNpc();
	if (npc == nullptr)return FALSE;
	if (!npc->Init(nIdent, pszName, nView, x, y, mapid, pObject))
	{
		deleteNpc(npc);
		return FALSE;
	}
	if (!m_xDynamicNpcList.addNode(npc->GetLinkNode(LNI_WORLD)))
	{
		deleteNpc(npc);
		return FALSE;
	}
	if (!pMap->AddObject(npc))
	{
		m_xDynamicNpcList.removeNode(npc->GetLinkNode(LNI_WORLD));
		deleteNpc(npc);
		return FALSE;
	}
	DPRINT(KEYWORD_PINK, "NPC %s 进入世界在(%d)(%d,%d)\n", pszName, mapid, x, y);
	return TRUE;
}

BOOL CNpcManager::RemoveDynamicNpc(UINT nIdent)
{
	xListHost<CMapObject>::xListNode* pNode = m_xDynamicNpcList.getHead();
	while (pNode)
	{
		if (pNode->getObject() &&
			((CScriptNpc*)pNode->getObject())->GetStoreId() == (nIdent | 0x70000000))
		{
			CScriptNpc* pNpc = (CScriptNpc*)pNode->getObject();
			m_xDynamicNpcList.removeNode(pNode);
			if (pNpc->GetMap())
				pNpc->GetMap()->RemoveObject(pNpc);
			pNpc->SaveItems();
			deleteNpc(pNpc);
			return TRUE;
		}
		pNode = pNode->getNext();
	}
	return FALSE;
}

CScriptNpc* CNpcManager::GetDynamicNpc(UINT nIdent)
{
	xListHost<CMapObject>::xListNode* pNode = m_xDynamicNpcList.getHead();
	while (pNode)
	{
		if (pNode->getObject() &&
			((CScriptNpc*)pNode->getObject())->GetStoreId() == (nIdent | 0x70000000))
			return (CScriptNpc*)pNode->getObject();

		pNode = pNode->getNext();
	}
	return nullptr;
}

CScriptNpc* CNpcManager::newNpc()
{
	CScriptNpc* npc = nullptr;
	UINT id = m_ScriptNpcs.New(&npc);
	if (id == 0 || npc == nullptr)return nullptr;
	id = (id & 0xffffff) | (OBJ_NPC << 24);
	npc->SetId(id);
	return npc;
}

VOID CNpcManager::deleteNpc(CScriptNpc* pNpc)
{
	UINT id = pNpc->GetId() & 0xffffff;
	pNpc->Clean();
	m_ScriptNpcs.Del(id);
}