#include "StdAfx.h"
#include ".\clientobj.h"
#include "server.h"

static void _tout(char* pret, char* p, int maxlen)
{
	int len = *p;
	if (len < 0)
	{
		pret[0] = 0;
		return;
	}
	if (len > maxlen)
		len = maxlen;
	p++;
	strncpy(pret, p, len);
	pret[len] = 0;
}

CClientObj::CClientObj(void)
{
	Clean();
}

CClientObj::~CClientObj(void)
{
}

void CClientObj::Clean()
{
	CClientObject::Clean();
}

VOID CClientObj::Update()
{
	CClientObject::Update();
}

VOID CClientObj::OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size)
{
}

VOID CClientObj::OnCodedMsg(xClientObject* pObject, PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	CAppDB& appDB = GetAppDB();
	DWORD dwKey = 0;

	pServer->OnParseMsg(pMsg->wCmd);

	switch (pMsg->wCmd)
	{
	case DM_QUERYTASKINFO:
	{
		pMsg->wParam[2] = appDB.QueryTaskInfo(*(DWORD*)pMsg->data, (TASKINFO*)pMsg->data);
		SendMsg(pMsg, sizeof(TASKINFO));
	}
	break;
	case DM_UPDATETASKINFO:
	{
		appDB.UpdateTaskInfo(pMsg->dwFlag, (TASKINFO*)pMsg->data);
	}
	break;
	case DM_QUERYFENGHAO:
	{
		pMsg->wParam[2] = appDB.QueryFengHaoInfo(*(DWORD*)pMsg->data, (FenghaoInfo*)pMsg->data);
		SendMsg(pMsg, sizeof(FenghaoInfo));
	}
	break;
	case DM_UPDATEFENGHAO:
	{
		appDB.UpdateFengHaoInfo(pMsg->dwFlag, (FenghaoInfo*)pMsg->data);
	}
	break;

	case DM_DELETEMAGIC:
	{
		appDB.DeleteMagic(pMsg->dwFlag, pMsg->wParam[0]);
	}
	break;
	case DM_ADDCREDIT:
	{
		UINT n = *(UINT*)pMsg->data;
		char* pszName = pMsg->data + sizeof(UINT);
		appDB.AddCredit(pszName, n);
	}
	break;
	case DM_UPDATECHARGUILDNAME:
	{
		char* pszName = pMsg->data;
		char* pspszGuildName = pMsg->data + 20;
		appDB.UpdateCharacterGuildName(pszName, pspszGuildName);
	}
	break;
	case DM_EXECSQL:
	{
		xPacket packet(this->m_szBuffer, 65536);
		DWORD* pdwParam = (DWORD*)pMsg->data;
		char* pszString = pMsg->data + sizeof(DWORD) * 2;
		EXECSQLRECORD* pRecord = (EXECSQLRECORD*)(pMsg->data + sizeof(DWORD) * 2 + pdwParam[1]);
		appDB.ExecSqlCommand(pdwParam[0], pszString, pRecord, packet);
		SendMsg(pMsg->dwFlag, pMsg->wCmd, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], (LPVOID)packet.getbuf(), static_cast<int>(packet.getsize()));
	}
	break;
	case DM_RESTOREGUILDNAME:
	{
		xStringsExtracter<2> ss(pMsg->data);
		if (ss.getCount() == 2)
		{
			appDB.RestoreGuild(ss[0], ss[1]);
		}
	}
	break;
	case DM_QUERYUPGRADEITEM:
	{
		int count = 1;
		if (appDB.QueryItems(*(DWORD*)pMsg->data, IDF_UPGRADE, (DBITEM*)pMsg->data, count) == SE_OK)
		{
			pMsg->wParam[2] = count;
			SendMsg(pMsg, sizeof(DBITEM));
		}
		else
		{
			pMsg->wParam[2] = 0;
			SendMsg(pMsg, 0);
		}
	}
	break;
	case DM_UPGRADEITEM:
	{
		appDB.UpgradeItem(pMsg->dwFlag, *(DWORD*)&pMsg->wParam[0]);
	}
	break;
	case DM_CACHECHARDATA:
	{
	}
	break;
	case DM_UPDATEITEMPOSEX:
	{
		appDB.UpdateItemPosEx(static_cast<BYTE>(pMsg->wParam[1]), pMsg->wParam[0], (BAGITEMPOS*)pMsg->data);
	}
	break;
	case DM_UPDATEMAGIC:
	{
		appDB.UpdateMagic(pMsg->dwFlag, (MAGICDB*)pMsg->data, pMsg->wParam[2]);
	}
	break;
	case DM_QUERYMAGIC:
	{
		int	count = 64;
		SERVER_ERROR ret = appDB.QueryMagic(*(DWORD*)pMsg->data, (MAGICDB*)pMsg->data, count);
		if (ret != SE_OK)
		{
			pMsg->wParam[2] = 0x8000 | (ret & 0xffff);
			count = 0;
		}
		else
		{
			pMsg->wParam[2] = count;
		}
		SendMsg(pMsg, count * sizeof(MAGICDB));
	}
	break;
	case DM_QUERYITEMS:
	{
		DWORD* pdwArray = (DWORD*)pMsg->data;
		int count = (int)pMsg->wParam[2];
		DBITEM* pItems = (DBITEM*)(pMsg->data + sizeof(DWORD));
		pMsg->wParam[0] = appDB.QueryItems(pdwArray[1], static_cast<BYTE>(pMsg->wParam[1]), pItems, count);
		if (pMsg->wParam[0] == SE_OK)
		{
			pMsg->wParam[2] = count;
			SendMsg(pMsg, sizeof(DWORD) + sizeof(DBITEM) * count);
		}
		else
			SendMsg(pMsg, sizeof(DWORD));
	}
	break;
	case DM_UPDATEITEMS:
	{
		DBITEM* pItem = (DBITEM*)pMsg->data;
		for (int i = 0; i < pMsg->wParam[0]; i++)
		{
			if (pItem[i].item.dwMakeIndex & 0x80000000 && pItem[i].btFlag == 0)
			{
				appDB.CreateItemEx(pMsg->dwFlag, static_cast<BYTE>(pMsg->wParam[1]), pItem[i].pos, &pItem[i].item);
			}
			else
			{
				appDB.UpdateItem(pMsg->dwFlag, static_cast<BYTE>(pMsg->wParam[1]), pItem[i].pos, &pItem[i].item);
			}
		}
	}
	break;
	//	dwFlag = ITEMID
	case DM_DELETEITEM:
	{
		appDB.DeleteItem(pMsg->dwFlag);
	}
	break;
	//	data = ITEMID
	//	wParam[0] = btFlag
	//	wParam[1] = wPos
	//	data = ownerid
	case DM_UPDATEITEMOWNER:
	{
		DWORD dwId = *(DWORD*)pMsg->data;
		appDB.UpdateItemOwner(pMsg->dwFlag, dwId, pMsg->wParam[0] & 0xff, pMsg->wParam[1]);
	}
	break;
	//	wParam[0] = btFlag
	//	wParam[1] = wPos
	case DM_UPDATEITEMPOS:
	{
		appDB.UpdateItemPos(pMsg->dwFlag, pMsg->wParam[0] & 0xff, pMsg->wParam[1]);
	}
	break;
	//	dwFlag = ownerid
	//	wParam[0] = btFlag
	//	wParam[1] = wPos
	case DM_UPDATEITEM:
	{
		ITEM* pItem = (ITEM*)pMsg->data;
		appDB.UpdateItem(pMsg->dwFlag, pMsg->wParam[0] & 0xff, pMsg->wParam[1], pItem);
	}
	break;
	//	data = CREATEITEM STRUCT
	case DM_CREATEITEM:
	{
		CREATEITEM* pInfo = (CREATEITEM*)pMsg->data;
		pMsg->wParam[0] = appDB.CreateItem(pInfo->dwOwner, pInfo->btFlag, pInfo->wPos, &pInfo->item);
		SendMsg(pMsg, datasize);
	}
	break;
	//	data = CHARDBINFO
	case DM_PUTCHARDBINFO:
	{
		appDB.PutCharDBInfo((CHARDBINFO*)pMsg->data);
	}
	break;
	case DM_GETCHARDBINFO:
	{
		tQueryPersonInfo* pInfo = (tQueryPersonInfo*)pMsg->data;
		pMsg->wParam[0] = appDB.GetCharDBInfo(pInfo->szAccount, pInfo->szServerName, pInfo->szName, (CHARDBINFO*)pMsg->data);
		SendMsg(pMsg, sizeof(CHARDBINFO));
	}
	break;
	case DM_GETCHARPOSITIONFORSELCHAR:
	{
		tQueryPersonInfo* pinfo = (tQueryPersonInfo*)pMsg->data;

		pMsg->wParam[0] = appDB.GetMapPosition(pinfo->szAccount, pinfo->szServerName, pinfo->szName, (tQueryMapPosition_Result*)pMsg->data);
		SendMsg(pMsg, sizeof(tQueryMapPosition_Result));
	}
	break;
	case DM_RESTORECHARACTER:
	{
		tQueryPersonInfo* pinfo = (tQueryPersonInfo*)pMsg->data;
		pMsg->wParam[0] = appDB.RestoreCharacter(pinfo->szAccount, pinfo->szServerName, pinfo->szName);
		SendMsg(pMsg, 4);
	}
	break;
	case DM_DELETEDCHARLIST:
	{
		tQueryCharList* plist = (tQueryCharList*)pMsg->data;
		tQueryCharList_Result* pResult = (tQueryCharList_Result*)pMsg->data;

		pMsg->wParam[0] = appDB.GetDelCharList(plist->szAccount, plist->szServerName, pResult);
		SendMsg(pMsg, pResult->GetSize());
	}
	break;
	case DM_DELETECHARACTER:
	{
		tQueryPersonInfo* pinfo = (tQueryPersonInfo*)pMsg->data;
		pMsg->wParam[0] = appDB.DelCharacter(pinfo->szAccount, pinfo->szServerName, pinfo->szName);
		SendMsg(pMsg, 4);
	}
	break;
	case DM_CREATECHARACTER:
	{
		pMsg->wParam[0] = appDB.CreateCharacter((CREATECHARDESC*)pMsg->data);
		SendMsg(pMsg, 4);
	}
	break;
	case DM_QUERYCHARLIST:
	{
		tQueryCharList* plist = (tQueryCharList*)pMsg->data;
		tQueryCharList_Result* pResult = (tQueryCharList_Result*)pMsg->data;
		pMsg->wParam[0] = appDB.GetCharList(plist->szAccount, plist->szServerName, pResult);
		SendMsg(pMsg, pResult->GetSize());
	}
	break;
	case DM_CHANGEPASSWORD:
	{
		char* Params[4];
		int nParam = SearchParam(pMsg->data, Params, 4, '\t');
		if (nParam != 3)
		{
			pMsg->wParam[0] = SE_LOGIN_ACCOUNTNOTEXIST;
		}
		else
			pMsg->wParam[0] = static_cast<WORD>(appDB.ChangePassword(Params[0], Params[1], Params[2]));
		SendMsg(pMsg, 0);
	}
	break;
	case DM_CHECKACCOUNT:
	{
		char* p = strchr(pMsg->data, '/');
		pMsg->wParam[0] = SE_FAIL;
		if (p != nullptr)
		{
			*p = 0;
			pMsg->wParam[0] = static_cast<WORD>(appDB.CheckAccount(pMsg->data, p + 1));
		}
		SendMsg(pMsg, static_cast<int>(strlen(pMsg->data)));
	}
	break;
	case DM_CHECKACCOUNTEXIST:
	{
		pMsg->wParam[0] = static_cast<WORD>(appDB.CheckAccountExist(pMsg->data));
		SendMsg(pMsg, 0);
	}
	break;
	case DM_QUERYCOMMUNITY:
	{
		char szText[4096];
		int iSize = 4096;
		DWORD dwOwner = *(DWORD*)pMsg->data;
		if (appDB.QueryCommunity(dwOwner, szText, iSize) == SE_OK)
		{
			SendMsg(pMsg->dwFlag, DM_QUERYCOMMUNITY, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2],
				(LPVOID)szText, iSize);
		}
		else
		{
			SendMsg(pMsg->dwFlag, DM_QUERYCOMMUNITY, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2],
				(LPVOID)"", 1);
		}
	}
	break;
	case DM_UPDATECOMMUNITY:
	{
		if (appDB.UpdateCommunity(pMsg->dwFlag, pMsg->data) != SE_OK)
			CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "更新社交信息出错～!\n");
	}
	break;
	//	data = c1/c2
	case DM_BREAKMARRIAGE:
	{
		xStringsExpander<20> param(pMsg->data, '/');
		if (param.getCount() == 2)
		{
			if (appDB.DeleteMarriage(param[0], param[1]) != SE_OK)
				CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "解除 %s 和 %s 的婚姻关系出错～!\n", param[0], param[1]);
			if (appDB.DeleteMarriage(param[1], param[0]) != SE_OK)
				CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "解除 %s 和 %s 的婚姻关系出错～!\n", param[1], param[0]);
		}
	}
	break;
	//	data = c1/c2
	case DM_BREAKMASTER:
	{
		xStringsExpander<20> param(pMsg->data, '/');
		if (param.getCount() == 2)
		{
			if (appDB.DeleteStudent(param[0], param[1]) != SE_OK)
				CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "解除 %s(师父) 和 %s(徒弟) 的师徒关系出错～!\n", param[0], param[1]);
			if (appDB.DeleteTeacher(param[1], param[0]) != SE_OK)
				CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "解除 %s(徒弟) 和 %s(师父) 的好友关系出错～!\n", param[1], param[0]);
		}
	}
	break;
	case DM_BREAKFRIEND:
	{
		xStringsExpander<20> param(pMsg->data, '/');
		if (param.getCount() == 2)
		{
			if (appDB.BreakFriend(param[0], param[1]) != SE_OK)
				CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "解除 %s 和 %s 的好友关系出错～!\n", param[0], param[1]);
			if (appDB.BreakFriend(param[1], param[0]) != SE_OK)
				CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "解除 %s 和 %s 的好友关系出错～!\n", param[1], param[0]);
		}
	}
	break;
	case DM_CHECKFREE:
	{
		SendMsg(pMsg->dwFlag, DM_CHECKFREE, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2]);
	}
	break;
	case DM_CREATEACCOUNT:
	{
		if (datasize < 226)
		{
			pMsg->wParam[0] = SE_REG_INVALIDACCOUNT;
			SendMsg(pMsg, 0);
			break;
		}
		CHAR szAccount[256];
		CHAR szPassword[256];
		CHAR szEMail[256];
		CHAR szQ1[256];
		CHAR szA1[256];
		CHAR szQ2[256];
		CHAR szA2[256];
		CHAR szMobilePhoneNumber[256];
		CHAR szPhoneNumber[256];
		CHAR szName[256];
		CHAR szBirthday[256];
		CHAR szIdCard[256];
		_tout(szAccount, pMsg->data + RAI_ACCOUNT, 10);// *(pMsg->data+RAI_ACCOUNT));
		_tout(szPassword, pMsg->data + RAI_PASSWORD, 10);// *(pMsg->data+RAI_PASSWORD));
		_tout(szName, pMsg->data + RAI_NAME, 20);// *(pMsg->data+RAI_NAME));
		_tout(szIdCard, pMsg->data + RAI_IDCARD, 20);// *(pMsg->data+RAI_IDCARD));
		_tout(szPhoneNumber, pMsg->data + RAI_PHONENUMBER, 12);// *(pMsg->data+RAI_PHONENUMBER));
		_tout(szQ1, pMsg->data + RAI_Q1, 20);// *(pMsg->data+RAI_Q1));
		_tout(szA1, pMsg->data + RAI_A1, 20);// *(pMsg->data+RAI_A1));
		_tout(szEMail, pMsg->data + RAI_MAIL, 40);// *(pMsg->data+RAI_MAIL));
		_tout(szQ2, pMsg->data + RAI_Q2, 20);// *(pMsg->data+RAI_Q2));
		_tout(szA2, pMsg->data + RAI_A2, 20);// *(pMsg->data+RAI_A2));
		_tout(szBirthday, pMsg->data + RAI_BIRTHDAY, 10);// *(pMsg->data+RAI_BIRTHDAY));
		_tout(szMobilePhoneNumber, pMsg->data + RAI_MOBILEPHONENUMBER, 11);// *(pMsg->data+RAI_MOBILEPHONENUMBER));
		pMsg->wParam[0] = static_cast<WORD>(appDB.CreateAccount(szAccount, szPassword, szName, szBirthday,
			szQ1, szA1, szQ2, szA2, szEMail,
			szPhoneNumber, szMobilePhoneNumber, szIdCard));
		SendMsg(pMsg, 0);
	}
	break;
	}
}
