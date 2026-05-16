#pragma once
#include "vmap.h"
#include "physicsmap.h"
#include "MapObject.h"
#include <map>

class CArea;
static DWORD g_dwInterFlag = 0;
static std::vector<std::string> g_szExtraParams;
class CHumanPlayer;
class CAliveObject;
typedef struct tagMineItemList
{
	tagMineItemList()
	{
		FILLSELF(0);
	}
	char szName[32];
	WORD wDuraMax;
	WORD wDuraMin;
	WORD wRate;
	WORD w0;
	tagMineItemList* pNext;
}MineItemList;

class CLogicMap
{
public:
	CLogicMap(void);
	virtual ~CLogicMap(void);
	BOOL LoadMap(const char* pszFilename);
	BOOL IsBlocked(int x, int y);

	BOOL SendMsg(CMapObject* pSender, const char* pszCodedMsg, int size);

	BOOL MoveObject(CMapObject* pObject, int x, int y);

	BOOL AddObject(CMapObject* pObject);
	BOOL RemoveObject(CMapObject* pObject);

	CMapObject* FindObject(int x, int y, e_object_type type);
	CMapObject* FindObjectMT(int x, int y, DWORD dwTypeFlag);
	CMapObject* FindEventObject(int x, int y, int View);

	CMapCellInfo* GetMapCellInfo(UINT x, UINT y)
	{
		if (m_pCellInfo == nullptr || !VerifyPos(x, y))return nullptr;
		return (m_pCellInfo[x + y * m_iWidth]);
	}
	CMapCellInfo** GetCellInfoBase() const { return m_pCellInfo; }
	CMapCellInfo* GetMapCellInfo_Safe(UINT x, UINT y)
	{
		if (m_pCellInfo == nullptr || !VerifyPos(x, y))return nullptr;
		CMapCellInfo** ppCellInfo = m_pCellInfo + x + y * m_iWidth;
		if (*ppCellInfo == nullptr)
		{
			*ppCellInfo = m_xCellInfoPool.newObject();
			(*ppCellInfo)->wEventFlag = 0;
			(*ppCellInfo)->wFlag = 0;
		}
		return (*ppCellInfo);
	}
	VOID RemoveMapCellInfo_Safe(int x, int y);
	int GetDupCount(int x, int y);
	int GetDupCount(int x, int y, e_object_type type);
	int GetDropItemPoint(int x, int y, POINT* ptArray, int ArraySize);
	int GetValidPoint(int x, int y, POINT* ptArray, int ArraySize);
	VOID CheckEnterEvent(CMapObject* pObject, int x, int y);
	VOID CheckLeaveEvent(CMapObject* pObject, int x, int y);
	VOID SetSafeArea(int x, int y, int range);
	VOID SetMapEventFlag(int x, int y, int range, DWORD dwFlag);
	VOID SetMapEventFlagRect(int x, int y, int xrange, int yrange, DWORD dwFlag);
	VOID AddStartPoint(START_POINT* pStartPoint);
	int	GetObjectCount(e_object_type type)const
	{
		int itype = (int)type;
		if (itype < 0 || itype >= OBJ_MAX)return 0;
		return m_iObjectCount[type];
	}
	BOOL IsPhysicsBlocked(int x, int y)
	{
		if (m_pPhysicsMap == nullptr)return TRUE;
		return m_pPhysicsMap->IsBlocked(x, y);
	}
	//在指定位置上是否有某类物体
	BOOL IsObjAtPosition(int x, int y, e_object_type type)
	{
		// 获取指定坐标的地图格子信息
		CMapCellInfo* pInfo = GetMapCellInfo(x, y);
		if (pInfo == nullptr) return FALSE;
		if (pInfo)
		{
			xListHost<CMapObject>::xListNode* pNode = pInfo->m_xObjectList.getHead();
			while (pNode)
			{
				if (pNode->getObject() && pNode->getObject()->GetType() == type)
					return TRUE;
				pNode = pNode->getNext();
			}
		}
		return FALSE;
	}
	VOID CheckEnterCity(CHumanPlayer* pPlayer);
	VOID DecObjectCount(DWORD dwType);
	VOID AddObjectCount(DWORD dwType);
	BOOL LockPos(int x, int y)
	{
		//	当可穿人的选项打开时, 这个层就是空的
		if (m_pLockLayer == nullptr)return TRUE;
		//	已经锁住了, 那么就无法再次锁住
		if (IsLocked(x, y))return FALSE;
		//	计算Lock的精确位置
		int f = y * m_iWidth + x;
		x = f / 32;
		y = f % 32;
		//	x 超出最大的Layer范围, 那么锁定失败
		if (x >= m_iMaxBlockElements)return FALSE;
		m_pLockLayer[x] |= (1 << y);
		return TRUE;
	}
	BOOL UnLockPos(int x, int y)
	{
		if (m_pLockLayer == nullptr)return TRUE;
		//	这里没有被锁住, 那么就无法解锁
		if (!IsLocked(x, y))return FALSE;
		int f = y * m_iWidth + x;
		x = f / 32;
		y = f % 32;
		if (x >= m_iMaxBlockElements)return FALSE;
		m_pLockLayer[x] ^= (m_pLockLayer[x] & (1 << y));
		return TRUE;
	}
	VOID ClearAllMonsters(const char* pszClassName = nullptr);
	UINT CountAllMonsters(const char* pszClassName = nullptr);
public:
	CAliveObject* FindTarget(CAliveObject* pAttacker, UINT x, UINT y, BOOL IsProperTarget = TRUE, BOOL IsDeath = TRUE);
	VOID AddMineItem(const char* pszName, WORD wDMin, WORD wDMax, WORD wRate);
	BOOL GotMineItem(CHumanPlayer* pPlayer);

