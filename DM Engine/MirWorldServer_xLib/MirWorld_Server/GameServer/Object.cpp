#include "StdAfx.h"
#include ".\Object.h"

CObject::CObject(void):
	m_nIdent(0),
	m_nRefCount(0)
{
}

CObject::~CObject(void)
{
}

VOID CObject::Clean()
{
	m_nIdent = 0;
}