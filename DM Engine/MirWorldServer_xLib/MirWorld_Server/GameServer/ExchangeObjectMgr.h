#pragma once
class CHumanPlayer;
class CExchangeObj;

class CExchangeObjectMgr : public xSingletonClass<CExchangeObjectMgr>
{
public:
	CExchangeObjectMgr(void);
	virtual ~CExchangeObjectMgr(void);
	//交易开始
	BOOL BeginExchange(CHumanPlayer* p1, CHumanPlayer* p2);
	//交易结束
	VOID EndExchange(CExchangeObj* pObj);
private:
	static xObjectPool<CExchangeObj> m_xExchangeObjPool;
	static CExchangeObj* newObject() { return m_xExchangeObjPool.newObject(); }
	static VOID deleteObject(CExchangeObj* e) { m_xExchangeObjPool.deleteObject(e); }
};