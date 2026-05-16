#pragma once
#include "eventobject.h"

enum SE_FLAG
{
	SEF_ENTER = 1,
	SEF_LEAVE = 2,
};

class CScriptEvent :
	public CEventObject
{
public:
	CScriptEvent(void);
	virtual ~CScriptEvent(void);
	static CScriptEvent* Create(UINT nMapId, UINT x, UINT y, DWORD dwFlag, const char* pszPage);
	VOID Release();

	VOID OnEnter(CMapObject* pObject);
	VOID OnLeave(CMapObject* pObject);
	VOID OnEnterMap(CLogicMap* pMap);
protected:
	char m_szScriptPage[256];
	DWORD m_dwFlag;
	xListHost<CScriptEvent>::xListNode m_xNode;
	static xListHost<CScriptEvent> m_xList;
	static xObjectPool<CScriptEvent> m_xScriptEventPool;
};