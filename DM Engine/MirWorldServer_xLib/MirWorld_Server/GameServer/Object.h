#pragma once
class CObject
{
public:
	CObject(void);
	virtual ~CObject(void);
	virtual VOID Clean();
	virtual VOID Update() {}
	UINT GetId()const { return m_nIdent; }
	VOID SetId(UINT id) { m_nIdent = id; }
	UINT Addref() { return ++m_nRefCount; }
	UINT Decref() { assert(m_nRefCount != 0); return --m_nRefCount; }
protected:
	UINT m_nIdent;
	UINT m_nRefCount;
};