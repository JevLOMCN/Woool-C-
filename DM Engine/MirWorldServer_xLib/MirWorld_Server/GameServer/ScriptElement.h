#pragma once
#include "ScriptFile.h"
static constexpr size_t MAX_CALL_PARAMS = 16;// @carryto&159&123  这种写法, 最多支持多少个参数., 使用 $_param0 获取指定参数

enum e_parse_state
{
	EPS_ACT,
	EPS_SAY,
	EPS_JSON,
	EPS_FLASH
};

class CScriptShell;
class CScriptTarget;
class CScriptView;
class CScriptObject;

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CScriptElement
{
public:
	CScriptElement()
	{
		m_pNext = nullptr;
		m_pPrev = nullptr;
		m_nLineNumber = 0;
		m_pViewString = nullptr;
	}
	virtual ~CScriptElement() { Destroy(); }
	virtual BOOL Parse(CScriptFile& file) { m_nLineNumber = file.GetCurrentLineNumber(); return TRUE; }
	//	执行接口
	//	需要一个SHELL和一个TARGET, 还需要一个PAGEVIEW
	virtual BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView) { return FALSE; }
	virtual VOID Destroy() {}

	VOID addTail(CScriptElement* pTail)
	{
		CScriptElement* p = this;
		while (p->getNext())
		{
			p = p->getNext();
		}
		p->setNext(pTail);
		pTail->setPrev(p);
		pTail->setNext(nullptr);
	}
	VOID delToTail()
	{
		CScriptElement* p = this;
		CScriptElement* pNext = nullptr;
		while (p)
		{
			pNext = p->getNext();
			delete p;
			p = pNext;
		}
	}
	VOID setNext(CScriptElement* pNext) { m_pNext = pNext; }
	VOID setPrev(CScriptElement* pPrev) { m_pPrev = pPrev; }
	CScriptElement* getNext() { return m_pNext; }
	CScriptElement* getPrev() { return m_pPrev; }
	const char* GetViewString() { return m_pViewString; }
	static xPacket* NewParamStack()
	{
		xPacket* packet = m_xParamStackPool.newObject();
		if (packet == nullptr)
			return nullptr;
		if (packet->getbuf() == nullptr)
		{
			if (!packet->create(65536))
			{
				m_xParamStackPool.deleteObject(packet);
				return nullptr;
			}
		}
		else
			packet->clear();
		return packet;
	}
	static VOID	DeleteParamStack(xPacket* pPacket)
	{
		pPacket->clear();
		m_xParamStackPool.deleteObject(pPacket);
	}
