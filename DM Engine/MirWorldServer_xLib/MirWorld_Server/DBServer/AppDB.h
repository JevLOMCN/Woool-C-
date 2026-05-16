#pragma once
#include "virtualdatabase.h"

class CAppDB
{
public:
	CAppDB(void);
	virtual ~CAppDB(void);
	SERVER_ERROR OpenDataBase(const char* pszServer, const char* pszPort, const char* pszDBName, const char* pszId, const char* pszPassword);
	SERVER_ERROR CreateAccount(const char* pszAccount, const char* pszPassword, const char* pszName, const char* pszBirthday,
		const char* pszQ1, const char* pszA1, const char* pszQ2, const char* pszA2, const char* pszEmail,
		const char* pszPhoneNumber, const char* pszMobilePhoneNumber, const char* pszIdCard);
	SERVER_ERROR CheckAccount(const char* pszAccount, const char* pszPassword);
	SERVER_ERROR ChangePassword(const char* pszAccount, const char* pszOldPass, const char* pszNewPass);
	SERVER_ERROR CheckAccountExist(const char* pszAccount);

	SERVER_ERROR UpdateAccountState(DWORD dwServerId, const char* pszAccount, UINT state);

	SERVER_ERROR GetCharList(const char* pszAccount, const char* pszServerName, tQueryCharList_Result* pResult);
	SERVER_ERROR GetDelCharList(const char* pszAccount, const char* pszServerName, tQueryCharList_Result* pResult);

	SERVER_ERROR CreateCharacter(CREATECHARDESC* pDesc);
	SERVER_ERROR CheckCharacterExist(const char* pszAccount, const char* pszServerName, const char* pszName);
	SERVER_ERROR DelCharacter(const char* pszAccount, const char* pszServerName, const char* pszName);
	SERVER_ERROR RestoreCharacter(const char* pszAccount, const char* pszServerName, const char* pszName);

	SERVER_ERROR GetMapPosition(const char* pszAccount, const char* pszServerName, const char* pszName, tQueryMapPosition_Result* pResult);
	SERVER_ERROR GetCharDBInfo(const char* pszAccount, const char* pszServerName, const char* pszName, CHARDBINFO* pInfo);
	SERVER_ERROR PutCharDBInfo(CHARDBINFO* pInfo);

	SERVER_ERROR GetFreeItemId(DWORD& dwItemIndex);
	SERVER_ERROR FindItemId(DWORD dwOwner, BYTE btFlag, WORD wPos, DWORD dwFindKey, DWORD& dwItemIndex);

	SERVER_ERROR CreateItem(DWORD dwOwner, BYTE	btFlag, WORD wPos, ITEM* pItem);
	SERVER_ERROR CreateItemEx(DWORD dwOwner, BYTE btFlag, WORD wPos, ITEM* pItem);
	SERVER_ERROR UpdateItem(DWORD dwOwner, BYTE	btFlag, WORD wPos, ITEM* pItem);
	SERVER_ERROR DeleteItem(DWORD dwItemIndex);
	SERVER_ERROR UpdateItemPos(DWORD dwItemIndex, BYTE btFlag, WORD wPos);
	SERVER_ERROR UpdateItemPosEx(BYTE btFlag, WORD wCount, BAGITEMPOS* pItemPos);

	SERVER_ERROR UpdateItemOwner(DWORD dwItemIndex, DWORD dwOwner, BYTE btFlag, WORD wPos);

	SERVER_ERROR QueryItems(DWORD dwOwner, BYTE	btFlag, DBITEM* pItemBuffer, int& count);

	SERVER_ERROR UpgradeItem(DWORD dwMakeIndex, DWORD dwUpgrade);

	SERVER_ERROR UpdateMagic(DWORD dwOwner, MAGICDB* pMagic, int count);
	SERVER_ERROR QueryMagic(DWORD dwOwner, MAGICDB* pMagicArray, int& count);

	SERVER_ERROR UpdateCommunity(DWORD dwOwner, const char* pszCommunity);
	SERVER_ERROR QueryCommunity(DWORD dwOwner, char* pszCommunity, int& iBufferSize);
	SERVER_ERROR BreakFriend(const char* friend1, const char* friend2);
	SERVER_ERROR RestoreGuild(const char* name, const char* guildname);

	SERVER_ERROR DeleteMarriage(const char* pszName, const char* pszMarriage);
	SERVER_ERROR DeleteTeacher(const char* pszName, const char* pszTeacher);
	SERVER_ERROR DeleteStudent(const char* pszTeacher, const char* pszStudent);

	SERVER_ERROR ExecSqlCommand(DWORD dwTransId, const char* pszCommand, EXECSQLRECORD* pRecordDef, xPacket& packet);
	SERVER_ERROR AddCredit(const char* pszName, UINT nCount);
	SERVER_ERROR UpdateCharacterGuildName(const char* pszName, const char* pszGuildName);
	SERVER_ERROR DeleteMagic(DWORD dwOwner, WORD wId);
	//任务系统数据
	SERVER_ERROR QueryTaskInfo(DWORD dwOwner, TASKINFO* pInfo);
	SERVER_ERROR UpdateTaskInfo(DWORD dwOwner, TASKINFO* pInfo);
	SERVER_ERROR InstertTaskInfo(DWORD dwOwner, TASKINFO* pInfo);
	//时长封号系统数据
	SERVER_ERROR QueryFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo);
	SERVER_ERROR UpdateFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo);
	SERVER_ERROR InstertFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo);

	VOID DoInit();
private:
	CVirtualDatabase* m_pDatabase;
	CVirtualDBConnection* m_pDBConnection;
};