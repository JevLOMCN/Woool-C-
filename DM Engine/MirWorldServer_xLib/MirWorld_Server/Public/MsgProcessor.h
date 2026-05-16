#pragma once
#include "inc.h"

class xClientObject;
class CMsgProcessor
{
public:
	virtual void OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size) {}
	virtual void OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize) {}
};