	const char* GetName() { assert(m_pPhysicsMap != nullptr); return m_pPhysicsMap->GetName(); }
	const char* GetTitle() { return m_pName; }
	int	 GetWidth()const { return m_iWidth; }
	int	 GetHeight()const { return m_iHeight; }
	int GetMusicId()const { return m_iMusicId; }
	CPhysicsMap* GetPhysicsMap() { return m_pPhysicsMap; }
	VOID SetPhysicsMap(CPhysicsMap* pPhysicsMap);

	UINT GetId()const { return m_Id; }
	VOID SetId(UINT id) { m_Id = id; }
	int	GetMiniMap()const { return m_iMiniMap; }
	int GetIndex()const { return m_nIndex; }
	VOID InitLinks();
	FLOAT GetExpFactor()const { return m_fExpFactor; }
	VOID SetExpFactor(FLOAT fFactor) { m_fExpFactor = fFactor; }
	VOID GetObjList(xListHelper<CMapObject>& objlist)
	{
		objlist.setList(&m_xObjList);
	}
	VOID SetFlag(const char* pszFlag);
	VOID UnSetFlag(const char* pszFlag);
	VOID UnSetFlag(e_map_flag flag);
	// 判断地图是否配置某标识, 并返回 dwParam 一个DWORD数 和 szExtraParams 一个字符数组
	BOOL IsFlagSeted(e_map_flag findex, DWORD& dwParam = g_dwInterFlag, std::vector<std::string>& szExtraParams = *(std::vector<std::string>*)&g_szExtraParams);
	BOOL IsLocked(int x, int y)
	{
		//	make sure that before call this , the pos must be right
		//	如果没有这层, 表示所有地方都没有被锁住
		if (m_pLockLayer == nullptr)return FALSE;
		int f = y * m_iWidth + x;
		x = f / 32;
		y = f % 32;
		return (x >= m_iMaxBlockElements || (m_pLockLayer[x] & (1 << y)) != 0);
	}
	static VOID	GetCellInfoInfo(int& used, int& free, int& total)
	{
		used = m_xCellInfoPool.getUsedCount();
		free = m_xCellInfoPool.getFreeCount();
		total = m_xCellInfoPool.getCount();
	}
	// 获取地图随机坐标的函数
	std::pair<int, int> GetRandomCoordinate()
	{
		while (true)
		{
			int x = GetRangeRand(0, m_iWidth - 1);
			int y = GetRangeRand(0, m_iHeight - 1);
			if (!IsPhysicsBlocked(x, y))//判断是否有阻挡
			{
				return std::make_pair(x, y);
			}
		}
	}
	VOID SendAroundMsg(int x, int y, int range, const char* szMsg, int size, CMapObject* pSender = nullptr, BOOL bIncludeSelf = TRUE);
	Weather& GetWeather() { return m_xWeather; }
	BOOL DamageAround(CAliveObject* pAttacker, UINT x, UINT y, UINT nRange, int nDamage, damage_type damagetype, DWORD dwFlag, DWORD dwDelay = 0, BOOL bPushed = TRUE);
	BOOL CureBagStatusAround(CAliveObject* pAttacker, UINT x, UINT y, UINT nRange, std::vector<BYTE>& btArray, UINT nArraySize, std::vector<CAliveObject*>* retTargets = nullptr);
	BOOL AddAllProcess(DWORD dwTypeFlag, e_process ident, DWORD dwParam1 = 0,
		DWORD dwParam2 = 0, DWORD dwParam3 = 0, DWORD dwParam4 = 0,
		DWORD dwDelay = 0, int repeattimes = 0, const char* pszString = nullptr);
private:
	FLOAT m_fExpFactor;
	BOOL AddObjectToPos(int x, int y, CMapObject* pObject);
	BOOL RemoveObjectFromPos(int x, int y, CMapObject* pObject);
	BOOL InitMapCells();
	BOOL VerifyPos(int x, int y)const
	{
		if (x < 0 || x >= m_iWidth)return FALSE;
		if (y < 0 || y >= m_iHeight)return FALSE;
		return TRUE;
	}
private:
	xStatus	m_Flag;
	std::map<int, std::vector<std::string>> m_flagExtraParams;
	int	m_nIndex;
	DWORD* m_pLockLayer;
	int m_iMaxBlockElements;
	CSettingFile m_DataFile;
	CPhysicsMap* m_pPhysicsMap;
	char* m_pName;
	int	m_iWidth;
	int m_iHeight;
	UINT m_Id;
	int	m_iMiniMap;
	int	m_iLinkCount;
	int m_iMusicId;
	xListHost<CMapObject> m_xObjList;
	// MapCell信息
	CMapCellInfo** m_pCellInfo;
	START_POINT* m_pStartPoints[256];
	int	m_iStartPointCount;
	int	m_iObjectCount[OBJ_MAX];
	static xObjectPool<CMapCellInfo> m_xCellInfoPool;
	MineItemList* m_pMineItemList;
	UINT m_nMineRateMax;
	Weather m_xWeather;
private:// 事件系统使用
	//怪物死亡掉落物品
	VOID handleMonsterDropItem(const MonsterDeathEvent& e);
	SubscriptionToken subMonsterDropItemToken_;
};
