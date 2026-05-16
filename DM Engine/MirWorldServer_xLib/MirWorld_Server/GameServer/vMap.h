#pragma once
//#include "mapobject.h"

constexpr int MAP_AREA_WIDTH = 1;
constexpr int MAP_AREA_HEIGHT = 1;

class CMapObject;
class CLogicMap;
class CPhysicsMap;

typedef struct t_VisibleObject
{
	t_VisibleObject() :Node(this)
	{
		pObject = nullptr;
		//btVisibleFlag = 0;
	}
	CMapObject* pObject;
	//BYTE	btVisibleFlag;
	//BYTE	btType;
	//DWORD	dwId;
	xListHost<t_VisibleObject>::xListNode Node;
}VISIBLE_OBJECT;

typedef xListHost<CMapObject> CELLOBJECTLIST;
typedef xListHost<CHumanPlayer> CELLHUMANLIST;

class CMapCellInfo
{
public:
	WORD wFlag;
	WORD wEventFlag;
	xListHost<CMapObject> m_xObjectList;
	//xListHost<CHumanPlayer> m_xHumanList;
	CMapCellInfo()
	{
		wFlag = 0;
		wEventFlag = 0;
	}
};

//class CPhysicsMap
//{
//public:
//	virtual BOOL LoadMap( const char * pszFilename ) = 0;
//	virtual BOOL IsBlocked( int x, int y ) = 0;
//	virtual int	GetHeight() = 0;
//	virtual int	GetWidth() = 0;
//	virtual const char * GetName() = 0;
//	virtual BOOL AddRefMap( const CLogicMap *) = 0;
//};
//
//class CLogicMap
//{
//public:
//	virtual BOOL LoadMap( const char * pszFilename ) = 0;
//	virtual e_maptype GetType() = 0;
//	virtual BOOL TestFlag( e_mapflag flag ) = 0;
//	virtual BOOL IsBlocked( int x, int y ) = 0;
//	virtual int	GetWidth() = 0;
//	virtual int	GetHeight() = 0;
//	virtual BOOL SendMsg( CMapObject * pSender, const char * pszCodedMsg, int size ) = 0;
//	virtual CPhysicsMap * GetPhysicsMap() = 0;
//	virtual VOID SetPhysicsMap( CPhysicsMap * pPhysicsMap ) = 0;
//
//	virtual BOOL LockPos( int x, int y ) = 0;
//	virtual BOOL UnLockPos( int x, int y ) = 0;
//
//	virtual const char * GetName() = 0;
//	virtual const char * GetTitle() = 0;
//
//	virtual BOOL MoveObject( CMapObject * pObject, int x, int y ) = 0;
//
//	virtual BOOL AddObject( CMapObject * pObject ) = 0;
//	virtual BOOL RemoveObject( CMapObject * pObject ) = 0;
//	virtual CMapObject * FindObject( int x, int y, e_object_type type ) = 0;
//	virtual CMapObject * FindObjectMT( int x, int y, DWORD dwTypeFlag ) = 0;
//
//	virtual CMapCellInfo * GetMapCellInfo( UINT x, UINT y ) = 0;
//	virtual int GetDupCount( int x, int y ) = 0;
//	virtual int GetDupCount( int x, int y, e_object_type type ) = 0;
//	virtual int	GetDropItemPoint( int x, int y, POINT * ptArray, int ArraySize ) = 0;
//
//	virtual VOID CheckEnterEvent( CMapObject * pObject, int x, int y ) = 0;
//	virtual VOID CheckLeaveEvent( CMapObject * pObject, int x, int y ) = 0;
//
//	virtual int	GetMiniMap() = 0;
//	virtual int GetIndex() = 0;
//
//	virtual VOID SetSafeArea( int x, int y, int range ) = 0;
//	virtual VOID AddStartPoint( START_POINT * pStartPoint ) = 0;
//	virtual int	GetObjectCount( e_object_type type ) = 0;
//	virtual BOOL IsPhysicsBlocked( int x, int y ) = 0;
//};
