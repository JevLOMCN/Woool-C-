#include "StdAfx.h"
#include "mapobject.h"
#include "vmap.h"
#include "logicmap.h"

CMapObject::CMapObject(void)
{
	m_pMap = nullptr;
	m_Mapid = 0;
	m_Id = 0;
	m_wX = 0, m_wY = 0;
	m_wInvisibleLevel = 0;
	m_dwRefenceCount = 0;
	m_dwUpdateKey = 0;
	int i = 0;
	for (i = 0; i < LNI_MAX; i++)
	{
		m_DLinkNodes[i] = new xListHost<CMapObject>::xListNode(this);
	}
}

CMapObject::~CMapObject(void)
{
}

VOID CMapObject::Clean()
{
	m_wX = 0, m_wY = 0;
	for (int i = 0; i < LNI_MAX; i++)
	{
		if (m_DLinkNodes[i])
			m_DLinkNodes[i]->Leave();
	}
	m_pMap = nullptr;
	m_Mapid = 0;
	m_wInvisibleLevel = 0;
	m_dwRefenceCount = 0;
	m_dwUpdateKey = 0;
	m_Id = 0;
}

VOID CMapObject::setXY(WORD x, WORD y)
{
	if (x != m_wX || y != m_wY)
		OnSetPos(m_wX, m_wY, x, y);
	m_wX = x, m_wY = y;
}

int CMapObject::EncodeMsg(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	MIRMSGHEADER header(dwFlag, wCmd, w1, w2, w3);
	int	codedsize = 1;
	buffer[0] = '#';
	codedsize += _CodeGameCode((BYTE*)&header, sizeof(header), (BYTE*)buffer + codedsize);
	if (lpdata != nullptr)
	{
		if (datasize < 0)
			datasize = (int)strlen((char*)lpdata);
		codedsize += _CodeGameCode((BYTE*)lpdata, datasize, (BYTE*)buffer + codedsize);
	}
	buffer[codedsize++] = '!';
	buffer[codedsize] = 0;
	return codedsize;
}

int CMapObject::EncodeMsgRaw(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	MIRMSGHEADER header(dwFlag, wCmd, w1, w2, w3);
	int	codedsize = 1;
	buffer[0] = '#';
	// Header 部分使用编码加密
	codedsize += _CodeGameCode((BYTE*)&header, sizeof(header), (BYTE*)buffer + codedsize);
	if (lpdata != nullptr)
	{
		if (datasize < 0)
			datasize = (int)strlen((char*)lpdata);
		// Data 部分直接拷贝，不使用编码
		memcpy(buffer + codedsize, lpdata, datasize);
		codedsize += datasize;
	}

	buffer[codedsize++] = '!';
	buffer[codedsize] = 0;
	return codedsize;
}

BOOL CMapObject::InSafeArea()
{
	if (m_pMap == nullptr)return TRUE;
	CMapCellInfo* pInfo = m_pMap->GetMapCellInfo(getX(), getY());
	if (pInfo)
	{
		if (pInfo->wFlag & MAPCELLFLAG_NOPK)
			return TRUE;
	}
	return FALSE;
}

BOOL CMapObject::InCityArea()
{
	if (m_pMap == nullptr)return TRUE;
	CMapCellInfo* pInfo = m_pMap->GetMapCellInfo(getX(), getY());
	if (pInfo)
	{
		if (pInfo->wEventFlag & EVENTFLAG_CITYEVENT)
			return TRUE;
	}
	return FALSE;
}

BOOL CMapObject::InWarArea()
{
	if (m_pMap == nullptr)return TRUE;
	CMapCellInfo* pInfo = m_pMap->GetMapCellInfo(getX(), getY());
	if (pInfo)
	{
		if (pInfo->wEventFlag & EVENTFLAG_WAREVENT)
			return TRUE;
	}
	return FALSE;
}

thread_local CMapObject::MessageCache CMapObject::s_msgCache;
VOID CMapObject::SmartEncodeMessage(CHAR* pBuffer, int& size,
	DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata, int datasize)
{
	if (datasize == -1 && lpdata)
		datasize = (int)strlen((char*)lpdata);
	switch (wCmd) // 排除一些封包不做缓存
	{
	case 5:case 6:case 7:case 9:case 10:case 11:case 12:case 13:case 0x262: case 0x263:
	{
		size = EncodeMsg(pBuffer, dwFlag, wCmd, w1, w2, w3, lpdata, datasize);
		return;
	}
	break;
	}
	if (datasize > MSG_LPDATA_SIZE) // 如果消息数据部分大于了缓存大小, 就直接编码.
	{
		size = EncodeMsg(pBuffer, dwFlag, wCmd, w1, w2, w3, lpdata, datasize);
		return;
	}
	// 构建缓存键
	MessageCacheKey key{ dwFlag, wCmd, w1, w2, w3, datasize };
	// 在哈希表中查找缓存
	const CachedMessageNode* pCached = s_msgCache.Find(key, lpdata);
	// 如果命中缓存, 直接复制
	if (pCached != nullptr)
	{
		memcpy(pBuffer, pCached->buffer.data(), pCached->encodedSize);
		size = pCached->encodedSize;
		return;
	}
	// 未命中缓存, 执行编码
	size = EncodeMsg(pBuffer, dwFlag, wCmd, w1, w2, w3, lpdata, datasize);
	// 存入缓存（哈希表+LRU自动管理）
	s_msgCache.Put(key, lpdata, pBuffer, size);
}