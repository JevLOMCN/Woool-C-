#include "StdAfx.h"
#include ".\appdb.h"
#include "mysqldatabase.h"
#include "server.h"
#include <string>

static thread_local char g_szTempBuffer[65536];
CAppDB::CAppDB(void)
{
	m_pDatabase = nullptr;
	m_pDBConnection = nullptr;
}

CAppDB::~CAppDB(void)
{
	if (m_pDBConnection != nullptr)
		m_pDatabase->DelConnection(m_pDBConnection);
	if (m_pDatabase != nullptr)
	{
		m_pDatabase->UnInit();
		delete m_pDatabase;
	}
}

SERVER_ERROR CAppDB::OpenDataBase(const char* pszServer, const char* pszPort, const char* pszDBName, const char* pszId, const char* pszPassword)
{
	SERVER_ERROR ret = SE_OK;
	m_pDatabase = new CMySQLDatabase;
	if (m_pDatabase == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = m_pDatabase->Init();
	if (ret != SE_OK) return ret;
	m_pDBConnection = m_pDatabase->GetConnection();
	if (m_pDBConnection == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = m_pDBConnection->Connect(pszServer, pszPort, pszDBName, pszId, pszPassword);
	if (ret != SE_OK) return ret;
	return SE_OK;
}

SERVER_ERROR CAppDB::CreateAccount(const char* pszAccount, const char* pszPassword, const char* pszName, const char* pszBirthday,
	const char* pszQ1, const char* pszA1, const char* pszQ2, const char* pszA2, const char* pszEmail,
	const char* pszPhoneNumber, const char* pszMobilePhoneNumber, const char* pszIdCard)
{
	CVirtualDataUnit* pDataUnit = nullptr;
	SERVER_ERROR ret = SE_OK;

	if (!xCheckAccount(pszAccount))
		return SE_REG_INVALIDACCOUNT;
	if (!xCheckPassword(pszPassword))
		return SE_REG_INVALIDPASSWORD;
	if (!xCheckNormalString(pszName))
		return SE_REG_INVALIDNAME;
	if (pszBirthday[0] != 0 && !xCheckBirthday(pszBirthday))
		return SE_REG_INVALIDBIRTHDAY;
	if (!xCheckIdCard(pszIdCard))
		return SE_REG_INVALIDIDCARD;
	if (!xCheckNormalString(pszQ1))
		return SE_REG_INVALIDQUESTION;
	if (!xCheckNormalString(pszA1))
		return SE_REG_INVALIDANSWER;
	if (!xCheckNormalString(pszQ2))
		return SE_REG_INVALIDQUESTION;
	if (!xCheckNormalString(pszA2))
		return SE_REG_INVALIDANSWER;
	if (!xCheckTelephone(pszPhoneNumber))
		return SE_REG_INVALIDPHONENUMBER;
	if (!xCheckMobilePhone(pszMobilePhoneNumber))
		return SE_REG_INVALIDMOBILEPHONE;

	if (pszEmail[0] != 0 && !xCheckEmail(pszEmail)) return SE_REG_INVALIDEMAIL;
	if ((ret = CheckAccountExist(pszAccount)) == SE_LOGIN_ACCOUNTNOTEXIST)
	{
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("Insert into TBL_ACCOUNT(ACCOUNT,PASSWORD,NAME,BIRTHDAY,Q1,A1,Q2,A2,PHONENUMBER,MOBILEPHONENUMBER,IDCARD,EMAIL,CREATEDATE)"
			"VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',CURRENT_TIMESTAMP)",
			pszAccount, pszPassword, pszName, pszBirthday,
			pszQ1, pszA1, pszQ2, pszA2, pszPhoneNumber, pszMobilePhoneNumber, pszIdCard, pszEmail);
		m_pDBConnection->DelDataUnit(pDataUnit);   //注册账号
	}
	return ret;
}

SERVER_ERROR CAppDB::CheckAccount(const char* pszAccount, const char* pszPassword)
{
	CHAR szPassword[65] = ""; // 增加缓冲区大小
	int bufferSize = 64;        // 缓冲区大小
	int dataSize = 0;          // 接收实际数据大小

	if (!xCheckAccount(pszAccount)) return SE_LOGIN_ACCOUNTNOTEXIST;
	if (!xCheckPassword(pszPassword)) return SE_LOGIN_PASSWORDERROR;

	//    ?  
	if (strlen(pszAccount) > 50) return SE_LOGIN_ACCOUNTNOTEXIST;

	SERVER_ERROR ret = CheckAccountExist(pszAccount);
	if (ret != SE_LOGIN_ACCOUNTEXIST)return ret;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	ret = pDataUnit->Operation("select PASSWORD from TBL_ACCOUNT where ACCOUNT = N'%s'", pszAccount);
	if (ret == SE_OK)
	{
		dataSize = bufferSize; // 设置缓冲区大小
		ret = pDataUnit->GetData(1, SQL_C_CHAR, szPassword, dataSize);
		if (ret == SE_OK)
		{
			// 确保字符串正确终止
			szPassword[bufferSize - 1] = '\0';
			if (strcmp(szPassword, pszPassword) == 0)
				ret = SE_OK;
			else
				ret = SE_LOGIN_PASSWORDERROR;
		}
		else
			ret = SE_LOGIN_ACCOUNTNOTEXIST;
	}
	else if (ret == SE_DB_NOMOREDATA)
		ret = SE_LOGIN_ACCOUNTNOTEXIST;
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::ChangePassword(const char* pszAccount, const char* pszOldPass, const char* pszNewPass)
{
	if (!xCheckPassword(pszNewPass)) return SE_LOGIN_PASSWORDERROR;
	SERVER_ERROR ret = CheckAccount(pszAccount, pszOldPass);
	if (ret == SE_OK)
	{
		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("update TBL_ACCOUNT set password = '%s' where account = '%s'", pszNewPass, pszAccount);
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::CheckAccountExist(const char* pszAccount)
{
	if (!xCheckAccount(pszAccount)) return SE_LOGIN_ACCOUNTNOTEXIST;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select account from TBL_ACCOUNT where account = '%s'", pszAccount);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_DB_NOMOREDATA) return SE_LOGIN_ACCOUNTNOTEXIST;
	if (ret == SE_OK) return SE_LOGIN_ACCOUNTEXIST;
	return ret;
}

SERVER_ERROR CAppDB::UpdateAccountState(DWORD dwServerId, const char* pszAccount, UINT state)
{
	return SERVER_ERROR();
}

SERVER_ERROR CAppDB::GetCharList(const char* pszAccount, const char* pszServerName, tQueryCharList_Result* pResult)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select NAME,CLASS,SEX,VLEVEL,HAIR,ODATE from TBL_CHARACTER_INFO where ACCOUNT = '%s' and SERVER = '%s' AND DELFLAG = 0", pszAccount, pszServerName);
	int count = 0;
	if (ret == SE_OK)
	{
		SELECT_CHAR_LIST* clist = pResult->charlist;
		CHAR szDate[200] = "";
		CSystemTime	stTime[2];
		do
		{
			int size = 19;
			pDataUnit->GetData(1, SQL_C_CHAR, (LPVOID)clist[count].szName, size);
			size = 0;
			pDataUnit->GetData(2, SQL_TINYINT, (LPVOID)&clist[count].btClass, size);
			size = 0;
			pDataUnit->GetData(3, SQL_TINYINT, (LPVOID)&clist[count].btSex, size);
			size = 0;
			pDataUnit->GetData(4, SQL_SMALLINT, (LPVOID)&clist[count].wLevel, size);
			size = 0;
			pDataUnit->GetData(5, SQL_TINYINT, (LPVOID)&clist[count].btHair, size);
			size = 200;
			pDataUnit->GetData(6, SQL_C_CHAR, (LPVOID)szDate, size);
			stTime[count] = szDate;
			pResult->charlist[count] = clist[count];
			count++;
			if (count >= 2)
				break;
		} while (pDataUnit->MoveNext() == SE_OK);

		if (count == 1)
		{
			clist[0].date.year = stTime[0].GetYear();
			clist[0].date.month = stTime[0].GetMonth();
			clist[0].date.day = stTime[0].GetDay();
			clist[0].date.hour = stTime[0].GetHour();
			clist[0].date.minute = stTime[0].GetMinute();
		}
		else if (count == 2)
		{
			if (stTime[0] > stTime[1])
				clist[0].date.bflag = 1, clist[1].date.bflag = 0;
			else
				clist[0].date.bflag = 0, clist[1].date.bflag = 1;
			clist[0].date.year = stTime[0].GetYear();
			clist[0].date.month = stTime[0].GetMonth();
			clist[0].date.day = stTime[0].GetDay();
			clist[0].date.hour = stTime[0].GetHour();
			clist[0].date.minute = stTime[0].GetMinute();
			clist[1].date.year = stTime[1].GetYear();
			clist[1].date.month = stTime[1].GetMonth();
			clist[1].date.day = stTime[1].GetDay();
			clist[1].date.hour = stTime[1].GetHour();
			clist[1].date.minute = stTime[1].GetMinute();
		}
	}
	if (ret == SE_DB_NOMOREDATA)
	{
		ret = SE_OK;
		count = 0;
	}
	//ret = SE_OK ,count = 0;
	pResult->count = count;
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::CreateCharacter(CREATECHARDESC* pDesc)
{
	SERVER_ERROR ret = CheckCharacterExist(pDesc->szAccount, pDesc->szServer, pDesc->szName);
	if (!xCheckAccount(pDesc->szAccount))
	{
		CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "检测szAccount返回错误\n");
		return SE_CREATECHARACTER_INVALID_CHARNAME;
	}
	if (!xCheckCharname(pDesc->szName)) {
		CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "检测szName返回错误\n");
		return SE_CREATECHARACTER_INVALID_CHARNAME;
	}
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "SE_SELCHAR_CHAREXIST返回错误\n");
		return ret;
	}
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
	{
		CServer::GetInstance()->GetIoConsole()->OutPut(ERROR_RED, "pDataUnit == nullptr返回错误\n");
		return SE_ALLOCMEMORYFAIL;
	}

	ret = pDataUnit->Operation("insert into TBL_CHARACTER_INFO(ACCOUNT,SERVER,NAME,CLASS,SEX,VLEVEL,HAIR,CREATEDATE) "
		"VALUES('%s', '%s', '%s', %u, %u, %u, %u, CURRENT_TIMESTAMP)",
		pDesc->szAccount, pDesc->szServer, pDesc->szName, pDesc->btClass, pDesc->btSex, pDesc->btLevel, pDesc->btHair);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::CheckCharacterExist(const char* pszAccount, const char* pszServerName, const char* pszName)
{
	if (!xCheckAccount(pszAccount))return SE_SELCHAR_NOTEXIST;
	if (!xCheckCharname(pszName))return SE_SELCHAR_NOTEXIST;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select ID from TBL_CHARACTER_INFO where server = '%s' and name = '%s'", pszServerName, pszName);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_OK) return SE_SELCHAR_CHAREXIST;
	return SE_SELCHAR_NOTEXIST;
}

SERVER_ERROR CAppDB::DelCharacter(const char* pszAccount, const char* pszServerName, const char* pszName)
{
	SERVER_ERROR ret = CheckCharacterExist(pszAccount, pszServerName, pszName);
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("update TBL_CHARACTER_INFO set DELFLAG = 1, deldate = CURRENT_TIMESTAMP where account = '%s' and server = '%s' and name = '%s'", pszAccount, pszServerName, pszName);
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::GetDelCharList(const char* pszAccount, const char* pszServerName, tQueryCharList_Result* pResult)
{
	if (!xCheckAccount(pszAccount))
	{
		pResult->count = 0;
		return SE_OK;
	}
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select NAME,CLASS,SEX,VLEVEL,HAIR,ODATE from TBL_CHARACTER_INFO where ACCOUNT = '%s' and SERVER = '%s' AND DELFLAG = 1", pszAccount, pszServerName);
	int count = 0;
	if (ret == SE_OK)
	{
		SELECT_CHAR_LIST* clist = nullptr;
		CHAR szDate[200] = { 0 };
		CSystemTime	stTime[1];
		do
		{
			clist = &pResult->charlist[count];
			int size = 19;
			pDataUnit->GetData(1, SQL_C_CHAR, (LPVOID)clist[0].szName, size);
			size = 0;
			pDataUnit->GetData(2, SQL_TINYINT, (LPVOID)&clist[0].btClass, size);
			size = 0;
			pDataUnit->GetData(3, SQL_TINYINT, (LPVOID)&clist[0].btSex, size);
			size = 0;
			pDataUnit->GetData(4, SQL_SMALLINT, (LPVOID)&clist[0].wLevel, size);
			size = 0;
			pDataUnit->GetData(5, SQL_TINYINT, (LPVOID)&clist[0].btHair, size);
			size = 200;
			pDataUnit->GetData(6, SQL_C_CHAR, (LPVOID)szDate, size);
			stTime[0] = szDate;
			clist[0].date.year = stTime[0].GetYear();
			clist[0].date.month = stTime[0].GetMonth();
			clist[0].date.day = stTime[0].GetDay();
			clist[0].date.hour = stTime[0].GetHour();
			clist[0].date.minute = stTime[0].GetMinute();
			count++;
			if (count >= MAX_DELCHARLISTCOUNT)break;
		} while (pDataUnit->MoveNext() == SE_OK);
	}
	if (ret == SE_DB_NOMOREDATA)
		ret = SE_OK, count = 0;
	pResult->count = count;
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::RestoreCharacter(const char* pszAccount, const char* pszServerName, const char* pszName)
{
	SERVER_ERROR ret = CheckCharacterExist(pszAccount, pszServerName, pszName);
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("update TBL_CHARACTER_INFO set DELFLAG = 0 where account = '%s' and server = '%s' and name = '%s'", pszAccount, pszServerName, pszName);
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::GetMapPosition(const char* pszAccount, const char* pszServerName, const char* pszName, tQueryMapPosition_Result* pResult)
{
	SERVER_ERROR ret = CheckCharacterExist(pszAccount, pszServerName, pszName);
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		int size = 32;
		ret = pDataUnit->Operation("select MAPNAME,POSX,POSY from TBL_CHARACTER_INFO where account = '%s' and server = '%s' and name = '%s'", pszAccount, pszServerName, pszName);
		if (ret == SE_OK)
		{
			pDataUnit->GetData(1, SQL_C_CHAR, (LPVOID)pResult->szName, size);
			size = 0;
			pDataUnit->GetData(2, SQL_SMALLINT, (LPVOID)&pResult->x, size);
			size = 0;
			pDataUnit->GetData(3, SQL_SMALLINT, (LPVOID)&pResult->y, size);
		}
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::GetCharDBInfo(const char* pszAccount, const char* pszServerName, const char* pszName, CHARDBINFO* pInfo)
{
	SERVER_ERROR ret = CheckCharacterExist(pszAccount, pszServerName, pszName);
	if (ret == SE_SELCHAR_CHAREXIST)
	{
		CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("select "
			"ID, CLASS, SEX,"
			"VLEVEL, MAPNAME, POSX, POSY, DIR, ATTACKMODE, HAIR,"
			"CUREXP, HP, MP, MAXHP, MAXMP, MINDC, MAXDC,"
			"MINMC, MAXMC, MINSC, MAXSC, MINAC, MAXAC,"
			"MINMAC, MAXMAC,WEIGHT, HANDWEIGHT, BODYWEIGHT,"
			"GAMETIME, GOLD, MAPID, YUANBAO, BIGGOLD, BIGBAG,"
			"PERSONCODE, PERSONSIGN, TEMPRANK,"
			"FLAG1, FLAG2, FLAG3, FLAG4, GUILDNAME, FORGEPOINT,"
			"PROP1, PROP2, PROP3, PROP4, PROP5, PROP6, PROP7, PROP8"
			" from TBL_CHARACTER_INFO where account = '%s' and server = '%s' and name = '%s'", pszAccount, pszServerName, pszName);
		if (ret == SE_OK)
		{
			strncpy(pInfo->szName, pszName, 18);
			pInfo->szName[18] = 0;
			pDataUnit->GetValue(pInfo->dwDBId);
			pDataUnit->GetValue(pInfo->btClass);
			pDataUnit->GetValue(pInfo->btSex);

			pDataUnit->GetValue(pInfo->wLevel);
			pDataUnit->GetValue(pInfo->szStartPoint, 16);
			pDataUnit->GetValue(pInfo->x);
			pDataUnit->GetValue(pInfo->y);
			pDataUnit->GetValue(pInfo->dir);
			pDataUnit->GetValue(pInfo->btAttackMode);
			pDataUnit->GetValue(pInfo->btHair);

			pDataUnit->GetValue(pInfo->dwCurExp);
			pDataUnit->GetValue(pInfo->hp);
			pDataUnit->GetValue(pInfo->mp);
			pDataUnit->GetValue(pInfo->maxhp);
			pDataUnit->GetValue(pInfo->maxmp);
			pDataUnit->GetValue(pInfo->mindc);
			pDataUnit->GetValue(pInfo->maxdc);

			pDataUnit->GetValue(pInfo->minmc);
			pDataUnit->GetValue(pInfo->maxmc);
			pDataUnit->GetValue(pInfo->minsc);
			pDataUnit->GetValue(pInfo->maxsc);
			pDataUnit->GetValue(pInfo->minac);
			pDataUnit->GetValue(pInfo->maxac);

			pDataUnit->GetValue(pInfo->minmac);
			pDataUnit->GetValue(pInfo->maxmac);
			pDataUnit->GetValue(pInfo->weight);
			pDataUnit->GetValue(pInfo->handweight);
			pDataUnit->GetValue(pInfo->bodyweight);

			pDataUnit->GetValue(pInfo->nGameTime);
			pDataUnit->GetValue(pInfo->dwGold);
			pDataUnit->GetValue(pInfo->mapid);
			pDataUnit->GetValue(pInfo->dwYuanbao);
			pDataUnit->GetValue(pInfo->bBigGold);
			pDataUnit->GetValue(pInfo->bBigBag);

			pDataUnit->GetValue(pInfo->wPersonCode);
			pDataUnit->GetValue(pInfo->szPersonSign, 20);
			pDataUnit->GetValue(pInfo->szTempRank, 60);

			pDataUnit->GetValue(pInfo->dwFlag[0]);
			pDataUnit->GetValue(pInfo->dwFlag[1]);
			pDataUnit->GetValue(pInfo->dwFlag[2]);
			pDataUnit->GetValue(pInfo->dwFlag[3]);
			pDataUnit->GetValue(pInfo->szGuildName, 20);
			pDataUnit->GetValue(pInfo->dwForgePoint);

			pDataUnit->GetValue(pInfo->dwProp[0]);
			pDataUnit->GetValue(pInfo->dwProp[1]);
			pDataUnit->GetValue(pInfo->dwProp[2]);
			pDataUnit->GetValue(pInfo->dwProp[3]);
			pDataUnit->GetValue(pInfo->dwProp[4]);
			pDataUnit->GetValue(pInfo->dwProp[5]);
			pDataUnit->GetValue(pInfo->dwProp[6]);
			pDataUnit->GetValue(pInfo->dwProp[7]);
		}
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::PutCharDBInfo(CHARDBINFO* pInfo)
{
	SERVER_ERROR ret = SE_FAIL;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("update TBL_CHARACTER_INFO "
		"set VLEVEL = %d, MAPNAME = '%s', POSX = %d, POSY = %d, DIR = %u, ATTACKMODE = %u, HAIR = %u,"
		"CUREXP = %d, HP = %d, MP = %d, MAXHP = %d, MAXMP = %d, MINDC = %u, MAXDC = %u,"
		"MINMC = %u, MAXMC = %u, MINSC = %u, MAXSC = %u, MINAC = %u, MAXAC = %u,"
		"MINMAC = %u, MAXMAC = %u,WEIGHT = %d, HANDWEIGHT = %u, BODYWEIGHT = %u,"
		"GAMETIME = %d, GOLD = %d, MAPID = %d, ODATE = CURRENT_TIMESTAMP, YUANBAO = %d, BIGGOLD = %d, BIGBAG = %d,"
		"PERSONCODE = %d, PERSONSIGN = '%s', TEMPRANK = '%s',"
		"FLAG1 = %d, FLAG2 = %d, FLAG3 = %d, FLAG4 = %d, GUILDNAME = '%s',FORGEPOINT = %d,"
		"PROP1 = %d, PROP2 = %d, PROP3 = %d, PROP4 = %d, PROP5 = %d, PROP6 = %d, PROP7 = %d, PROP8 = %d"
		" where ID = %u",
		pInfo->wLevel, pInfo->szStartPoint, pInfo->x, pInfo->y, pInfo->dir, pInfo->btAttackMode, pInfo->btHair,
		pInfo->dwCurExp, pInfo->hp, pInfo->mp, pInfo->maxhp, pInfo->maxmp, pInfo->mindc, pInfo->maxdc,
		pInfo->minmc, pInfo->maxmc, pInfo->minsc, pInfo->maxsc, pInfo->minac, pInfo->maxac,
		pInfo->minmac, pInfo->maxmac, pInfo->weight, pInfo->handweight, pInfo->bodyweight,
		pInfo->nGameTime, pInfo->dwGold, pInfo->mapid, pInfo->dwYuanbao, pInfo->bBigGold, pInfo->bBigBag,
		pInfo->wPersonCode, pInfo->szPersonSign, pInfo->szTempRank,
		pInfo->dwFlag[0], pInfo->dwFlag[1], pInfo->dwFlag[2], pInfo->dwFlag[3], pInfo->szGuildName, pInfo->dwForgePoint,
		pInfo->dwProp[0], pInfo->dwProp[1], pInfo->dwProp[2], pInfo->dwProp[3], pInfo->dwProp[4], pInfo->dwProp[5], pInfo->dwProp[6], pInfo->dwProp[7],
		pInfo->dwDBId
	);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::GetFreeItemId(DWORD& dwItemIndex)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select ID from TBL_CHARACTER_ITEM where DELFLAG = 1 LIMIT 1");
	if (ret == SE_OK)
	{
		int size = 0;
		ret = pDataUnit->GetData(1, SQL_INTEGER, &dwItemIndex, size);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::FindItemId(DWORD dwOwner, BYTE btFlag, WORD wPos, DWORD dwFindKey, DWORD& dwItemIndex)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select ID from TBL_CHARACTER_ITEM where "
		"OWNERID = %u and Flag = %u and POS = %hd and FINDKEY = %u LIMIT 1",
		dwOwner, btFlag, wPos, dwFindKey);
	if (ret == SE_OK)
	{
		int size = 0;
		ret = pDataUnit->GetData(1, SQL_INTEGER, &dwItemIndex, size);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_OK)
	{
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		pDataUnit->Operation("update TBL_CHARACTER_ITEM "
			"set FINDKEY = 0 "
			"where ID = %u", dwItemIndex);
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

SERVER_ERROR CAppDB::UpgradeItem(DWORD dwMakeIndex, DWORD dwUpgrade)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
		return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_ITEM "
		"set NEEDIDENTIFY = %d, FLAG = %u, FINDKEY = %u "
		"where ID = %u", 1, IDF_UPGRADE, dwUpgrade, dwMakeIndex);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::CreateItem(DWORD dwOwner, BYTE	btFlag, WORD wPos, ITEM* pItem)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	char szName[32];
	char szFullName[32];
	DWORD FindKey = static_cast<DWORD>(Getrand(0x7fffffff)) | timeGetTime();
	o_strncpy(szName, pItem->baseitem.szName, 14);
	o_strncpy(szFullName, pItem->szFullName, 16);
	// 将 btItemExt 转换为 HEX 字符串
	char szExtHex[721] = ""; // 360 * 2 + 1
	for (int i = 0; i < 360; i++)
	{
		sprintf_s(&szExtHex[i * 2], 3, "%02X", (unsigned char)pItem->btItemExt[i]);
	}
	SERVER_ERROR ret = pDataUnit->Operation("insert into TBL_CHARACTER_ITEM( "
		"OWNERID,NAME,FULLNAME,MINDC,MAXDC,MINMC,MAXMC,MINSC,MAXSC,MINAC,MAXAC,"
		"MINMAC,MAXMAC,DURA,CURDURA,MAXDURA,NEEDTYPE,NEEDLEVEL,SPECIALPOWER,NEEDIDENTIFY,"
		"WEIGHT,STDMODE,SHAPE,PRICE,UNKNOWN_1,UNKNOWN_2,FLAG,POS,FINDKEY,IMAGEINDEX,EXT,CREATEDATE)"
		" VALUES("
		"%u,'%s','%s',%u,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,UNHEX('%s'),CURRENT_TIMESTAMP)",
		dwOwner, szName, szFullName, pItem->baseitem.btMinAtk, pItem->baseitem.btMaxAtk, pItem->baseitem.btMinMagAtk, pItem->baseitem.btMaxMagAtk,
		pItem->baseitem.btMinSouAtk, pItem->baseitem.btMaxSouAtk, pItem->baseitem.btMinDef, pItem->baseitem.btMaxDef,
		pItem->baseitem.btMinMagDef, pItem->baseitem.btMaxMagDef, pItem->baseitem.wMaxDura, pItem->wCurDura, pItem->wMaxDura, pItem->baseitem.needtype,
		pItem->baseitem.needvalue, pItem->baseitem.btSpecialpower, pItem->baseitem.bNeedIdentify, pItem->baseitem.btWeight,
		pItem->baseitem.btStdMode, pItem->baseitem.btShape, pItem->baseitem.nPrice, pItem->baseitem.btPriceType,
		pItem->baseitem.wFeature, btFlag, wPos, FindKey, pItem->baseitem.wImageIndex, szExtHex
	);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_OK)
		ret = FindItemId(dwOwner, btFlag, wPos, FindKey, pItem->dwMakeIndex);
	return ret;
}

SERVER_ERROR CAppDB::CreateItemEx(DWORD dwOwner, BYTE btFlag, WORD wPos, ITEM* pItem)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	char szName[32];
	char szFullName[32];
	o_strncpy(szName, pItem->baseitem.szName, 14);
	o_strncpy(szFullName, pItem->szFullName, 16);
	// 将 btItemExt 转换为 HEX 字符串
	char szExtHex[721] = ""; // 360 * 2 + 1
	for (int i = 0; i < 360; i++)
	{
		sprintf_s(&szExtHex[i * 2], 3, "%02X", (unsigned char)pItem->btItemExt[i]);
	}
	SERVER_ERROR ret = pDataUnit->Operation("insert into TBL_CHARACTER_ITEM("
		"OWNERID,NAME,FULLNAME,MINDC,MAXDC,MINMC,MAXMC,MINSC,MAXSC,MINAC,MAXAC,"
		"MINMAC,MAXMAC,DURA,CURDURA,MAXDURA,NEEDTYPE,NEEDLEVEL,SPECIALPOWER,NEEDIDENTIFY,"
		"WEIGHT,STDMODE,SHAPE,PRICE,UNKNOWN_1,UNKNOWN_2,FLAG,POS,FINDKEY,IMAGEINDEX,EXT,CREATEDATE)"
		" VALUES("
		"%u,'%s','%s',%u,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,%u,%u,%u,"
		"%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,UNHEX('%s'),CURRENT_TIMESTAMP)",
		dwOwner, szName, szFullName, pItem->baseitem.btMinAtk, pItem->baseitem.btMaxAtk, pItem->baseitem.btMinMagAtk, pItem->baseitem.btMaxMagAtk,
		pItem->baseitem.btMinSouAtk, pItem->baseitem.btMaxSouAtk, pItem->baseitem.btMinDef, pItem->baseitem.btMaxDef,
		pItem->baseitem.btMinMagDef, pItem->baseitem.btMaxMagDef, pItem->baseitem.wMaxDura, pItem->wCurDura, pItem->wMaxDura, pItem->baseitem.needtype,
		pItem->baseitem.needvalue, pItem->baseitem.btSpecialpower, pItem->baseitem.bNeedIdentify, pItem->baseitem.btWeight,
		pItem->baseitem.btStdMode, pItem->baseitem.btShape, pItem->baseitem.nPrice, pItem->baseitem.btPriceType, pItem->baseitem.wFeature,
		btFlag, wPos, pItem->dwParam[0], pItem->baseitem.wImageIndex, szExtHex
	);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateItem(DWORD	dwOwner, BYTE	btFlag, WORD wPos, ITEM* pItem)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	char szName[32];
	char szFullName[32];
	o_strncpy(szName, pItem->baseitem.szName, 14);
	o_strncpy(szFullName, pItem->szFullName, 16);
	// 将 btItemExt 转换为 HEX 字符串
	char szExtHex[721] = ""; // 360 * 2 + 1
	for (int i = 0; i < 360; i++)
	{
		sprintf_s(&szExtHex[i * 2], 3, "%02X", (unsigned char)pItem->btItemExt[i]);
	}
	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_ITEM "
		"set OWNERID = %u, NAME = '%s', FULLNAME = '%s', MINDC = %u, MAXDC = %u, MINMC = %u, MAXMC = %u, MINSC = %u, MAXSC = %u, MINAC = %u, MAXAC = %u, "
		"MINMAC = %u, MAXMAC = %u, DURA = %u, CURDURA = %u, MAXDURA = %u, NEEDTYPE = %u, NEEDLEVEL = %u, SPECIALPOWER = %u, NEEDIDENTIFY = %u, "
		"WEIGHT = %u, STDMODE = %d, SHAPE = %u, PRICE = %u, UNKNOWN_1 = %u, UNKNOWN_2 = %u, FLAG = %u, POS = %u, FINDKEY = %u, IMAGEINDEX = %u, EXT = UNHEX('%s'), DELFLAG = 0 "
		"where ID = %u",
		dwOwner, szName, szFullName, pItem->baseitem.btMinAtk, pItem->baseitem.btMaxAtk, pItem->baseitem.btMinMagAtk, pItem->baseitem.btMaxMagAtk,
		pItem->baseitem.btMinSouAtk, pItem->baseitem.btMaxSouAtk, pItem->baseitem.btMinDef, pItem->baseitem.btMaxDef,
		pItem->baseitem.btMinMagDef, pItem->baseitem.btMaxMagDef, pItem->baseitem.wMaxDura, pItem->wCurDura, pItem->wMaxDura, pItem->baseitem.needtype,
		pItem->baseitem.needvalue, pItem->baseitem.btSpecialpower, pItem->baseitem.bNeedIdentify, pItem->baseitem.btWeight,
		pItem->baseitem.btStdMode, pItem->baseitem.btShape, pItem->baseitem.nPrice, pItem->baseitem.btPriceType, pItem->baseitem.wFeature,
		btFlag, wPos, pItem->dwParam[0], pItem->baseitem.wImageIndex, szExtHex, pItem->dwMakeIndex
	);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::DeleteItem(DWORD dwItemIndex)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("DELETE FROM TBL_CHARACTER_ITEM "
		"where ID = %u", dwItemIndex);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateItemPos(DWORD dwItemIndex, BYTE btFlag, WORD wPos)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_ITEM "
		"set FLAG = %u, POS = %u "
		"where ID = %u", btFlag, wPos, dwItemIndex);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateItemPosEx(BYTE btFlag, WORD wCount, BAGITEMPOS* pItemPos)
{
	for (int i = 0; i < wCount; i++)
	{
		UpdateItemPos(pItemPos[i].ItemId, btFlag, pItemPos[i].wPos);
	}
	return SE_OK;
}

SERVER_ERROR CAppDB::UpdateItemOwner(DWORD dwItemIndex, DWORD dwOwner, BYTE btFlag, WORD wPos)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_ITEM "
		"set OWNERID = %u,FLAG = %u, POS = %u "
		"where ID = %u", dwOwner, btFlag, wPos, dwItemIndex);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::QueryItems(DWORD dwOwner, BYTE	btFlag, DBITEM* pItemBuffer, int& count)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	int getcount = 0;
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select "
		"ID,NAME,FULLNAME,MINDC,MAXDC,MINMC,MAXMC,MINSC,MAXSC,MINAC,MAXAC,"
		"MINMAC,MAXMAC,DURA,CURDURA,MAXDURA,NEEDTYPE,NEEDLEVEL,SPECIALPOWER,NEEDIDENTIFY,"
		"WEIGHT,STDMODE,SHAPE,PRICE,UNKNOWN_1,UNKNOWN_2,POS,FINDKEY,IMAGEINDEX,HEX(EXT) "
		"from TBL_CHARACTER_ITEM "
		"where OWNERID = %u and FLAG = %u and DELFLAG = 0 LIMIT %d", dwOwner, btFlag, count);
	if (ret == SE_OK)
	{
		memset(pItemBuffer, 0, sizeof(DBITEM) * count);  // 清空缓冲区, 避免旧数据残留
		do
		{
			DBITEM* p = &pItemBuffer[getcount];
			char szName[32];
			char szFullName[32];
			char szExtHex[721] = "";  // 360 * 2 + 1
			pDataUnit->GetValue(p->item.dwMakeIndex);
			pDataUnit->GetValue(szName, 32);
			pDataUnit->GetValue(szFullName, 32);
			pDataUnit->GetValue(p->item.baseitem.btMinAtk);
			pDataUnit->GetValue(p->item.baseitem.btMaxAtk);
			pDataUnit->GetValue(p->item.baseitem.btMinMagAtk);
			pDataUnit->GetValue(p->item.baseitem.btMaxMagAtk);
			pDataUnit->GetValue(p->item.baseitem.btMinSouAtk);
			pDataUnit->GetValue(p->item.baseitem.btMaxSouAtk);
			pDataUnit->GetValue(p->item.baseitem.btMinDef);
			pDataUnit->GetValue(p->item.baseitem.btMaxDef);
			pDataUnit->GetValue(p->item.baseitem.btMinMagDef);
			pDataUnit->GetValue(p->item.baseitem.btMaxMagDef);
			pDataUnit->GetValue(p->item.baseitem.wMaxDura);
			pDataUnit->GetValue(p->item.wCurDura);
			pDataUnit->GetValue(p->item.wMaxDura);
			pDataUnit->GetValue(p->item.baseitem.needtype);
			pDataUnit->GetValue(p->item.baseitem.needvalue);
			pDataUnit->GetValue(p->item.baseitem.btSpecialpower);
			pDataUnit->GetValue(p->item.baseitem.bNeedIdentify);
			pDataUnit->GetValue(p->item.baseitem.btWeight);
			pDataUnit->GetValue(p->item.baseitem.btStdMode);
			pDataUnit->GetValue(p->item.baseitem.btShape);
			pDataUnit->GetValue(p->item.baseitem.nPrice);
			pDataUnit->GetValue(p->item.baseitem.btPriceType);
			pDataUnit->GetValue(p->item.baseitem.wFeature);
			pDataUnit->GetValue(p->pos);
			pDataUnit->GetValue(p->item.dwParam[0]);
			pDataUnit->GetValue(p->item.baseitem.wImageIndex);
			pDataUnit->GetValue(szExtHex, 721);
			// 将 HEX 字符串转换回二进制数据
			for (int i = 0; i < 360 && szExtHex[i * 2] != '\0'; i++)
			{
				char high = szExtHex[i * 2];
				char low = szExtHex[i * 2 + 1];
				BYTE val = 0;
				// 处理高位
				if (high >= '0' && high <= '9')
					val |= (high - '0') << 4;
				else if (high >= 'A' && high <= 'F')
					val |= (high - 'A' + 10) << 4;
				else if (high >= 'a' && high <= 'f')
					val |= (high - 'a' + 10) << 4;
				// 处理低位
				if (low >= '0' && low <= '9')
					val |= (low - '0');
				else if (low >= 'A' && low <= 'F')
					val |= (low - 'A' + 10);
				else if (low >= 'a' && low <= 'f')
					val |= (low - 'a' + 10);
				p->item.btItemExt[i] = val;
			}
			p->btFlag = btFlag;

			if (p->item.baseitem.wMaxDura < p->item.wMaxDura)
				p->item.baseitem.wMaxDura = p->item.wMaxDura;
			//	设置名字
			p->item.baseitem.btNameLength = static_cast<BYTE>(strlen(szName));
			strncpy(p->item.baseitem.szName, szName, 14);
			strncpy(p->item.szFullName, szFullName, 16);
			getcount++;
		} while (pDataUnit->MoveNext() == SE_OK && getcount < count);
	}
	count = getcount;
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_DB_NOMOREDATA)
		ret = SE_OK, count = 0;
	return ret;
}

SERVER_ERROR CAppDB::UpdateMagic(DWORD dwOwner, MAGICDB* pMagic, int count)
{
	CVirtualDataUnit* pDataUnit = nullptr;
	SERVER_ERROR ret = SE_OK;
	for (int i = 0; i < count; i++)
	{
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		ret = pDataUnit->Operation("select * "
			"from tbl_character_magic "
			"where CHARID = %u AND MAGICID = %u LIMIT 1", dwOwner, pMagic[i].wId);
		m_pDBConnection->DelDataUnit(pDataUnit);

		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		if (ret == SE_OK)
		{
			ret = pDataUnit->Operation("update tbl_character_magic "
				"set USERKEY = %u, CURLEVEL = %u, CURTRAIN = %u ,LEARNDATE = CURRENT_TIMESTAMP "
				"where CHARID = %u AND MAGICID = %u", pMagic[i].btKey, pMagic[i].btLevel, pMagic[i].dwCurTrain, dwOwner, pMagic[i].wId);
		}
		else
		{
			ret = pDataUnit->Operation("insert into tbl_character_magic("
				"CHARID, USERKEY, CURLEVEL, MAGICID, CURTRAIN, LEARNDATE) "
				"VALUES(%u, %u, %u, %u, %u, CURRENT_TIMESTAMP)",
				dwOwner, pMagic[i].btKey, pMagic[i].btLevel, pMagic[i].wId, pMagic[i].dwCurTrain);
		}
		m_pDBConnection->DelDataUnit(pDataUnit);
	}
	return ret;
}

static int	qsort_cmpmagic(MAGICDB* p1, MAGICDB* p2)
{
	if (p1->wId > p2->wId)return -1;
	if (p1->wId == p2->wId)return 0;
	return 1;
}

SERVER_ERROR CAppDB::QueryMagic(DWORD dwOwner, MAGICDB* pMagicArray, int& count)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	int getcount = 0;
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select USERKEY,CURLEVEL,MAGICID,CURTRAIN "
		"FROM TBL_CHARACTER_MAGIC "
		"where CHARID = %d  LIMIT %d", dwOwner, count);
	memset(pMagicArray, 0, sizeof(MAGICDB) * count); // 清空缓冲区, 避免旧数据残留
	if (ret == SE_OK)
	{
		do
		{
			pDataUnit->GetValue(pMagicArray[getcount].btKey);
			pDataUnit->GetValue(pMagicArray[getcount].btLevel);
			pDataUnit->GetValue(pMagicArray[getcount].wId);
			pDataUnit->GetValue(pMagicArray[getcount].dwCurTrain);
			getcount++;
		} while (pDataUnit->MoveNext() == SE_OK && getcount < count);
	}

	count = getcount;
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_DB_NOMOREDATA)
		ret = SE_OK, count = 0;
	else
		qsort(pMagicArray, count, sizeof(MAGICDB), (int(*)(const void*, const void*))qsort_cmpmagic);
	return ret;
}

