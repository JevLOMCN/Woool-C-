#pragma once
#include "inc.h"

typedef struct tagSERVERSTATE
{
	tagSERVERSTATE()
	{
		FILLSELF(0);
	}
	int	numConnection;
	int	numConnected;
	int	numDisconnected;
	DWORD	dwRecvBytes;
	DWORD	dwSendBytes;
	DWORD	dwRunningSeconds;
	DWORD	dwLoopTimes;
	float	fLoopTime;
	BOOL	bUpdated;

	char* pServerDescript;
	char* pServerState;
}SERVERSTATE;

#define GETERRORMESSAGE(id)	(id)
#define ERRMSG_NOCONFIGFILE	"无法打开Config.ini."
#define ERRMSG_NOSERVER		"服务器指针没有初始化!"
#define MAX_ACCEPTQUEUE		32

class CVirtualServer
{
public:
	virtual VOID	GetServerState(SERVERSTATE& state) = 0;
	virtual BOOL	Start(CSettingFile&) = 0;
	virtual BOOL	Stop() = 0;
};