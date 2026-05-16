#pragma once
#include "findfile.h"

class CScriptNpc;
class CMapObject;
class CNpcManager : public xSingletonClass<CNpcManager>
{
public:
	CNpcManager(void);
	virtual ~CNpcManager(void);
	BOOL Load(const char* pszFilename);

	CScriptNpc* AddNpc(const char* pszString);

	CScriptNpc* GetScriptNpcById(UINT id)
	{
		return m_ScriptNpcs.Get(id & 0xffffff);
	}
	NpcGoodsList* AllocGoodsList()
	{
		return m_GoodsLists.newObject();
	}
	VOID FreeGoodsList(NpcGoodsList* p)
	{
		memset(p, 0, sizeof(NpcGoodsList));
		m_GoodsLists.deleteObject(p);
	}
	NpcGoodsItemList* AllocGoodsItemList()
	{
		return m_GoodsItemLists.newObject();
	}
	VOID FreeGoodsItemList(NpcGoodsItemList* p)
	{
		memset(p, 0, sizeof(NpcGoodsItemList));
		m_GoodsItemLists.deleteObject(p);
	}
	int	getCount()
	{
		return m_ScriptNpcs.GetCount();
	}
	VOID Update();
	BOOL AddDynamicNpc(UINT nIdent, const char* pszName, UINT nView, UINT mapid, UINT x, UINT y, const char* pszScript);
	BOOL RemoveDynamicNpc(UINT nIdent);
	CScriptNpc* GetDynamicNpc(UINT nIdent);
private:
	CScriptNpc* newNpc();
	VOID deleteNpc(CScriptNpc* pNpc);
	xListHost<CMapObject> m_xDynamicNpcList;
	xPtrQueue<CScriptNpc> m_xQNpcs;
	CIndexListEx<CScriptNpc> m_ScriptNpcs;
	xObjectPool<NpcGoodsList> m_GoodsLists;
	xObjectPool<NpcGoodsItemList> m_GoodsItemLists;
};