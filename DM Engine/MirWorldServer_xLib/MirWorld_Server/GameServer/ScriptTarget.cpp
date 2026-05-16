#include "StdAfx.h"
#include ".\scripttarget.h"
#include ".\scriptshell.h"
#include ".\scriptelement.h"
#include ".\scriptobject.h"
#include ".\ScriptObjectMgr.h"
#include ".\humanplayer.h"
#include ".\scriptvariable.h"
#include ".\guildex.h"

CScriptTarget::CScriptTarget(void)
{
	m_pCurScriptObject = nullptr;
	m_pCurShell = nullptr;
	m_szToPage[0] = 0;
	m_pInputShell = nullptr;
	m_nInputLength = 0;
}

CScriptTarget::~CScriptTarget(void)
{
}

VOID CScriptTarget::OnInputConfirm(char* pszInputText)
{
	m_xVarList.AddVar("inputtext", pszInputText);
	if (m_pInputShell)
		m_pInputShell->Execute(this, m_szToPage, FALSE);
}

VOID CScriptTarget::SetInputPage(const char* pszPage, CScriptShell* pInputShell, UINT nLength)
{
	m_nInputLength = nLength;
	o_strncpy(m_szToPage, pszPage, 60);
	this->m_pInputShell = pInputShell;
}

VOID CScriptTarget::SetPutItemPage(const char* pszPage, CScriptShell* pPutItemShell)
{
	o_strncpy(m_szToPage, pszPage, 60);
	this->m_pInputShell = pPutItemShell;
}

VOID CScriptTarget::OnPutItem(UINT nShellId, UINT nMakeIndex)
{
	if (m_pInputShell)
		m_pInputShell->Execute(this, m_szToPage, FALSE);
}

VOID CScriptTarget::SetVariableValue(const char* pszVariable, const char* pszValue)
{
	this->m_xVarList.AddVar(pszVariable, (char*)pszValue);
}

VOID CScriptTarget::ClrVariable(const char* pszVariable)
{
	m_xVarList.DelVar(pszVariable);
}

BOOL CScriptTarget::AddVariable(const char* pszVariable, const char* pszValue)
{
	if (m_xVarList.GetVarValue(pszVariable) != nullptr)return FALSE;
	this->m_xVarList.AddVar(pszVariable, (char*)pszValue);
	return TRUE;
}

char* CScriptTarget::GetVariableValue(const char* pszVariable)
{
	//	对于下划线的变量, 解释为Shell所有.
	if (*pszVariable == '_')
		return (m_pCurShell == nullptr ? nullptr : m_pCurShell->GetVariableValue(pszVariable + 1));
	if (*pszVariable == '%')
		return (m_pCurScriptObject == nullptr ? nullptr : m_pCurScriptObject->GetVariableValue(pszVariable + 1));
	char* p = CScriptObjectMgr::GetInstance()->GetVariableValue(pszVariable);
	if (p)return p;
	//else if( *p == '%' )
	//	return "";
	p = m_xVarList.GetVarValue(pszVariable);
	if (p)return p;
	return xVariableProvider::GetVariableValue(pszVariable);
}

CScriptTargetForPlayer::CScriptTargetForPlayer(CHumanPlayer* pPlayer)
{
	this->m_pOwner = pPlayer;
}

VOID CScriptTargetForPlayer::SendPage(CScriptShell* pShell, CScriptView* pView)
{
	m_pOwner->SendPage(pShell, pView);
}

VOID CScriptTargetForPlayer::SendClosePage(CScriptShell* pShell)
{
	m_pOwner->SendClosePage(pShell);
	CScriptTarget::SendClosePage(pShell);
}

