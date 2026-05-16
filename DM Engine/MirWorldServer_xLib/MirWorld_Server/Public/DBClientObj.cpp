#include "StdAfx.h"
#include ".\dbclientobj.h"
#include <memory>

CDBClientObj::CDBClientObj(void)
{
	Clean();
}

CDBClientObj::~CDBClientObj(void)
{
}

void CDBClientObj::Update()
{
	CClientObject::Update();
}

VOID CDBClientObj::SendQueryCharlist(UINT id, UINT key, const char* pszAccount, const char* pszServerName)
{
	if (this == nullptr)return;
	tQueryCharList qlist(key, pszAccount, pszServerName);
	SendMsg(id, DM_QUERYCHARLIST, 0, 0, 0, qlist.GetPtr(), qlist.GetSize());
}

VOID CDBClientObj::SendQueryCreateChar(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName, BYTE btClass, BYTE btSex, BYTE btHair)
{
	if (this == nullptr)return;
	CREATECHARDESC desc;
	desc.dwKey = key;
	desc.btClass = btClass;
	desc.btHair = btHair;
	desc.btLevel = 0;
	desc.btSex = btSex;
	strncpy(desc.szServer, pszServerName, 32);
	strncpy(desc.szName, pszName, 32);
	strncpy(desc.szAccount, pszAccount, 10);
	SendMsg(id, DM_CREATECHARACTER, 0, 0, 0, &desc, sizeof(desc));
}

VOID CDBClientObj::SendQueryDelChar(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName)
{
	if (this == nullptr)return;
	tQueryPersonInfo info(key, pszAccount, pszServerName, pszName);
	SendMsg(id, DM_DELETECHARACTER, 0, 0, 0, info.GetPtr(), info.GetSize());
}

VOID CDBClientObj::SendQueryDeletedCharList(UINT id, UINT key, const char* pszAccount, const char* pszServerName)
{
	if (this == nullptr)return;
	tQueryCharList clist(key, pszAccount, pszServerName);
	SendMsg(id, DM_DELETEDCHARLIST, 0, 0, 0, clist.GetPtr(), clist.GetSize());
}

VOID CDBClientObj::SendQueryRestoreChar(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName)
{
	if (this == nullptr)return;
	tQueryPersonInfo info(key, pszAccount, pszServerName, pszName);
	SendMsg(id, DM_RESTORECHARACTER, 0, 0, 0, info.GetPtr(), info.GetSize());
}

VOID CDBClientObj::SendQueryMapPosition(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName)
{
	if (this == nullptr)return;
	tQueryPersonInfo info(key, pszAccount, pszServerName, pszName);
	SendMsg(id, DM_GETCHARPOSITIONFORSELCHAR, 0, 0, 0, info.GetPtr(), info.GetSize());
}

VOID CDBClientObj::SendQueryDbInfo(UINT id, UINT key, const char* pszAccount, const char* pszServerName, const char* pszName)
{
	if (this == nullptr)return;
	tQueryPersonInfo info(key, pszAccount, pszServerName, pszName);
	SendMsg(id, DM_GETCHARDBINFO, 0, 0, 0, info.GetPtr(), info.GetSize());
}

VOID CDBClientObj::SendPutDbInfo(UINT id, UINT key, CHARDBINFO& info)
{
	info.dwClientKey = key;
	SendMsg(id, DM_PUTCHARDBINFO, 0, 0, 0, &info, sizeof(info));
}

VOID CDBClientObj::SendDeleteItem(DWORD dwMakeIndex, BYTE btDelReason)
{
	SendMsg(dwMakeIndex, DM_DELETEITEM, (WORD)btDelReason, 0, 0, nullptr);
}

VOID CDBClientObj::SendUpdateItemOwner(DWORD dwMakeIndex, DWORD dwOwner, BYTE btFlag, WORD wPos)
{
	SendMsg(dwMakeIndex, DM_UPDATEITEMOWNER, (WORD)btFlag, wPos, 0, &dwOwner, 4);
}

VOID CDBClientObj::SendUpdateItemPos(DWORD dwMakeIndex, BYTE btFlag, WORD wPos)
{
	SendMsg(dwMakeIndex, DM_UPDATEITEMPOS, (WORD)btFlag, wPos, 0, nullptr);
}

