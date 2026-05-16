#pragma once
#include "clientobj.h"

constexpr UINT MAX_CLIENTOBJECT = 8;

typedef struct tagSERVERARRAY
{
	UINT Ids[64];
	int count;
	int	pickptr;
	tagSERVERARRAY()
	{
		count = 0;
		pickptr = 0;
	}
	void DelId(UINT id)
	{
		int i = 0;
		for (i = 0; i < count; i++)
		{
			if (Ids[i] == id)
			{
				for (int j = i; j < count - 1; j++)
				{
					Ids[j] = Ids[j + 1];
				}
				count--;
				break;
			}
		}
	}
	BOOL AddId(UINT id)
	{
		if (count >= 64)return FALSE;
		Ids[count++] = id;
		return TRUE;
	}
}SERVERARRAY;

class CServer :
	public CBaseServer,
	public xIndexObjectPool<CClientObj>,
	public xSingletonClass<CServer>
{
public:
	CServer(void);
	virtual ~CServer(void);
	void OnInput(const char* pString) { CBaseServer::OnInput(pString); }
	CClientObject* GetClientObject(UINT id) { return getObject(id); }
	CClientObject* NewClientObject();
	VOID DeleteClientObject(CClientObject* pObject);
	BOOL InitServer(CSettingFile&);
	VOID CleanServer();
	VOID Update();

	SERVER_ERROR RegisterServer(CClientObj* pObj, REGISTER_SERVER_INFO* pInfo, REGISTER_SERVER_RESULT* pResult);
	SERVER_ERROR UnRegisterServer(CClientObj* pObj);
	SERVER_ERROR FindServer(servertype type, const char* pszName, FINDSERVER_RESULT* pResult);
public:
	VOID SendDBServer(SERVERADDR* pAddr, int count);
	int PrepareServer(int type, int count, SERVERADDR* pAddrArray);
protected:
	SERVERARRAY	m_ServerArrays[ST_GAMESERVER];
};