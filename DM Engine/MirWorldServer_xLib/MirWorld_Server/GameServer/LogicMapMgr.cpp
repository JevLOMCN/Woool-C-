#include "StdAfx.h"
#include ".\logicmapmgr.h"
#include "logicmap.h"

CLogicMapMgr::CLogicMapMgr(void)
{
	m_pLoadingMap = nullptr;
	memset(m_pMaps, 0, sizeof(m_pMaps));
}

CLogicMapMgr::~CLogicMapMgr(void)
{
}

VOID CLogicMapMgr::OnFoundFile(const char* pszFilename, UINT nParam)
{
	if (m_pLoadingMap == nullptr)
	{
		//m_pLoadingMap = new CLogicMap;
		UINT id = m_LogicMaps.New(&m_pLoadingMap);
		if (id == 0)
			m_pLoadingMap = nullptr;
		else
			m_pLoadingMap->SetId(id);
	}
	assert(m_pLoadingMap != nullptr);
	//	读取地图
	if (!m_pLoadingMap->LoadMap(pszFilename))
		return;
	//	添加到地图名字哈西表
	if (!m_MapNameHash.HAdd(m_pLoadingMap->GetTitle(), (LPVOID)m_pLoadingMap))
	{
		//	return;
	}
	UINT index = m_pLoadingMap->GetIndex();
	if (index > 0 && index <= MAX_LOGIC_MAP)
		m_pMaps[index - 1] = m_pLoadingMap;
	else 
	{
		m_pLoadingMap = nullptr;
		PRINT(ERROR_RED, "地图索引错误, 地图文件 %s\n", pszFilename);
		return; //错误处理
	}
	m_pLoadingMap = nullptr; //成功后, 将指针设置成空, 让下一个文件重新new一个对象.
}

VOID CLogicMapMgr::Load(const char* pszPath)
{
	StartFind(pszPath, "*.ini");
	//	如果最后一个指针读取失败, 删除最后一个指针.
	if (m_pLoadingMap != nullptr)
		delete m_pLoadingMap;
	for (int i = 0; i < MAX_LOGIC_MAP; i++)
	{
		if (m_pMaps[i] != nullptr)
			m_pMaps[i]->InitLinks();
	}
}

CLogicMap* CLogicMapMgr::GetLogicMapByTitle(const char* pszMapTitle)
{
	return (CLogicMap*)m_MapNameHash.HGet(pszMapTitle);
}