VOID CDBClientObj::SendUpdateItem(DWORD dwOwner, BYTE btFlag, WORD wPos, ITEM* pItem)
{
	SendMsg(dwOwner, DM_UPDATEITEM, (WORD)btFlag, wPos, 0, (LPVOID)pItem, sizeof(ITEM));
}

VOID CDBClientObj::SendCreateItem(DWORD id, DWORD dwKey, DWORD dwOwner, BYTE btFlag, WORD wPos, ITEM* pItem)
{
	CREATEITEM ci;
	ci.dwClientKey = dwKey;
	ci.dwOwner = dwOwner;
	ci.btFlag = btFlag;
	ci.wPos = wPos;
	ci.item = *pItem;
	SendMsg(id, DM_CREATEITEM, 0, 0, 0, &ci, sizeof(ci));
}

VOID CDBClientObj::SendQueryItem(UINT id, UINT key, DWORD dwDBId, BYTE btFlag, WORD wMaxCount)
{
	DWORD dwArray[] = { key, dwDBId };
	SendMsg(id, DM_QUERYITEMS, 0, btFlag, wMaxCount, (LPVOID)dwArray, sizeof(dwArray));
}

VOID CDBClientObj::SendUpdateMagic(DWORD dwDBId, int count, MAGICDB* pArray)
{
	SendMsg(dwDBId, DM_UPDATEMAGIC, 0, 0, count, (LPVOID)pArray, sizeof(MAGICDB) * count);
}

VOID CDBClientObj::SendQueryMagic(UINT id, UINT key, DWORD dwDBId)
{
	SendMsg(id, DM_QUERYMAGIC, key & 0xffff, ((key & 0xffff0000) >> 16), 0, &dwDBId, sizeof(DWORD));
}

VOID CDBClientObj::UpdateAccountState(DWORD dwServerId, const char* pszAccount, UINT state)
{
	SendMsg(dwServerId, DM_UPDATEACCOUNTSTATE, state, 0, 0, (LPVOID)pszAccount);
}

VOID CDBClientObj::SendUpdateItemPos(BYTE btFlag, BAGITEMPOS* pItemPos, WORD wCount)
{
	SendMsg(0, DM_UPDATEITEMPOSEX, wCount, btFlag, 0, (LPVOID)pItemPos, wCount * sizeof(BAGITEMPOS));
}

VOID CDBClientObj::UpdateCommunity(DWORD dwOwner, const char* pszCommunity)
{
	SendMsg(dwOwner, DM_UPDATECOMMUNITY, 0, 0, 0, (LPVOID)pszCommunity);
}

VOID CDBClientObj::QueryCommunity(UINT id, DWORD dwKey, DWORD dwOwner)
{
	SendMsg(id, DM_QUERYCOMMUNITY, dwKey & 0xffff, (dwKey & 0xffff0000) >> 16, 0, &dwOwner, sizeof(DWORD));
}

VOID CDBClientObj::SendBreakFriend(const char* friend1, const char* friend2)
{
	char szText[1024];
	sprintf(szText, "%s/%s", friend1, friend2);
	SendMsg(0, DM_BREAKFRIEND, 0, 0, 0, (LPVOID)szText);
}
VOID CDBClientObj::SendBreakTeacher(const char* pszTeacher, const char* pszStudent)
{
	char szText[1024];
	sprintf(szText, "%s/%s", pszTeacher, pszStudent);
	SendMsg(0, DM_BREAKMASTER, 0, 0, 0, (LPVOID)szText);
}

VOID CDBClientObj::SendBreakMarriage(const char* pszMarriage1, const char* pszMarriage2)
{
	char szText[1024];
	sprintf(szText, "%s/%s", pszMarriage1, pszMarriage2);
	SendMsg(0, DM_BREAKMARRIAGE, 0, 0, 0, (LPVOID)szText);
}

VOID CDBClientObj::SendUpdateItems(DWORD dwOwner, BYTE btFlag, DBITEM* pItemArray, int count)
{
	SendMsg(dwOwner, DM_UPDATEITEMS, count, btFlag, 0, (LPVOID)pItemArray, sizeof(DBITEM) * count);
}

