#ifndef _INC_H_
#define _INC_H_

// 根据平台定义正确的宏
#ifdef _WIN64
#define WIN64_LEAN_AND_MEAN
#else
#define WIN32_LEAN_AND_MEAN
#endif

//#define ASSERT()

#include <winsock2.h>

#include <windows.h>
#pragma comment( lib, "ws2_32.lib" )
#include <mmsystem.h>
#pragma comment( lib, "winmm.lib")

#include <stdio.h>
#include <stdlib.h>
#include "..\..\MirPublic\MirDefine.h"

//#define /*USE_X_LIB*/

//#ifndef USE_X_LIB
//#include "support.h"
//#else
#include "..\xlib\xlib.h"
//#endif
typedef union tagServerId
{
	struct
	{
		BYTE	bType;	//	服务器类型
		BYTE	bGroup;	//	服务器组
		BYTE	bId;	//	服务器唯一标识
		BYTE	bIndex;	//	服务器在服务器中心的注册顺序(由服务器中心负责填写)
	};
	DWORD 	dwId;
}ServerId;

enum	servertype
{
	ST_UNKNOWN,
	ST_SERVERCENTER,
	ST_DATABASESERVER,
	ST_LOGINSERVER,
	ST_SELCHARSERVER,
	ST_GAMESERVER_,
	ST_LOGINSERVER_,
	ST_GAMESERVER,
};


inline	servertype GetServerTypeByName( const char * pszName )
{
	if( pszName == nullptr )return ST_UNKNOWN;
	if(_stricmp( pszName, "servercenter" ) == 0 )return ST_SERVERCENTER;
	if(_stricmp( pszName, "databaseserver" ) == 0 )return ST_DATABASESERVER;
	if(_stricmp( pszName, "loginserver" ) == 0 )return ST_LOGINSERVER;
	if(_stricmp( pszName, "selcharserver" ) == 0 )return ST_SELCHARSERVER;
	if(_stricmp( pszName, "gameserver" ) == 0 )return ST_GAMESERVER;
	return ST_UNKNOWN;
}

inline const char * GetServerTypeNameByType( servertype type )
{
	switch( type )
	{
	case	ST_SERVERCENTER:
		return "服务器中心";
	case	ST_DATABASESERVER:
		return "数据库服务器";
	case	ST_LOGINSERVER:
		return "登陆服务器";
	case	ST_SELCHARSERVER:
		return "角色服务器";
	case	ST_GAMESERVER:
		return "游戏世界服务器";
	}
	return "未知的服务器类型";
}

typedef struct tagSERVERADDR
{
	tagSERVERADDR()
	{
		memset( this, 0, sizeof( *this));
	}
	char	addr[16];
	DWORD	nPort;
	void	SetAddr( const char * pAddr )
	{
		strncpy( addr, pAddr, 16 );
		addr[15] = 0;
	}

}SERVERADDR;


typedef struct tagREGISTER_SERVER_INFO
{
	char	szName[64];
	ServerId	Id;
	SERVERADDR	addr;
}REGISTER_SERVER_INFO;

typedef struct tagREGISTER_SERVER_RESULT
{
	ServerId	Id;
	int			nDbCount;
	SERVERADDR	DbAddr[2];
}REGISTER_SERVER_RESULT;

typedef struct tagFINDSERVER_RESULT
{
	tagFINDSERVER_RESULT()
	{
		memset( this, 0, sizeof(*this));
	}
	SERVERADDR	addr;
	ServerId	Id;
}FINDSERVER_RESULT;

typedef enum	e_servererror
{
	SE_FAIL,
	SE_OK,
	SE_RS_INVALIDTYPE,
	SE_SERVERFULL,
	SE_INVALIDPARAM,
	SE_ALLOCMEMORYFAIL,
	SE_ODBC_SQLALLOCHANDLEFAIL,
	SE_ODBC_SQLSETENVATTRFAIL,
	SE_ODBC_SQLCONNECTFAIL,
	SE_ODBC_SQLDISCONNECTFAIL,
	SE_ODBC_SQLEXECDIRECTFAIL,
	SE_ODBC_SQLNUMRESULTCOLSFAIL,//NumResultCols
	SE_ODBC_SQLDESCRIBECOLFAIL,//DescribeCol
	SE_ODBC_SQLBINDCOLFAIL,
	SE_ODBC_SQLFETCHFAIL,
	SE_ODBC_SQLPREPAREFAIL,
	SE_ODBC_SQLEXECUTEFAIL,
	SE_DB_NOMOREDATA,
	SE_DB_NOTINITED,
	SE_LOGIN_ACCOUNTEXIST,
	SE_LOGIN_ACCOUNTINGAME,
	SE_LOGIN_TIMEDELAY,
	SE_LOGIN_ACCOUNTNOTEXIST,
	SE_LOGIN_PASSWORDERROR,
	SE_SELCHAR_CHAREXIST,
	SE_SELCHAR_NOTEXIST,
	SE_NOTINITLIZED,
	SE_SERVERNOTFOUND,
	SE_REG_INVALIDACCOUNT,
	SE_REG_INVALIDPASSWORD,
	SE_REG_INVALIDNAME,
	SE_REG_INVALIDBIRTHDAY,
	SE_REG_INVALIDPHONENUMBER,
	SE_REG_INVALIDMOBILEPHONE,
	SE_REG_INVALIDQUESTION,
	SE_REG_INVALIDANSWER,
	SE_REG_INVALIDIDCARD,
	SE_REG_INVALIDEMAIL,
	SE_CREATECHARACTER_INVALID_CHARNAME,
}SERVER_ERROR;

