#pragma once
#include "xsupport.h"
#include "xindexobject.h"
template <class T>
class xIndexObjectPool
{
public:
	xIndexObjectPool(void)
	{
	}
	virtual ~xIndexObjectPool(void)
	{
	}

	BOOL create(int nMax)
	{
		return m_ObjectPool.Create(nMax);
	}

	T* getObject(UINT id)
	{
		return m_ObjectPool.Get(id);
	}

	T* newObject()
	{
		T* pObject = nullptr;
		UINT id = m_ObjectPool.New(&pObject);
		if (id == 0 || pObject == nullptr)
			return nullptr;
		xIndexObject* pIndexObject = dynamic_cast<xIndexObject*>(pObject);
		if (pIndexObject == nullptr)
		{
			m_ObjectPool.Del(id);
			return nullptr;
		}
		pIndexObject->setId(id);
		return pObject;
	}
	void	deleteObject(T* pObject)
	{
		xIndexObject* pIndexObject = dynamic_cast<xIndexObject*>(pObject);
		if (pIndexObject)
		{
			UINT	id = pIndexObject->getId();
			pIndexObject->Clean();
			m_ObjectPool.Del(id);
		}
	}
protected:
	CIndexListEx<T> m_ObjectPool; // CIndexListEx<T>
};
