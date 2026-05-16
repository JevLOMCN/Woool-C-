#pragma once

class CPoolManager : public xSingletonClass<CPoolManager>
{
public:
	CPoolManager(void);
	virtual ~CPoolManager(void);
	xObjectPool<StringCacheNode>* GetStringCachePool() { return &m_xStringCachePool; }
protected:
	xObjectPool<StringCacheNode> m_xStringCachePool;
};