enum	loginseverstate
{
	LST_NOTVERIFIED,
	LST_SELECTSERVER,
};

enum	selcharserverstate
{
	SCS_NOTVERIFIED, // 未验证状态
	SCS_WAITINGFORVERIFIED, // 等待账号验证状态
	SCS_VERIFIED, // 账号验证通过状态
	SCS_SELECTED, // 角色选择状态
};

enum	gameserverusermode
{
	GSUM_NOTVERIFIED,
	GSUM_WAITINGDBINFO,
	GSUM_WAITINGCONFIRM,
	GSUM_VERIFIED,
};


#define ID_SERVERCENTERCONNECTION	100032
#define ID_DBSERVERCONNECTIONSTART	100033


enum	MAS_SENDTYPE
{
	MST_SINGLE,	//	发给一个单个服务器, 参数为服务器ｉｎｄｅｘ
	MST_GROUP,	//	发给一个服务器组, 参数为服务器组号
	MST_TYPE,	//	发给一类服务器, 参数为服务器类型号
};

typedef struct tagENTERGAMESERVER
{
	tagENTERGAMESERVER()
	{
		memset( this, 0, sizeof( *this));
	}
	char	szAccount[12];
	UINT	nLoginId;
	UINT	nSelCharId;
	union
	{
		UINT			nClientId;
		SERVER_ERROR	result;
	};
	DWORD	dwEnterTime;
	char	szName[32];
	DWORD	dwSelectCharServerId;
}ENTERGAMESERVER;

typedef struct tagHUMANINFO
{
	DWORD	dwDBId;
	CHAR	szName[32];

	WORD wLevel;
	BYTE btMinDef;	
	BYTE btMaxDef;			//4

	BYTE btMinMagicDef;
	BYTE btMaxMagicDef;		//6
	BYTE btMinAtk;
	BYTE btMaxAtk;			//8

	BYTE btMinMagAtk;	
	BYTE btMaxMagAtk;		//10
	BYTE btMinSprAtk;
	BYTE btMaxSprAtk;		//12

	WORD wCurHp;
	WORD wCurMp;			//16

	WORD wMaxHp;
	WORD wMaxMp;			//20

	DWORD dwCurexp;
	DWORD dwMaxexp;

	WORD wMaxBagWeight;		//36
	BYTE btMaxBodyWeight;
	BYTE btMaxHandWeight;	//40
}HUMANINFO;
struct tQueryIdent
{
	UINT lid;
	UINT sid;
};
struct tQueryPersonInfo
{
	tQueryPersonInfo()
	{
		memset( this, 0, sizeof( *this));
	}
	tQueryPersonInfo( DWORD _dwKey, const char * pszAccount, const char * pszServerName, const char * pszName )
	{
		dwKey = _dwKey;
		if( pszAccount == nullptr )
			szAccount[0] = 0;
		else
			o_strncpy( szAccount, pszAccount, 10 );
		if( pszServerName == nullptr )
			szServerName[0] = 0;
		else
			o_strncpy( szServerName, pszServerName, 18 );
		if( pszName == nullptr )
			szName[0] = 0;
		else
			o_strncpy( szName, pszName, 18 );
	}
	int	GetSize()
	{
		return sizeof( *this);
	}
	LPVOID GetPtr()
	{
		return (LPVOID)this;
	}
	DWORD	dwKey;
	char	szAccount[12];
	char	szServerName[20];
	char	szName[20];
};

struct tQueryCharList
{
	tQueryCharList()
	{
		memset( this, 0, sizeof( *this));
	}
	tQueryCharList(DWORD _dwKey, const char * pszAccount, const char * pszServerName )
	{
		dwKey = _dwKey;
		if( pszAccount == nullptr )
			szAccount[0] = 0;
		else
			o_strncpy( szAccount, pszAccount, 10 );
		if( pszServerName == nullptr )
			szServerName[0] = 0;
		else
			o_strncpy( szServerName, pszServerName, 18 );
	}
	int	GetSize()
	{
		return sizeof( *this);
	}
	LPVOID GetPtr()
	{
		return (LPVOID)this;
	}
	DWORD	dwKey;
	char	szAccount[12];
	char	szServerName[20];
};
struct tQueryCharList_Result
{
	tQueryCharList_Result()
	{
		memset( this, 0, sizeof( *this));
	}
	int	GetSize()
	{
		return (sizeof( SELECT_CHAR_LIST)*count+sizeof( DWORD ) + sizeof( int ) );
	}
	LPVOID GetPtr()
	{
		return (LPVOID)this;
	}
	DWORD	dwKey;
	int	count;
	SELECT_CHAR_LIST	charlist[2];
};
struct tQueryMapPosition_Result
{
	DWORD	dwKey;
	CHAR	szName[32];
	WORD	x,y;
};

enum	e_direction
{
	ED_UP, // 上
	ED_RU, // 右上
	ED_RT, // 右
	ED_RD, // 右下
	ED_DN, // 下
	ED_LD, // 左下
	ED_LT, // 左
	ED_LU, // 左上
	ED_MAX,
};
#define MAKEFEATHER(a,hair,b,c)  (((a&0xff) << 24) | ((hair&0xff) << 16 )|( (b&0xff) << 8 )|(c&0xff))

#define MAX_DELCHARLISTCOUNT	4
#define MAX_CHARLISTCOUNT	2

#endif