SERVER_ERROR CAppDB::UpdateCommunity(DWORD dwOwner, const char* pszCommunity)
{
	char* szText = g_szTempBuffer;
	o_strncpy(szText, pszCommunity, 65535);
	xStringsExpander<200> community((char*)szText, '/');

	char* name;
	char* wife;
	char* master;
	char* friends[32] = { 0 };
	char* students[3] = { 0 };

	name = community[0] != nullptr ? community[0] : "";
	wife = community[1] != nullptr ? community[1] : "";
	master = community[2] != nullptr ? community[2] : "";
	if (name[0] == 0)return SE_DB_NOMOREDATA;
	for (int i = 0; i < 3; i++)
	{
		students[i] = community[3 + i] != nullptr ? community[3 + i] : "";
	}
	for (int i = 0; i < 32; i++)
	{
		friends[i] = community[6 + i] != nullptr ? community[6 + i] : "";
	}

	SERVER_ERROR ret = SE_OK;
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	ret = pDataUnit->Operation("select * from TBL_CHARACTER_COMMUNITY where OWNERID = %u LIMIT 1", dwOwner);
	m_pDBConnection->DelDataUnit(pDataUnit);

	pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	if (ret == SE_OK)
	{
		ret = pDataUnit->Operation("update TBL_CHARACTER_COMMUNITY "
			"set name = '%s',marriage = '%s',master = '%s',"
			"student1 = '%s',student2 = '%s',student3 = '%s',"
			"friend1 = '%s',friend2 = '%s',friend3 = '%s',friend4 = '%s',friend5 = '%s',friend6 = '%s',friend7 = '%s',friend8 = '%s',friend9 = '%s',friend10 = '%s',"
			"friend11 = '%s',friend12 = '%s',friend13 = '%s',friend14 = '%s',friend15 = '%s',friend16 = '%s',friend17 = '%s',friend18 = '%s',friend19 = '%s',friend20 = '%s',"
			"friend21 = '%s',friend22 = '%s',friend23 = '%s',friend24 = '%s',friend25 = '%s',friend26 = '%s',friend27 = '%s',friend28 = '%s',friend29 = '%s',friend30 = '%s',"
			"friend31 = '%s',friend32 = '%s'"
			" WHERE OWNERID = %d",
			name, wife, master,
			students[0], students[1], students[2],
			friends[0], friends[1], friends[2], friends[3], friends[4], friends[5], friends[6], friends[7], friends[8], friends[9],
			friends[10], friends[11], friends[12], friends[13], friends[14], friends[15], friends[16], friends[17], friends[18], friends[19],
			friends[20], friends[21], friends[22], friends[23], friends[24], friends[25], friends[26], friends[27], friends[28], friends[29],
			friends[30], friends[31],
			dwOwner);
	}
	else
	{
		ret = pDataUnit->Operation("insert into TBL_CHARACTER_COMMUNITY("
			"OWNERID,name,marriage,master,"
			"student1,student2,student3,"
			"friend1,friend2,friend3,friend4,friend5,friend6,friend7,friend8,friend9,friend10,"
			"friend11,friend12,friend13,friend14,friend15,friend16,friend17,friend18,friend19,friend20,"
			"friend21,friend22,friend23,friend24,friend25,friend26,friend27,friend28,friend29,friend30,"
			"friend31,friend32"
			") VALUES(%d,'%s','%s','%s',"
			"'%s','%s','%s',"
			"'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',"
			"'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',"
			"'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',"
			"'%s','%s'"
			")",
			dwOwner, name, wife, master,
			students[0], students[1], students[2],
			friends[0], friends[1], friends[2], friends[3], friends[4], friends[5], friends[6], friends[7], friends[8], friends[9],
			friends[10], friends[11], friends[12], friends[13], friends[14], friends[15], friends[16], friends[17], friends[18], friends[19],
			friends[20], friends[21], friends[22], friends[23], friends[24], friends[25], friends[26], friends[27], friends[28], friends[29],
			friends[30], friends[31]);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

typedef char SHORTNAME[64];
SERVER_ERROR CAppDB::QueryCommunity(DWORD dwOwner, char* pszCommunity, int& iBufferSize)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select * "
		"FROM TBL_CHARACTER_COMMUNITY "
		"where OWNERID = %d", dwOwner);
	SHORTNAME friends[32] = { 0 };
	SHORTNAME wife;
	SHORTNAME master;
	SHORTNAME students[3] = { 0 };
	SHORTNAME name;
	xPacket packet(pszCommunity, iBufferSize);
	if (ret == SE_OK)
	{
		pDataUnit->GetValue(dwOwner);
		pDataUnit->GetValue(name, 64);
		//	wife
		pDataUnit->GetValue(wife, 64);
		packet.push((LPVOID)wife, static_cast<int>(strlen(wife)));
		packet.push((LPVOID)"/", 1);
		//	wife
		pDataUnit->GetValue(master, 64);
		packet.push((LPVOID)master, static_cast<int>(strlen(master)));
		packet.push((LPVOID)"/", 1);
		//	student~
		pDataUnit->GetValue(students[0], 64);
		pDataUnit->GetValue(students[1], 64);
		pDataUnit->GetValue(students[2], 64);
		for (int i = 0; i < 3; i++)
		{
			packet.push((LPVOID)students[i], static_cast<int>(strlen(students[i])));
			packet.push((LPVOID)"/", 1);
		}
		//	friends
		pDataUnit->GetValue(friends[0], 64);
		pDataUnit->GetValue(friends[1], 64);
		pDataUnit->GetValue(friends[2], 64);
		pDataUnit->GetValue(friends[3], 64);
		pDataUnit->GetValue(friends[4], 64);
		pDataUnit->GetValue(friends[5], 64);
		pDataUnit->GetValue(friends[6], 64);
		pDataUnit->GetValue(friends[7], 64);
		pDataUnit->GetValue(friends[8], 64);
		pDataUnit->GetValue(friends[9], 64);
		pDataUnit->GetValue(friends[10], 64);
		pDataUnit->GetValue(friends[11], 64);
		pDataUnit->GetValue(friends[12], 64);
		pDataUnit->GetValue(friends[13], 64);
		pDataUnit->GetValue(friends[14], 64);
		pDataUnit->GetValue(friends[15], 64);
		pDataUnit->GetValue(friends[16], 64);
		pDataUnit->GetValue(friends[17], 64);
		pDataUnit->GetValue(friends[18], 64);
		pDataUnit->GetValue(friends[19], 64);
		pDataUnit->GetValue(friends[20], 64);
		pDataUnit->GetValue(friends[21], 64);
		pDataUnit->GetValue(friends[22], 64);
		pDataUnit->GetValue(friends[23], 64);
		pDataUnit->GetValue(friends[24], 64);
		pDataUnit->GetValue(friends[25], 64);
		pDataUnit->GetValue(friends[26], 64);
		pDataUnit->GetValue(friends[27], 64);
		pDataUnit->GetValue(friends[28], 64);
		pDataUnit->GetValue(friends[29], 64);
		pDataUnit->GetValue(friends[30], 64);
		pDataUnit->GetValue(friends[31], 64);
		for (int i = 0; i < 32; i++)
		{
			packet.push((LPVOID)friends[i], static_cast<int>(strlen(friends[i])));
			packet.push((LPVOID)"/", 1);
		}
		packet.push((LPVOID)"\0", 1);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_DB_NOMOREDATA)
	{
		strcpy(pszCommunity, "//////////////////////////////////////");
		iBufferSize = static_cast<int>(strlen(pszCommunity));
		return SE_OK;
	}
	iBufferSize = static_cast<int>(packet.getsize());
	return ret;
}