protected:
	static xObjectPool<xPacket> m_xParamStackPool;
	CScriptElement* m_pNext;
	CScriptElement* m_pPrev;
	UINT m_nLineNumber;
	char* m_pViewString;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CParamStackHelper
{
public:
	CParamStackHelper()
	{
		this->m_pPacket = CScriptElement::NewParamStack();
	}
	~CParamStackHelper()
	{
		if (m_pPacket)
			CScriptElement::DeleteParamStack(m_pPacket);
	}
	xPacket* getPacket() { return m_pPacket; }
	BOOL push(LPVOID lp, int si) { if (m_pPacket)return m_pPacket->push(lp, si); return FALSE; }
	char* getbuf() { if (m_pPacket)return (char*)m_pPacket->getbuf(); return nullptr; }
	char* getfreebuf() { if (m_pPacket)return (char*)m_pPacket->getfreebuf(); return nullptr; }
	int	getsize() { if (m_pPacket)return m_pPacket->getsize(); return 0; }
	int getfreesize() { if (m_pPacket)return m_pPacket->getfreesize(); return 0; }
	void clear() { if (m_pPacket)m_pPacket->clear(); }
protected:
	xPacket* m_pPacket;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		NPC商人贩卖的物品列表
//		解析脚本中的[Goods]段
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CSe_Goods : public CScriptElement
{
public:
	CSe_Goods() { m_pGoodsList = nullptr; }
	virtual ~CSe_Goods() { Destroy(); }
	BOOL Parse(CScriptFile& file);
	VOID Destroy();
	bool hasFourchar(const char* szName);//判断是否有4个负数, 来表示数组包含两个中文
	Goods* getList() { return m_pGoodsList; }
protected:
	VOID AddGoods(Goods* pGoods);
	Goods* m_pGoodsList;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：脚本Page页
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
enum Page_Access_Right
{
	PAR_PUBLIC,
	PAR_PROTECTED,
	PAR_PRIVATE,
};
class CSe_Page : public CScriptElement
{
public:
	CSe_Page(void);
	virtual ~CSe_Page(void) { Destroy(); }
	BOOL Parse(CScriptFile& file);
	BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	Page_Access_Right getAccessRight() { return m_AccessRight; }
	const char* getName() { return m_szName; }
	CScriptObject* getObject() { return m_pObject; }
	VOID setObject(CScriptObject* pObject) { m_pObject = pObject; }
	VOID Destroy();
protected:
	VOID AddScriptElement(CScriptElement* pElement);
	Page_Access_Right m_AccessRight;
	char m_szName[64];
	CScriptElement* m_pElements;
	CScriptObject* m_pObject;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：Act逻辑
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CSe_NormalAct : public CScriptElement
{
public:
	CSe_NormalAct()
	{
		m_nCommand = 0;
		m_nParamCount = 0;
		m_pParams = nullptr;
		m_dwResult = 0;
		m_fnProc = nullptr;
	}
	virtual ~CSe_NormalAct() { Destroy(); }
	BOOL Parse(CScriptFile& file);
	BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	//VOID setResult( DWORD dwResult ){ m_dwResult = dwResult;}
	DWORD getResult()const { return m_dwResult; }
	VOID Destroy();
	//UINT getCommand(){ return m_nCommand;}
	//ScriptParamEx * getParams(){ return m_pParams;}
	//DWORD	getParamCount(){ return m_nParamCount;}
protected:
	DWORD m_dwResult;
	UINT m_nCommand;
	DWORD m_nParamCount;
	ScriptParamEx* m_pParams;
	fnCommandProc m_fnProc;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：Say逻辑
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CSe_NormalSay : public CScriptElement
{
public:
	CSe_NormalSay()
	{
		m_pSayWords = nullptr;
	}
	virtual ~CSe_NormalSay() { Destroy(); }
	BOOL Parse(CScriptFile& file);
	BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	VOID Destroy();
protected:
	char* m_pSayWords;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：If逻辑
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
enum if_parse_state
{
	IPS_IF,
	IPS_TRUE,
	IPS_FALSE,
};
class CSe_IfStatement : public CScriptElement
{
public:
	CSe_IfStatement()
	{
		m_pCondition = nullptr;
		m_pTrueElements = nullptr;
		m_pFalseElements = nullptr;
		m_pElseIfStatement = nullptr;
		m_bNot = FALSE;
		m_bOne = FALSE;
	}
	virtual ~CSe_IfStatement() { Destroy(); }
	BOOL Parse(CScriptFile& file);
	BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	VOID Destroy();
protected:
	BOOL CheckConditionList(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	CSe_NormalAct* m_pCondition;
	CScriptElement* m_pTrueElements;
	CScriptElement* m_pFalseElements;
	CSe_IfStatement* m_pElseIfStatement;
	BOOL m_bNot;
	BOOL m_bOne;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：结合Switch 的 Case 逻辑
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CSe_CaseBlock : public CScriptElement
{
public:
	CSe_CaseBlock()
	{
		m_nCode = 0;
		m_pElements = nullptr;
	}
	virtual ~CSe_CaseBlock() { Destroy(); }
	BOOL Parse(CScriptFile& file);
	BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	UINT getCode()const { return m_nCode; }
	VOID Destroy();
protected:
	UINT m_nCode;
	CScriptElement* m_pElements;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：Switch逻辑
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
enum switch_parse_state
{
	SPS_SWITCH,
	SPS_CASESAY,
	SPS_CASEACT,
	SPS_DEFAULTSAY,
	SPS_DEFAULTACT,
};
class CSe_SwitchStatement : public CScriptElement
{
public:
	CSe_SwitchStatement()
	{
		m_nCaseBlocks = 0;
		m_pBranchSource = nullptr;
		m_pCaseBlocks = nullptr;
		m_pDefaultBlock = nullptr;
	}
	virtual ~CSe_SwitchStatement() { Destroy(); }
	BOOL Parse(CScriptFile& file);
	BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	VOID Destroy();
protected:
	UINT m_nCaseBlocks;
	CSe_NormalAct* m_pBranchSource;
	CSe_CaseBlock** m_pCaseBlocks;
	CSe_CaseBlock* m_pDefaultBlock;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：JSON控制语句
//		注释：支持 #json 后面跟数字参数, 如 #json123、#json456 等
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CSe_JsonStatement : public CScriptElement
{
public:
	CSe_JsonStatement()
	{
		m_nType = 0;
		m_pWords = nullptr;
	}
	virtual ~CSe_JsonStatement() { Destroy(); }
	BOOL Parse(CScriptFile& file);
	BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	VOID Destroy();
protected:
	UINT m_nType;
	char* m_pWords;
};


//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		描述：FLASH解析
//		注释：
//----------------------------------------------------------------------------------------------------------------------------------------------------------
class CSe_FlashStatement : public CScriptElement
{
public:
	CSe_FlashStatement()
	{
		m_nType = 0;
		m_pWords = nullptr;
	}
	virtual ~CSe_FlashStatement() { Destroy(); }
	BOOL Parse(CScriptFile& file);
	BOOL Execute(CScriptShell* pShell, CScriptTarget* pTarget, CScriptView* pView);
	VOID Destroy();
protected:
	UINT m_nType;
	char* m_pWords;
};