static thread_local char g_szVariable[1024];
char* CScriptTargetForPlayer::GetVariableValue(const char* pszVariable)
{
	o_strncpy(g_szVariable, pszVariable, 1020);
	if (g_szVariable[0] == '$')
		pszVariable = GetVariableValue(g_szVariable + 1);
	else
		pszVariable = g_szVariable;
	char* p = strchr(g_szVariable, '.');
	char* pVal = nullptr;
	if (p == nullptr)
		pVal = CScriptTarget::GetVariableValue(pszVariable);
	else
	{
		*p++ = 0;
		if (_stricmp(pszVariable, "guild") == 0)
		{
			if (m_pOwner && m_pOwner->GetGuild())
				pVal = m_pOwner->GetGuild()->GetVariableValue(p);
			else
				pVal = nullptr;
		}
		else
			pVal = (char*)CScriptVariableManager::GetInstance()->GetVariable(pszVariable, p);
		if (pVal == nullptr)
			return "";
		return pVal;
	}
	if (pVal == nullptr)
		return (char*)m_pOwner->GetScriptVarValue(pszVariable);
	return pVal;
}

VOID CScriptTargetForPlayer::SetVariableValue(const char* pszVariable, const char* pszValue)
{
	o_strncpy(g_szVariable, pszVariable, 1020);
	pszVariable = g_szVariable;
	char* p = strchr(g_szVariable, '.');
	if (p == nullptr)
	{
		return CScriptTarget::SetVariableValue(pszVariable, pszValue);
	}
	else
	{
		*p++ = 0;
		if (_stricmp(pszVariable, "guild") == 0)
		{
			if (m_pOwner && m_pOwner->GetGuild())
				return m_pOwner->GetGuild()->SetVariableValue(p, pszValue);
			return;
		}
		CScriptVariableManager::GetInstance()->SetVariable(pszVariable, p, pszValue);
	}
}

VOID CScriptTargetForPlayer::ClrVariable(const char* pszVariable)
{
	o_strncpy(g_szVariable, pszVariable, 1020);
	pszVariable = g_szVariable;
	char* p = strchr(g_szVariable, '.');
	if (p == nullptr)
	{
		return CScriptTarget::ClrVariable(pszVariable);
	}
	else
	{
		*p++ = 0;
		if (_stricmp(pszVariable, "guild") == 0)
		{
			if (m_pOwner && m_pOwner->GetGuild())
				return m_pOwner->GetGuild()->ClrVariable(p);
			return;
		}
		CScriptVariableManager::GetInstance()->DelVariable(pszVariable, p);
	}
}

BOOL CScriptTargetForPlayer::AddVariable(const char* pszVariable, const char* pszValue)
{
	o_strncpy(g_szVariable, pszVariable, 1020);
	pszVariable = g_szVariable;
	char* p = strchr(g_szVariable, '.');
	if (p == nullptr)
	{
		//if( m_xVarList.GetVarValue( pszVariable ) != nullptr )return FALSE;
		return CScriptTarget::AddVariable(pszVariable, pszValue);
	}
	else
	{
		*p++ = 0;
		if (_stricmp(pszVariable, "guild") == 0)
		{
			if (m_pOwner && m_pOwner->GetGuild() && m_pOwner->GetGuild()->GetVariableValue(p) == nullptr)
				return m_pOwner->GetGuild()->AddVariable(p, pszValue);
			return FALSE;
		}
		if (CScriptVariableManager::GetInstance()->GetVariable(pszVariable, p) == nullptr)
			return CScriptVariableManager::GetInstance()->SetVariable(pszVariable, p, pszValue);
		return FALSE;
	}
}

VOID CScriptTargetForPlayer::Clean()
{
	m_xVarList.ClearVars();
}

VOID CScriptTargetForPlayer::SaveVars(const char* pszFilename)
{
	m_xVarList.SaveToFile(pszFilename);
}

VOID CScriptTargetForPlayer::LoadVars(const char* pszFilename)
{
	m_xVarList.LoadFromFile(pszFilename);
}

BOOL CScriptTargetForPlayer::CheckExceedDistance(BOOL boCLose)
{
	CScriptNpc* pNPC = (CScriptNpc*)m_pCurShell;
	if (pNPC && m_pOwner)
	{
		if (boCLose && pNPC->IsNPC() && DISTANCE(pNPC->getX(), pNPC->getY(), m_pOwner->getX(), m_pOwner->getY()) > pNPC->GetDistance())
		{
			SendClosePage(m_pCurShell);
			return TRUE;
		}
	}
	return FALSE;
}