VOID CDBClientObj::SendUpgradeItem(DWORD dwMakeIndex, DWORD dwUpgrade)
{
	SendMsg(dwMakeIndex, DM_UPGRADEITEM, dwUpgrade & 0xffff, (dwUpgrade & 0xffff0000) >> 16, 0);
}

VOID CDBClientObj::SendQueryUpgradeItem(DWORD id, DWORD dwKey, DWORD dwDBId)
{
	SendMsg(id, DM_QUERYUPGRADEITEM, dwKey & 0xffff, (dwKey & 0xffff0000) >> 16, 0, &dwDBId, sizeof(DWORD));
}

VOID CDBClientObj::SendRestoreGuild(const char* name, const char* guildname)
{
	char szTemp[200];
	sprintf(szTemp, "%s/%s", name, guildname);
	SendMsg(0, DM_RESTOREGUILDNAME, 0, 0, 0, (LPVOID)szTemp);
}

VOID CDBClientObj::SendExecSqlCommand(UINT nId, UINT nKey, UINT nTransid, const char* pszCommand, UINT nRecordDefCount, COLTYPE* pDefTypes, UINT nRecordCount)
{
	const int BUFFER_SIZE = 65536;
	std::unique_ptr<char[]> szBuffer(new char[BUFFER_SIZE]);
	xPacket packet(szBuffer.get(), BUFFER_SIZE);
	DWORD dwTemp = static_cast<DWORD>(strlen(pszCommand)) + 1;
	packet.push(&nTransid, sizeof(DWORD));
	packet.push(&dwTemp, sizeof(DWORD));
	packet.push((LPVOID)pszCommand, dwTemp);
	packet.push(&nRecordCount, sizeof(DWORD));
	if (nRecordDefCount == 0)pDefTypes = nullptr;
	if (pDefTypes == nullptr)nRecordDefCount = 0;
	packet.push(&nRecordDefCount, sizeof(DWORD));
	if (pDefTypes)
		packet.push((LPVOID)pDefTypes, sizeof(COLTYPE) * nRecordDefCount);
	SendMsg(nId, DM_EXECSQL, nKey & 0xffff, (nKey & 0xffff0000) >> 16, 0, (LPVOID)packet.getbuf(), static_cast<int>(packet.getsize()));
}

VOID CDBClientObj::SendAddCredit(const char* pszName, UINT nCount)
{
	char szTemp[1024] = { 0 };
	*(UINT*)szTemp = nCount;
	o_strncpy(szTemp + sizeof(UINT), pszName, 16);
	SendMsg(0, DM_ADDCREDIT, 0, 0, 0, (LPVOID)szTemp, sizeof(UINT) + 20);
}

VOID CDBClientObj::SendDeleteMagic(DWORD dwOwner, WORD wMagicId)
{
	SendMsg(dwOwner, DM_DELETEMAGIC, wMagicId, 0, 0, nullptr, 0);
}

VOID CDBClientObj::UpdateTaskInfo(DWORD dwOwner, TASKINFO* pInfo)
{
	SendMsg(dwOwner, DM_UPDATETASKINFO, 0, 0, 0, (LPVOID)pInfo, sizeof(TASKINFO));
}

VOID CDBClientObj::QueryTaskInfo(UINT id, DWORD dwKey, DWORD dwOwner)
{
	SendMsg(id, DM_QUERYTASKINFO, dwKey & 0xffff, (dwKey & 0xffff0000) >> 16, 0, &dwOwner, sizeof(DWORD));
}

VOID CDBClientObj::UpdateFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo)
{
	SendMsg(dwOwner, DM_UPDATEFENGHAO, 0, 0, 0, (LPVOID)pInfo, sizeof(FenghaoInfo));
}

VOID CDBClientObj::QueryFengHaoInfo(UINT id, DWORD dwKey, DWORD dwOwner)
{
	SendMsg(id, DM_QUERYFENGHAO, dwKey & 0xffff, (dwKey & 0xffff0000) >> 16, 0, &dwOwner, sizeof(DWORD));
}
