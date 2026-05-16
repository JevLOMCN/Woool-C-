#include "StdAfx.h"
#include ".\cmdproc.h"
#include "sandcity.h"
#include "humanplayer.h"
#include "itemmanager.h"
#include "StringListManager.h"
#include "LogicMapMgr.h"
#include "logicmap.h"
#include "monstergenmanager.h"
#include "GroupObject.h"
#include "topmanager.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//		ÃèÊö£º
//		×¢ÊÍ£º
//----------------------------------------------------------------------------------------------------------------------------------------------------------
CCommandManager::CCommandManager() :m_xCommandList(TRUE)
{
}

BOOL CCommandManager::AddCommand(const char* pszCommand, fnCommandProc proc)
{
	LPVOID lpProc = this->m_xCommandList.ObjectOf(pszCommand);
	if (lpProc != nullptr)return FALSE;
	return (m_xCommandList.Add(pszCommand, (LPVOID)proc) != -1);
}

fnCommandProc CCommandManager::GetCommandProc(const char* pszCommand)
{
	return (fnCommandProc)m_xCommandList.ObjectOf(pszCommand);
}

BOOL CCommandManager::ChangeCommandName(const char* pszCommand, const char* pszNewName)
{
	LPVOID lpProc = this->m_xCommandList.ObjectOf(pszCommand);
	if (lpProc)
	{
		if (m_xCommandList.Add(pszNewName, lpProc) != -1)
		{
			m_xCommandList.Delete(pszCommand);
			return TRUE;
		}
	}
	return FALSE;
}