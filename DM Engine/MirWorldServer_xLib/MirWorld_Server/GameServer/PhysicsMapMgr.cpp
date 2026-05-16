#include "StdAfx.h"
#include ".\physicsmapmgr.h"
#include "physicsmap.h"

CPhysicsMapMgr::CPhysicsMapMgr(void) :
	m_xMapList(TRUE),
	m_bUseCache(FALSE),
	m_szPhysicsCachePath{ 0 },
	m_szPhysicsMapPath{ 0 }
{
}

CPhysicsMapMgr::~CPhysicsMapMgr(void)
{
}

CPhysicsMap* CPhysicsMapMgr::Load(const char* pszName)
{
	char szFilename[1024];
	_makepath(szFilename, nullptr, m_szPhysicsCachePath, pszName, ".map");
	CPhysicsMap* pMap = new CPhysicsMap;
	if (!pMap->LoadCache(szFilename))
	{
		_makepath(szFilename, nullptr, m_szPhysicsMapPath, pszName, ".nmp");
		if (!pMap->LoadMap(szFilename))
		{
			delete pMap;
			return nullptr;
		}
		pMap->SaveCache(m_szPhysicsCachePath);
	}
	m_xMapList.Add(pMap->GetName(), (LPVOID)pMap);
	return pMap;
}

CPhysicsMap* CPhysicsMapMgr::GetPhysicsMapByName(const char* pszMapName)
{
	CPhysicsMap* p = (CPhysicsMap*)m_xMapList.ObjectOf(pszMapName);
	if (p == nullptr)
		p = Load(pszMapName);
	return p;
}

VOID CPhysicsMapMgr::Init(const char* pszPath, const char* pszCachePath)
{
	o_strncpy(this->m_szPhysicsMapPath, pszPath, 1020);
	o_strncpy(this->m_szPhysicsCachePath, pszCachePath, 1020);
	if (!PathIsFolder(m_szPhysicsCachePath))
		m_bUseCache = FALSE;
	else
		m_bUseCache = TRUE;
	if (!m_bUseCache)
	{
		PRINT(ERROR_RED, "地图CACHE路径不可用, CACHE被禁用, 可能导致读取时间过长!\n");
		if (!PathIsFolder(m_szPhysicsMapPath))
		{
			PRINT(ERROR_RED, "物理地图路径不可用, 物理地图无法正常读取!\n");
		}
	}
	else
	{
		PRINT(SUCCESS_GREEN, "地图CACHE功能已启用, 将大大提高地图读取速度!\n");
	}
}
