#include "StdAfx.h"
#include ".\systemscript.h"

CSystemScript::CSystemScript(void)
{
	m_pScriptObject = nullptr;
}

CSystemScript::~CSystemScript(void)
{
}

BOOL CSystemScript::Init(CScriptObject* pScriptObject)
{
	this->m_pScriptObject = pScriptObject;
	return TRUE;
}