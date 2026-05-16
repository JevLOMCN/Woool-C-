#include "StdAfx.h"
#include "mysqldatabase.h"
#include "server.h"
#include <cstring>

CMySQLDatabase::CMySQLDatabase(void)
{
	m_pMySQL = nullptr;
}

CMySQLDatabase::~CMySQLDatabase(void)
{
	UnInit();
}

SERVER_ERROR CMySQLDatabase::Init()
{
	m_pMySQL = mysql_init(nullptr);
	if (m_pMySQL == nullptr)
		return SE_ODBC_SQLALLOCHANDLEFAIL;

	// 设置字符集为 gb2312
	if (mysql_options(m_pMySQL, MYSQL_SET_CHARSET_NAME, "gb2312") != 0)
		return SE_ODBC_SQLSETENVATTRFAIL;

	return SE_OK;
}

SERVER_ERROR CMySQLDatabase::UnInit()
{
	if (m_pMySQL)
	{
		mysql_close(m_pMySQL);
		m_pMySQL = nullptr;
		return SE_OK;
	}
	return SE_DB_NOTINITED;
}

CVirtualDBConnection* CMySQLDatabase::GetConnection()
{
	CMySQLDBConnection* pConnection = nullptr;
	UINT id = m_DbConnections.New(&pConnection);
	if (id == 0 || pConnection == nullptr)
		return nullptr;
	pConnection->SetId(id);
	pConnection->SetDatabase(this);
	pConnection->Init(m_pMySQL);
	return pConnection;
}

SERVER_ERROR CMySQLDatabase::DelConnection(CVirtualDBConnection* pConnection)
{
	if (pConnection == nullptr)
		return SE_INVALIDPARAM;
	CMySQLDBConnection* pConnectionT = (CMySQLDBConnection*)pConnection;
	pConnectionT->UnInit();
	m_DbConnections.Del(pConnectionT->GetId());
	return SE_OK;
}

CMySQLDBConnection::CMySQLDBConnection()
{
	m_pMySQL = nullptr;
	m_pDatabase = nullptr;
	m_Id = 0;
	//m_pUnit = nullptr;
	IsstartDB = FALSE;
	memset(m_szServerName, 0, sizeof(m_szServerName));
	memset(m_szPort, 0, sizeof(m_szPort));
	memset(m_szDBName, 0, sizeof(m_szDBName));
	memset(m_szId, 0, sizeof(m_szId));
	memset(m_szPassword, 0, sizeof(m_szPassword));
}

CMySQLDBConnection::~CMySQLDBConnection()
{
	UnInit();
}

SERVER_ERROR CMySQLDBConnection::Init(void* pMySQL)
{
	// 注意：pMySQL 参数是从 CMySQLDatabase 传递过来的, 但它只是一个初始化模板
	// 每个连接需要自己独立的 MYSQL* 句柄（mysql_real_connect 绑定到具体句柄）
	// pMySQL 参数实际上没有实际作用, 保留是为了接口兼容性

	// 初始化 MySQL 连接句柄（为当前连接创建独立句柄）
	m_pMySQL = mysql_init(nullptr);
	if (m_pMySQL == nullptr)
		return SE_ODBC_SQLALLOCHANDLEFAIL;

	// 设置字符集（与 CMySQLDatabase::Init() 保持一致）
	if (mysql_options(m_pMySQL, MYSQL_SET_CHARSET_NAME, "gb2312") != 0)
		return SE_ODBC_SQLSETENVATTRFAIL;

	return SE_OK;
}

SERVER_ERROR CMySQLDBConnection::UnInit()
{
	if (m_pMySQL != nullptr)
	{
		mysql_close(m_pMySQL);
		m_pMySQL = nullptr;
	}

	// 清理所有 DataUnit, 防止内存泄漏
	// 注意：CIndexList 的析构函数会自动清理所有分配的对象
	// m_pUnit = nullptr;  // m_pUnit 由 m_DataUnits 管理, 不需要手动清零

	return SE_OK;
}

