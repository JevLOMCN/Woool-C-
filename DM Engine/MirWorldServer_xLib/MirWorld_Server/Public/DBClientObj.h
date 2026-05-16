#pragma once
#include "clientobject.h"

class CDBClientObj :
	public CClientObject
{
public:
	CDBClientObj(void);
	virtual ~CDBClientObj(void);
	void Update();
	VOID SendQueryCharlist(UINT id, UINT key, const char* pszAccount, const char* pszServerName);
	VOID SendQueryCreateChar(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName, BYTE btClass, BYTE btSex, BYTE btHair);
	VOID SendQueryDelChar(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName);
	VOID SendQueryDeletedCharList(UINT id, UINT key, const char* pszAccount, const char* pszServerName);
	VOID SendQueryRestoreChar(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName);
	VOID SendQueryMapPosition(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName);
	VOID SendQueryDbInfo(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName);
	VOID SendDeleteItem(DWORD dwMakeIndex, BYTE btDelReason);
	VOID SendUpdateItemOwner(DWORD dwMakeIndex, DWORD dwOwner, BYTE btFlag, WORD wPos);
	VOID SendUpdateItemPos(DWORD dwMakeIndex, BYTE btFlag, WORD wPos);
	VOID SendUpdateItemPos(BYTE btFlag, BAGITEMPOS* pItemPos, WORD wCount);
	VOID SendUpdateItem(DWORD dwOwner, BYTE btFlag, WORD wPos, ITEM* pItem);
	VOID SendCreateItem(DWORD id, DWORD dwKey, DWORD dwOwner, BYTE btFlag, WORD wPos, ITEM* pItem);
	VOID SendPutDbInfo(UINT id, UINT key, CHARDBINFO& info);
	VOID SendQueryItem(UINT id, UINT key, DWORD dwDBId, BYTE btFlag, WORD wMaxCount);
	VOID SendUpdateMagic(DWORD dwDBId, int count, MAGICDB* pArray);
	VOID SendQueryMagic(UINT id, UINT key, DWORD dwDBId);
	VOID UpdateAccountState(DWORD dwServerId, const char* pszAccount, UINT state);
	VOID UpdateCommunity(DWORD dwOwner, const char* pszCommunity);
	VOID QueryCommunity(UINT id, DWORD dwKey, DWORD dwOwner);
	VOID SendBreakFriend(const char* friend1, const char* friend2);
	VOID SendBreakTeacher(const char* pszTeacher, const char* pszStudent);
	VOID SendBreakMarriage(const char* pszMarriage1, const char* pszMarriage2);
	VOID SendUpdateItems(DWORD dwOwner, BYTE btFlag, DBITEM* pItemArray, int count);
	VOID SendUpgradeItem(DWORD dwMakeIndex, DWORD dwUpgrade);
	VOID SendQueryUpgradeItem(DWORD id, DWORD dwKey, DWORD dwDBId);
	VOID SendRestoreGuild(const char* name, const char* guildname);
	VOID SendExecSqlCommand(UINT nId, UINT nKey, UINT nTransid, const char* pszCommand, UINT nRecordDefCount, COLTYPE* pDefTypes = nullptr, UINT nRecordCount = 1);
	VOID SendAddCredit(const char* pszName, UINT nCount);
	VOID SendDeleteMagic(DWORD dwOwner, WORD wMagicId);
	xPacket* GetPacket() { return m_xPacketQueue.pop(); }
	//任务系统数据
	VOID UpdateTaskInfo(DWORD dwOwner, TASKINFO* pInfo);
	VOID QueryTaskInfo(UINT id, DWORD dwKey, DWORD dwOwner);
	//int	GetSendBufferSize(){ return (2048 * 2048);}
	//int	GetRecvBufferSize(){ return (2048 * 2048);}
	//时长封号系统数据
	VOID UpdateFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo);
	VOID QueryFengHaoInfo(UINT id, DWORD dwKey, DWORD dwOwner);
};
