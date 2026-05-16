#pragma once
#include "scriptshell.h"

class CSystemScript :
	public CScriptShell,
	public xSingletonClass<CSystemScript>
{
public:
	CSystemScript(void);
	virtual ~CSystemScript(void);
	BOOL Init(CScriptObject* pScriptObject);
};