SERVER_ERROR CAppDB::DeleteMarriage(const char* pszName, const char* pszMarriage)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_COMMUNITY set marriage = '' where Name = '%s' and marriage = '%s'", pszName, pszMarriage);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::DeleteTeacher(const char* pszName, const char* pszTeacher)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_COMMUNITY set master = '' where Name = '%s' and master = '%s'", pszName, pszTeacher);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::DeleteStudent(const char* pszTeacher, const char* pszStudent)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SHORTNAME studentname;
	DWORD dwOwner;
	SERVER_ERROR ret = pDataUnit->Operation("select "
		"ownerid,"
		"student1,student2,student3 "
		"from TBL_CHARACTER_COMMUNITY where Name = '%s'", pszTeacher);
	if (ret == SE_OK)
	{
		pDataUnit->GetValue(dwOwner);
		for (int i = 0; i < 3; i++)
		{
			pDataUnit->GetValue(studentname, 64);
			if (strcmp(pszStudent, studentname) == 0)
			{
				m_pDBConnection->DelDataUnit(pDataUnit);
				pDataUnit = m_pDBConnection->GetDataUnit();
				if (pDataUnit)
					ret = pDataUnit->Operation("update TBL_CHARACTER_COMMUNITY set student%d = '' where ownerid = %d", i + 1, dwOwner);
				else
					return SE_ALLOCMEMORYFAIL;
				break;
			}
		}
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::BreakFriend(const char* friend1, const char* friend2)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SHORTNAME friendname;
	DWORD dwOwner;
	SERVER_ERROR ret = pDataUnit->Operation("select "
		"ownerid,"
		"friend1,"
		"friend2,"
		"friend3,"
		"friend4,"
		"friend5,"
		"friend6,"
		"friend7,"
		"friend8,"
		"friend9,"
		"friend10,"
		"friend11,"
		"friend12,"
		"friend13,"
		"friend14,"
		"friend15,"
		"friend16,"
		"friend17,"
		"friend18,"
		"friend19,"
		"friend20,"
		"friend21,"
		"friend22,"
		"friend23,"
		"friend24,"
		"friend25,"
		"friend26,"
		"friend27,"
		"friend28,"
		"friend29,"
		"friend30,"
		"friend31,"
		"friend32  "
		"from TBL_CHARACTER_COMMUNITY where Name = '%s'", friend1);
	if (ret == SE_OK)
	{
		pDataUnit->GetValue(dwOwner);
		for (int i = 0; i < 32; i++)
		{
			pDataUnit->GetValue(friendname, 64);
			if (strcmp(friend2, friendname) == 0)
			{
				m_pDBConnection->DelDataUnit(pDataUnit);
				pDataUnit = m_pDBConnection->GetDataUnit();
				if (pDataUnit)
					ret = pDataUnit->Operation("update TBL_CHARACTER_COMMUNITY set friend%d = '' where ownerid = %d", i + 1, dwOwner);
				else
					return SE_ALLOCMEMORYFAIL;
				break;
			}
		}
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

VOID CAppDB::DoInit()
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return;
	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_ITEM "
		"set DELFLAG = 1 "
		"where FLAG = %u", IDF_GROUND);
	m_pDBConnection->DelDataUnit(pDataUnit);
	if (ret == SE_OK)
		CServer::GetInstance()->GetIoConsole()->OutPut(SUCCESS_GREEN, "垃圾物品清理完毕!\n");
}