SERVER_ERROR CMySQLDBConnection::Connect(const char* pServerName, const char* pPort, const char* pdbname, const char* pId, const char* pPassword)
{
	// 保存连接参数用于重连
	strncpy_s(m_szServerName, pServerName, sizeof(m_szServerName) - 1);
	strncpy_s(m_szPort, pPort, sizeof(m_szPort) - 1);
	strncpy_s(m_szDBName, pdbname, sizeof(m_szDBName) - 1);
	strncpy_s(m_szId, pId, sizeof(m_szId) - 1);
	strncpy_s(m_szPassword, pPassword, sizeof(m_szPassword) - 1);

	UINT port = atoi(pPort);

	// 连接到 MySQL 服务器
	// 修复：移除 CLIENT_MULTI_STATEMENTS 标志, 避免 SQL 注入安全风险
	if (!mysql_real_connect(m_pMySQL, pServerName, pId, pPassword, pdbname, port, nullptr, 0))
	{
		return SE_ODBC_SQLCONNECTFAIL;
	}

	// 设置超时选项
	char sql[512];
	sprintf_s(sql, sizeof(sql), "SET SESSION wait_timeout=28800,interactive_timeout=28800,net_read_timeout=3600,net_write_timeout=3600");
	mysql_query(m_pMySQL, sql);

	// 移除 m_pUnit 的创建, DataUnit 应该在 GetDataUnit() 中动态创建
	IsstartDB = TRUE;
	return SE_OK;
}

SERVER_ERROR CMySQLDBConnection::Disconnect()
{
	if (m_pMySQL != nullptr)
	{
		mysql_close(m_pMySQL);
		m_pMySQL = nullptr;
	}
	IsstartDB = FALSE;

	return SE_OK;
}

// 检查连接是否有效
BOOL CMySQLDBConnection::IsConnectionValid()
{
	if (m_pMySQL == nullptr || !IsstartDB) return FALSE;

	// 执行 ping 来测试连接
	if (mysql_ping(m_pMySQL) != 0)
		return FALSE;

	return TRUE;
}

// 重连数据库
SERVER_ERROR CMySQLDBConnection::Reconnect()
{
	// 清理旧的连接句柄
	if (m_pMySQL != nullptr && IsstartDB)
	{
		mysql_close(m_pMySQL);
		m_pMySQL = nullptr;
		IsstartDB = FALSE;
	}

	// 重新初始化连接句柄
	m_pMySQL = mysql_init(nullptr);
	if (m_pMySQL == nullptr)
		return SE_ODBC_SQLALLOCHANDLEFAIL;

	// 设置字符集（与 Init() 保持一致）
	if (mysql_options(m_pMySQL, MYSQL_SET_CHARSET_NAME, "gb2312") != 0)
		return SE_ODBC_SQLSETENVATTRFAIL;

	// 使用保存的参数重新连接

	SERVER_ERROR ret = Connect(m_szServerName, m_szPort, m_szDBName, m_szId, m_szPassword);
	if (ret != SE_OK)
	{
		CServer* pServer = CServer::GetInstance();
		CIOConsole* pConsole = (CIOConsole*)pServer->GetIoConsole();
		pConsole->OutPut(ERROR_RED, "数据库重连失败!\n");
	}
	else
	{
		CServer* pServer = CServer::GetInstance();
		CIOConsole* pConsole = (CIOConsole*)pServer->GetIoConsole();
		pConsole->OutPut(SUCCESS_GREEN, "数据库重连成功!\n");
	}
	return ret;
}

CVirtualDataUnit* CMySQLDBConnection::GetDataUnit()
{
	// 在获取 DataUnit 前检查连接状态
	if (!IsConnectionValid())
	{
		CServer* pServer = CServer::GetInstance();
		CIOConsole* pConsole = (CIOConsole*)pServer->GetIoConsole();
		pConsole->OutPut(ERROR_RED, "GetDataUnit: 检测到连接断开, 尝试重连...\n");
		if (Reconnect() != SE_OK)
			return nullptr;
	}

	// 每次从 m_DataUnits 获取新的 DataUnit, 而不是复用 m_pUnit
	// 修复：原来的 m_pUnit 单例设计会导致多线程数据竞争
	CMySQLDataUnit* pUnit = nullptr;
	UINT id = m_DataUnits.New(&pUnit);
	if (id == 0 || pUnit == nullptr)
		return nullptr;

	pUnit->SetId(id);
	pUnit->SetDBConnection(this);
	pUnit->Init(m_pMySQL);
	return pUnit;
}

