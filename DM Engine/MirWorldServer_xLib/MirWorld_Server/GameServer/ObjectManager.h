#pragma once
class CObject;

class CObjectManager
{
public:
	CObjectManager(void);
	virtual ~CObjectManager(void);
	virtual CObject* getObject(UINT id) { return nullptr; }
	virtual VOID deleteObject(CObject* pObject) {}
	virtual CObject* newObject() { return nullptr; }
};