SERVER_ERROR CAppDB::RestoreGuild(const char* name, const char* guildname)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("update tbl_character_info set guildname = '%s' where name = '%s'", guildname, name);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::AddCredit(const char* pszName, UINT nCount)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select flag1 from tbl_character_info where name = '%s'", pszName);
	if (ret == SE_OK)
	{
		DWORD dwCredit;
		pDataUnit->GetValue(dwCredit);
		m_pDBConnection->DelDataUnit(pDataUnit);
		pDataUnit = m_pDBConnection->GetDataUnit();
		if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
		if (nCount > static_cast<WORD>(0xffff) - (dwCredit & 0xffff))
			dwCredit = (dwCredit & 0xffff0000) | 0xffff;
		else
			dwCredit = (dwCredit & 0xffff0000) | ((dwCredit & 0xffff) + nCount);
		ret = pDataUnit->Operation("update tbl_character_info set flag1 = %d", dwCredit);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateCharacterGuildName(const char* pszName, const char* pszGuildName)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("select GUILDNAME from tbl_character_info where name = '%s'", pszName);
	if (ret == SE_OK)
	{
		m_pDBConnection->DelDataUnit(pDataUnit);
		pDataUnit = m_pDBConnection->GetDataUnit();
		ret = pDataUnit->Operation("update tbl_character_info set GUILDNAME = '%s'", pszGuildName);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

static DWORD GetSqlRecordSize(EXECSQLRECORD* pRecord)
{
	DWORD dwSize = 0;
	for (UINT i = 0; i < pRecord->dwColCount; i++)
	{
		switch (pRecord->coldef[i].type)
		{
		case CT_STRING:			//	字符串
			dwSize += pRecord->coldef[i].length;
			break;
		case CT_TINYINT:		//	8位整数
			dwSize += sizeof(BYTE);
			break;
		case CT_SMALLINT:		//	16位整数
			dwSize += sizeof(WORD);
			break;
		case CT_INTEGER:		//	32位整数
			dwSize += sizeof(DWORD);
			break;
		case CT_BIGINT:			//	64位整数
			dwSize += sizeof(__int64);
			break;
		case CT_DATETIME:		//	时间
			dwSize += sizeof(SYSTEMTIME);
			break;
		case CT_CODEDARRAY:		//	编码存的数据		
			dwSize += pRecord->coldef[i].length;
			break;
		}
	}
	return dwSize;
}

SERVER_ERROR CAppDB::ExecSqlCommand(DWORD dwTransId, const char* pszCommand, EXECSQLRECORD* pRecordDef, xPacket& packet)
{
	char* szBuffer = g_szTempBuffer;
	packet.clear();
	packet.push(&dwTransId, sizeof(dwTransId));
	packet.push(sizeof(DWORD) * 2);

	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SERVER_ERROR ret = pDataUnit->Operation(pszCommand);
	DWORD dwCount = 0;
	DWORD* pdwParam = (DWORD*)packet.getbuf();
	BOOL bContinue = TRUE;
	if (ret == SE_OK && pRecordDef->dwColCount > 0 && pRecordDef->dwCount > 0)
	{
		do
		{
			for (UINT i = 0; i < pRecordDef->dwColCount; i++)
			{
				switch (pRecordDef->coldef[i].type)
				{
				case CT_STRING:		//	字符串
				{
					szBuffer[0] = 0;
					pDataUnit->GetValue(szBuffer, 65536);
					if (!packet.push((LPVOID)szBuffer, pRecordDef->coldef[i].length))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_TINYINT:		//	8位整数
				{
					BYTE	bt = 0;
					pDataUnit->GetValue(bt);
					if (!packet.push((LPVOID)&bt, sizeof(BYTE)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_SMALLINT:		///	16位整数
				{
					WORD w = 0;
					pDataUnit->GetValue(w);
					if (!packet.push((LPVOID)&w, sizeof(WORD)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_INTEGER:		//	32位整数
				{
					DWORD dw = 0;
					pDataUnit->GetValue(dw);
					if (!packet.push((LPVOID)&dw, sizeof(DWORD)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_BIGINT:			//	64位整数
				{
					__int64 i64 = 0;
					//pDataUnit->GetValue( dw );
					if (!packet.push((LPVOID)&i64, sizeof(__int64)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_DATETIME:		//	时间
				{
					szBuffer[0] = 0;
					SYSTEMTIME	st;
					pDataUnit->GetValue(szBuffer, 65536);
					GetTimeFromString(st, szBuffer);
					if (!packet.push((LPVOID)&st, sizeof(st)))
					{
						bContinue = FALSE;
						break;
					}
				}
				break;
				case CT_CODEDARRAY:		//	编码存的数据	
				{
					szBuffer[0] = 0;
					if (packet.getfreesize() < (int)pRecordDef->coldef[i].length)
					{
						bContinue = FALSE;
						break;
					}
					pDataUnit->GetValue(szBuffer, 65536);
					int limitsize = ((pRecordDef->coldef[i].length + 2) / 3) * 4;
					if (szBuffer[0] != 0)
					{
						szBuffer[limitsize] = 0;
						_UnGameCode(szBuffer, (BYTE*)packet.getfreebuf());
					}
					else
					{
						memset((void*)packet.getfreebuf(), 0, pRecordDef->coldef[i].length);
					}
					packet.addsize(pRecordDef->coldef[i].length);
				}
				break;
				}
			}
			if (!bContinue)
				break;
			dwCount++;
		} while (pDataUnit->MoveNext() == SE_OK && dwCount < pRecordDef->dwCount);
	}
	m_pDBConnection->DelDataUnit(pDataUnit);
	pdwParam[0] = ret;
	pdwParam[1] = dwCount;
	return ret;
}

SERVER_ERROR CAppDB::DeleteMagic(DWORD dwOwner, WORD wId)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
		return SE_ALLOCMEMORYFAIL;
	SERVER_ERROR ret = pDataUnit->Operation("DELETE FROM TBL_CHARACTER_MAGIC "
		//"set DELFLAG = 1 "
		"where CHARID = %d AND MAGICID = %d", dwOwner, wId);
	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

static constexpr int TASK_BINARY_SIZE = sizeof(TASKNODE) * MAX_TASK;
static void SerializeTasksBinary(TASKINFO* pInfo, BYTE* pBuffer, int bufferSize)
{
	memset(pBuffer, 0, bufferSize);
	int offset = 0;
	for (int i = 0; i < MAX_TASK; i++)
	{
		memcpy(pBuffer + offset, &pInfo->tasks[i].wTaskId, sizeof(WORD));
		offset += sizeof(WORD);
		memcpy(pBuffer + offset, &pInfo->tasks[i].wStep, sizeof(WORD));
		offset += sizeof(WORD);
		if (offset + sizeof(DWORD) * 10 <= bufferSize)
		{
			memcpy(pBuffer + offset, pInfo->tasks[i].vParam, sizeof(DWORD) * 10);
			offset += sizeof(DWORD) * 10;
		}
		if (offset + sizeof(char) * 10 * 32 <= bufferSize)
		{
			memcpy(pBuffer + offset, pInfo->tasks[i].sParam, sizeof(char) * 10 * 32);
			offset += sizeof(char) * 10 * 32;
		}
	}
}

static void DeserializeTasksBinary(const BYTE* pData, int dataSize, TASKINFO* pInfo)
{
	int offset = 0;
	for (int i = 0; i < MAX_TASK; i++)
	{
		memcpy(&pInfo->tasks[i].wTaskId, pData + offset, sizeof(WORD));
		offset += sizeof(WORD);
		memcpy(&pInfo->tasks[i].wStep, pData + offset, sizeof(WORD));
		offset += sizeof(WORD);
		if (offset + sizeof(DWORD) * 10 <= dataSize)
		{
			memcpy(pInfo->tasks[i].vParam, pData + offset, sizeof(DWORD) * 10);
			offset += sizeof(DWORD) * 10;
		}
		if (offset + sizeof(char) * 10 * 32 <= dataSize)
		{
			memcpy(pInfo->tasks[i].sParam, pData + offset, sizeof(char) * 10 * 32);
			offset += sizeof(char) * 10 * 32;
		}
	}
}

static thread_local BYTE g_szTasksBinary[TASK_BINARY_SIZE];
SERVER_ERROR CAppDB::QueryTaskInfo(DWORD dwOwner, TASKINFO* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SERVER_ERROR ret = pDataUnit->Operation("select Achievement,TotalTaskCount,Tasks "
		"FROM TBL_CHARACTER_TASK "
		"where CharId = %d", dwOwner);
	memset(pInfo, 0, sizeof(*pInfo));
	if (ret == SE_OK)
	{
		pDataUnit->GetValue(pInfo->dwAchievement);
		pDataUnit->GetValue(pInfo->dwTotalTaskCount);
		memset(g_szTasksBinary, 0, sizeof(g_szTasksBinary));
		int binarySize = sizeof(g_szTasksBinary);
		pDataUnit->GetData(3, SQL_C_BINARY, g_szTasksBinary, binarySize);
		DeserializeTasksBinary(g_szTasksBinary, binarySize, pInfo);
	}
	else
	{
		pInfo->dwAchievement = 0;
		pInfo->dwTotalTaskCount = 0;
		ret = InstertTaskInfo(dwOwner, pInfo);
	}
	return ret;
}

static const char HEX_TABLE[] = "0123456789ABCDEF";
SERVER_ERROR CAppDB::InstertTaskInfo(DWORD dwOwner, TASKINFO* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
		return SE_ALLOCMEMORYFAIL;
	CMySQLDBConnection* pMySQLConn = (CMySQLDBConnection*)m_pDBConnection;
	MYSQL* pMySQL = pMySQLConn->GetConnectionHandle();
	if (pMySQL == nullptr)
		return SE_FAIL;
	SerializeTasksBinary(pInfo, g_szTasksBinary, sizeof(g_szTasksBinary));
	std::string hexString;
	hexString.resize(TASK_BINARY_SIZE * 2);
	for (int i = 0; i < TASK_BINARY_SIZE; i++)
	{
		BYTE b = g_szTasksBinary[i];
		hexString[i * 2] = HEX_TABLE[b >> 4];
		hexString[i * 2 + 1] = HEX_TABLE[b & 0xF];
	}
	std::string sql = "insert into tbl_character_task(CharId,Achievement,TotalTaskCount,Tasks) values(" 
	                  + std::to_string(dwOwner) + ", " 
	                  + std::to_string(pInfo->dwAchievement) +", "
					  + std::to_string(pInfo->dwTotalTaskCount) + ", "
					  + "UNHEX('" + hexString + "'))";
	int result = mysql_query(pMySQL, sql.c_str());
	m_pDBConnection->DelDataUnit(pDataUnit);

	return (result == 0) ? SE_OK : SE_FAIL;
}

SERVER_ERROR CAppDB::UpdateTaskInfo(DWORD dwOwner, TASKINFO* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr)
		return SE_ALLOCMEMORYFAIL;
	CMySQLDBConnection* pMySQLConn = (CMySQLDBConnection*)m_pDBConnection;
	MYSQL* pMySQL = pMySQLConn->GetConnectionHandle();
	if (pMySQL == nullptr)
		return SE_FAIL;
	SerializeTasksBinary(pInfo, g_szTasksBinary, sizeof(g_szTasksBinary));
	std::string hexString;
	hexString.resize(TASK_BINARY_SIZE * 2);
	for (int i = 0; i < TASK_BINARY_SIZE; i++)
	{
		BYTE b = g_szTasksBinary[i];
		hexString[i * 2] = HEX_TABLE[b >> 4];
		hexString[i * 2 + 1] = HEX_TABLE[b & 0xF];
	}
	std::string sql = "update tbl_character_task set Achievement = " + std::to_string(pInfo->dwAchievement) 
						+ ", TotalTaskCount = " + std::to_string(pInfo->dwTotalTaskCount)
						+ ", Tasks = UNHEX('" + hexString + "') where CharId = " + std::to_string(dwOwner);
	int result = mysql_query(pMySQL, sql.c_str());
	SERVER_ERROR ret = (result == 0) ? SE_OK : SE_FAIL;

	m_pDBConnection->DelDataUnit(pDataUnit);

	if (ret != SE_OK)
		return InstertTaskInfo(dwOwner, pInfo);
	return ret;
}

static void SerializeFengHaoGrowsBinary(FenghaoInfo* pInfo, BYTE* pBuffer, int bufferSize)
{
	memset(pBuffer, 0, bufferSize);
	int offset = 0;
	for (int i = 0; i < MAX_FENGHAO; i++)
	{
		memcpy(pBuffer + offset, &pInfo->mFengHaoRow[i].boActivation, sizeof(BOOL));
		offset += sizeof(BOOL);
		memcpy(pBuffer + offset, &pInfo->mFengHaoRow[i].dwLastDate, sizeof(DWORD));
		offset += sizeof(DWORD);
	}
}

static void DeserializeFengHaoGrowsBinary(const BYTE* pData, int dataSize, FenghaoInfo* pInfo)
{
	int offset = 0;
	for (int i = 0; i < MAX_FENGHAO; i++)
	{
		memcpy(&pInfo->mFengHaoRow[i].boActivation, pData + offset, sizeof(BOOL));
		offset += sizeof(BOOL);
		memcpy(&pInfo->mFengHaoRow[i].dwLastDate, pData + offset, sizeof(DWORD));
		offset += sizeof(DWORD);
	}
}

static constexpr int FENGHAOGROW_BINARY_SIZE = sizeof(FengHaoRow) * MAX_FENGHAO;
static thread_local BYTE g_szFengHaoGrowsBinary[FENGHAOGROW_BINARY_SIZE];
SERVER_ERROR CAppDB::QueryFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SERVER_ERROR ret = pDataUnit->Operation("select Type1, Type2, Type3, FengHaoRow "
		"FROM TBL_CHARACTER_FENGHAO "
		"WHERE CharId = %u", dwOwner);

	if (ret == SE_OK)
	{
		pDataUnit->GetValue(pInfo->btType1);
		pDataUnit->GetValue(pInfo->btType2);
		pDataUnit->GetValue(pInfo->btType3);

		memset(g_szFengHaoGrowsBinary, 0, sizeof(g_szFengHaoGrowsBinary));
		int binarySize = sizeof(g_szFengHaoGrowsBinary);
		pDataUnit->GetData(4, SQL_C_BINARY, g_szFengHaoGrowsBinary, binarySize);
		DeserializeFengHaoGrowsBinary(g_szFengHaoGrowsBinary, binarySize, pInfo);
	}
	else if (ret == SE_DB_NOMOREDATA)
	{
		pInfo->btType1 = 0;
		pInfo->btType2 = 0;
		pInfo->btType3 = 0;
		ret = InstertFengHaoInfo(dwOwner, pInfo);
	}

	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::InstertFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SerializeFengHaoGrowsBinary(pInfo, g_szFengHaoGrowsBinary, sizeof(g_szFengHaoGrowsBinary));

	std::string hexString;
	hexString.resize(FENGHAOGROW_BINARY_SIZE * 2);
	for (int i = 0; i < FENGHAOGROW_BINARY_SIZE; i++)
	{
		BYTE b = g_szFengHaoGrowsBinary[i];
		hexString[i * 2] = HEX_TABLE[b >> 4];
		hexString[i * 2 + 1] = HEX_TABLE[b & 0xF];
	}

	SERVER_ERROR ret = pDataUnit->Operation("insert into TBL_CHARACTER_FENGHAO(CharId, Type1, Type2, Type3, FengHaoRow) values(%u, %u, %u, %u, UNHEX('%s'))",
		dwOwner, pInfo->btType1, pInfo->btType2, pInfo->btType3, hexString.c_str());

	m_pDBConnection->DelDataUnit(pDataUnit);
	return ret;
}

SERVER_ERROR CAppDB::UpdateFengHaoInfo(DWORD dwOwner, FenghaoInfo* pInfo)
{
	CVirtualDataUnit* pDataUnit = m_pDBConnection->GetDataUnit();
	if (pDataUnit == nullptr) return SE_ALLOCMEMORYFAIL;

	SerializeFengHaoGrowsBinary(pInfo, g_szFengHaoGrowsBinary, sizeof(g_szFengHaoGrowsBinary));

	std::string hexString;
	hexString.resize(FENGHAOGROW_BINARY_SIZE * 2);
	for (int i = 0; i < FENGHAOGROW_BINARY_SIZE; i++)
	{
		BYTE b = g_szFengHaoGrowsBinary[i];
		hexString[i * 2] = HEX_TABLE[b >> 4];
		hexString[i * 2 + 1] = HEX_TABLE[b & 0xF];
	}

	SERVER_ERROR ret = pDataUnit->Operation("update TBL_CHARACTER_FENGHAO set Type1 = %u, Type2 = %u, Type3 = %u, FengHaoRow = UNHEX('%s') where CharId = %u",
		pInfo->btType1, pInfo->btType2, pInfo->btType3, hexString.c_str(), dwOwner);

	m_pDBConnection->DelDataUnit(pDataUnit);

	if (ret != SE_OK)
		return InstertFengHaoInfo(dwOwner, pInfo);
	return ret;
}