SERVER_ERROR CMySQLDBConnection::DelDataUnit(CVirtualDataUnit* pDataUnit)
{
	if (pDataUnit == nullptr)
		return SE_INVALIDPARAM;
	CMySQLDataUnit* pUnit = (CMySQLDataUnit*)pDataUnit;
	pUnit->UnInit();

	// 从 m_DataUnits 中删除 DataUnit, 释放资源
	UINT id = pUnit->GetId();
	if (id > 0)
		m_DataUnits.Del(id);

	return SE_OK;
}

CMySQLDataUnit::CMySQLDataUnit()
{
	m_Id = 0;
	m_pResult = nullptr;
	m_pCurrentRow = nullptr;
	//m_pAllRows = nullptr;
	m_nRowCount = 0;
	m_nCols = 0;
	m_nCurCols = 0;
	m_nCurrentRowIndex = -1;
	m_pConnection = nullptr;
}

CMySQLDataUnit::~CMySQLDataUnit()
{
	UnInit();
}

SERVER_ERROR CMySQLDataUnit::Init(void* pMySQL)
{
	if (m_pResult)
	{
		mysql_free_result(m_pResult);
		m_pResult = nullptr;
	}

	// 修复：移除 m_pAllRows 的清理（已不再使用双重内存分配）
	// if (m_pAllRows)
	// {
	//     delete[] m_pAllRows;
	//     m_pAllRows = nullptr;
	// }

	m_pCurrentRow = nullptr;
	m_nRowCount = 0;
	m_nCols = 0;
	m_nCurCols = 0;
	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::UnInit()
{
	if (m_pResult)
	{
		mysql_free_result(m_pResult);
		m_pResult = nullptr;
	}

	// 修复：移除 m_pAllRows 的清理（已不再使用双重内存分配）
	// if (m_pAllRows)
	// {
	//     delete[] m_pAllRows;
	//     m_pAllRows = nullptr;
	// }

	m_pCurrentRow = nullptr;
	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::Operation(const char* desc, ...)
{
	char szBuff[2048];
	va_list	vl;
	va_start(vl, desc);
	vsprintf_s(szBuff, sizeof(szBuff), desc, vl);
	va_end(vl);

	m_nCurCols = 0;

	CMySQLDBConnection* pConnection = (CMySQLDBConnection*)m_pConnection;
	if (pConnection == nullptr)
		return SE_FAIL;

	MYSQL* pMySQL = pConnection->GetConnectionHandle();
	if (pMySQL == nullptr)
		return SE_FAIL;

	// 清理之前的结果集
	if (m_pResult)
	{
		mysql_free_result(m_pResult);
		m_pResult = nullptr;
	}

	// 修复：移除 m_pAllRows 的清理（已不再使用双重内存分配）
	// if (m_pAllRows)
	// {
	//     delete[] m_pAllRows;
	//     m_pAllRows = nullptr;
	// }

	m_pCurrentRow = nullptr;

	// 执行查询
	int nResult = mysql_query(pMySQL, szBuff);

	// 检测连接错误
	if (nResult != 0)
	{
		unsigned int error_code = mysql_errno(pMySQL);
		CServer* pServer = CServer::GetInstance();
		CIOConsole* pConsole = (CIOConsole*)pServer->GetIoConsole();

		// 检查是否是连接断开错误 (2006 或 2013)
		if (error_code == 2006 || error_code == CR_SERVER_LOST)
		{
			pConsole->OutPut(ERROR_RED, "检测到 MySQL server has gone away, 尝试重连...\n");

			if (pConnection->Reconnect() == SE_OK)
			{
				pMySQL = pConnection->GetConnectionHandle();
				nResult = mysql_query(pMySQL, szBuff);
				if (nResult != 0)
				{
					pConsole->OutPut(ERROR_RED, "MySQL Error: %s\n", mysql_error(pMySQL));
					return SE_ODBC_SQLEXECDIRECTFAIL;
				}
			}
			else
			{
				return SE_ODBC_SQLEXECDIRECTFAIL;
			}
		}
		else
		{
			pConsole->OutPut(ERROR_RED, "MySQL Error: %s\n", mysql_error(pMySQL));
			return SE_ODBC_SQLEXECDIRECTFAIL;
		}
	}

	// 获取结果集
	m_pResult = mysql_store_result(pMySQL);
	if (m_pResult == nullptr)
	{
		// 如果没有结果集（如 INSERT, UPDATE, DELETE）, 返回成功
		if (mysql_field_count(pMySQL) == 0)
		{
			return SE_OK;
		}
		return SE_FAIL;
	}

	// 修复：处理可能的多语句结果（即使不使用 CLIENT_MULTI_STATEMENTS, 也添加保护）
	// 如果有更多结果集, 清理它们以避免后续查询错误
	while (mysql_next_result(pMySQL) == 0)
	{
		MYSQL_RES* pNextResult = mysql_store_result(pMySQL);
		if (pNextResult)
			mysql_free_result(pNextResult);
	}

	nResult = PrepareColAndRow();
	if (nResult != SE_OK)
		return SE_FAIL;
	if (m_nCols == 0)
		return SE_OK;
	return MoveNext();
}

SERVER_ERROR CMySQLDataUnit::GetData(int nCol, int type, LPVOID lpBuffer, int& size)
{
	if (m_pResult == nullptr || m_pCurrentRow == nullptr)
		return SE_DB_NOTINITED;

	int colIndex = nCol - 1;
	if (colIndex < 0 || colIndex >= m_nCols)
		return SE_FAIL;

	// 获取字段长度
	unsigned long* lengths = mysql_fetch_lengths(m_pResult);
	if (lengths == nullptr)
		return SE_FAIL;

	if (size == 0)
	{
		// 普通变量, 只返回所需大小
		if (m_pCurrentRow[colIndex] == nullptr)
			return SE_FAIL;

		switch (type)
		{
		case SQL_C_CHAR:
			size = (int)lengths[colIndex] + 1;
			break;
		case SQL_INTEGER:
			size = sizeof(int);
			break;
		case SQL_SMALLINT:
			size = sizeof(short);
			break;
		case SQL_TINYINT:
			size = sizeof(BYTE);
			break;
		case SQL_C_BINARY:
			size = (int)lengths[colIndex];
			break;
		default:
			return SE_FAIL;
		}
	}

	// 检查缓冲区大小和列号的有效性
	if (lpBuffer == nullptr || size < 1) return SE_FAIL;

	if (m_pCurrentRow[colIndex] == nullptr)
	{
		memset(lpBuffer, 0, size);
		return SE_OK;
	}

	switch (type)
	{
	case SQL_C_CHAR:
		strncpy_s((char*)lpBuffer, size, m_pCurrentRow[colIndex], _TRUNCATE);
		size = (int)lengths[colIndex];
		break;
	case SQL_INTEGER:
		if (sizeof(int) <= size)
			*(int*)lpBuffer = atoi(m_pCurrentRow[colIndex]);
		size = sizeof(int);
		break;
	case SQL_SMALLINT:
		if (sizeof(short) <= size)
			*(short*)lpBuffer = (short)atoi(m_pCurrentRow[colIndex]);
		size = sizeof(short);
		break;
	case SQL_TINYINT:
		if (sizeof(BYTE) <= size)
			*(BYTE*)lpBuffer = (BYTE)atoi(m_pCurrentRow[colIndex]);
		size = sizeof(BYTE);
		break;
	case SQL_C_BINARY:
		if (size >= (int)lengths[colIndex])
			memcpy(lpBuffer, m_pCurrentRow[colIndex], lengths[colIndex]);
		size = (int)lengths[colIndex];
		break;
	default:
		return SE_FAIL;
	}
	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::PrepareColAndRow()
{
	if (m_pResult == nullptr)
		return SE_FAIL;

	m_nCols = mysql_num_fields(m_pResult);
	if (m_nCols == 0)
	{
		m_nRowCount = 0;
		return SE_OK;
	}

	m_nRowCount = (int)mysql_num_rows(m_pResult);

	// 修复：移除双重内存消耗
	// 原来的代码：mysql_store_result() 已经将所有数据加载到内存
	// 又额外分配 m_pAllRows 数组缓存行指针, 导致双倍内存消耗
	// 现在直接使用 mysql_store_result() 的内部数据, 不再额外分配 m_pAllRows
	// 注意：MoveNext() 现在需要使用 mysql_fetch_row() 而不是 m_pAllRows

	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::MoveNext()
{
	if (m_pResult == nullptr)
		return SE_DB_NOTINITED;

	// 移动到下一行
	// 修复：不再使用 m_pAllRows（已移除双重内存分配）, 直接使用 mysql_fetch_row()
	m_pCurrentRow = mysql_fetch_row(m_pResult);
	if (m_pCurrentRow == nullptr)
		return SE_DB_NOMOREDATA;

	m_nCurCols = 0;
	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::GetColDesc(int nCol, ColumnInfo* pInfo)
{
	if (nCol < 0 || nCol >= m_nCols || m_pResult == nullptr)
		return SE_FAIL;

	MYSQL_FIELD* field = mysql_fetch_field_direct(m_pResult, nCol);
	if (field == nullptr)
		return SE_FAIL;

	strncpy_s(pInfo->szColName, field->name, _TRUNCATE);
	pInfo->nColNameSize = (int)strlen(pInfo->szColName);
	pInfo->nColType = field->type;
	pInfo->nColSize = field->length;
	pInfo->nAllowDecimalDigit = field->decimals;
	pInfo->nAllowNull = (field->flags & NOT_NULL_FLAG) ? 0 : 1;

	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::UpdateImage(const char* pszTableName, const char* pszColName, const char* pszCondition, LPVOID lpData, int datasize)
{
	// 使用转义字符串的方式更新 BLOB 数据
	CMySQLDBConnection* pConnection = (CMySQLDBConnection*)m_pConnection;
	if (pConnection == nullptr)
		return SE_FAIL;

	MYSQL* pMySQL = pConnection->GetConnectionHandle();
	if (pMySQL == nullptr)
		return SE_FAIL;

	// 修复：使用 vector 替代裸指针, 确保异常安全
	std::vector<char> escapedData(datasize * 2 + 1);
	mysql_real_escape_string(pMySQL, escapedData.data(), (char*)lpData, datasize);

	// 构建 SQL（使用 vector 避免内存泄漏）
	size_t sqlSize = strlen(pszTableName) + strlen(pszColName) + strlen(pszCondition) + datasize * 2 + 100;
	std::vector<char> szSQLStr(sqlSize);
	sprintf_s(szSQLStr.data(), sqlSize,
		"UPDATE %s SET %s = '%s' WHERE %s", pszTableName, pszColName, escapedData.data(), pszCondition);

	// 执行更新
	int result = mysql_query(pMySQL, szSQLStr.data());

	if (result != 0)
		return SE_FAIL;

	return SE_OK;
}

SERVER_ERROR CMySQLDataUnit::GetValue(DWORD& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_INTEGER, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(WORD& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_SMALLINT, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(BYTE& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_TINYINT, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(int& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_INTEGER, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(short& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_SMALLINT, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(char& val)
{
	int size = 0;
	return GetData(++m_nCurCols, SQL_TINYINT, &val, size);
}

SERVER_ERROR CMySQLDataUnit::GetValue(char* pszString, int size)
{
	if (pszString == nullptr || size < 1) return SE_FAIL;
	SERVER_ERROR sr = GetData(++m_nCurCols, SQL_C_CHAR, (LPVOID)pszString, size);
	return sr;
}