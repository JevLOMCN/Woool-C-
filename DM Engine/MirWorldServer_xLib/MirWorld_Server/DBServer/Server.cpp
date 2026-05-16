#include "StdAfx.h"
#include ".\server.h"
#include "time.h"

CServer* CServer::m_pInstance = nullptr;
CServer::CServer(void)
{
	m_pInstance = this;
	memset(m_szServer, 0, sizeof(m_szServer));
	memset(m_szPort, 0, sizeof(m_szPort));
	memset(m_szDatabase, 0, sizeof(m_szDatabase));
	memset(m_szAccount, 0, sizeof(m_szAccount));
	memset(m_szPassword, 0, sizeof(m_szPassword));
	memset(m_dwMsgTimes, 0, sizeof(m_dwMsgTimes));
}

CServer::~CServer(void)
{
}

CClientObject* CServer::NewClientObject()
{
	CClientObj* pObj = newObject();
	if (pObj->GetAppDB().OpenDataBase(m_szServer, m_szPort, m_szDatabase, m_szAccount, m_szPassword) != SE_OK)
	{
		m_pIoConsole->OutPut(ERROR_RED, "无法连接数据库, 请检查服务器是否开启!\n");
		deleteObject(pObj);
		return nullptr;
	}
	return pObj;
}

VOID CServer::DeleteClientObject(CClientObject* pObject)
{
	deleteObject((CClientObj*)pObject);
}

BOOL CServer::InitServer(CSettingFile& s)
{
	int maxconnection = s.GetInteger(m_pServerName, "MaxConnection", MAX_CLIENTOBJECT);
	create(maxconnection);
	CBaseServer::GetIoConsole()->OutPut(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);

	std::random_device rd;
	std::mt19937 gen(rd());

	char* pszServer = (char*)s.GetString(m_pServerName, "DbServer", "localhost");
	char* pszPort = (char*)s.GetString(m_pServerName, "DbPort", "3306");
	char* pszDatabase = (char*)s.GetString(m_pServerName, "DbName", "mirworlddb");
	char* szAccount = (char*)s.GetString(m_pServerName, "DbAccount", "root");
	char* szPassword = (char*)s.GetString(m_pServerName, "DbPassword", "root");

	strncpy(m_szServer, pszServer, 63);
	strncpy(m_szPort, pszPort, 63);
	strncpy(m_szDatabase, pszDatabase, 63);
	strncpy(m_szAccount, szAccount, 63);
	strncpy(m_szPassword, szPassword, 63);

	if (m_appDB.OpenDataBase(pszServer, pszPort, pszDatabase, szAccount, szPassword) != SE_OK)
	{
		m_pIoConsole->OutPut(ERROR_RED, "无法连接数据库, 请检查服务器是否开启!\n");
		return FALSE;
	}
	else
	{
		m_pIoConsole->OutPut(SUCCESS_GREEN, "连接数据库成功!\n");
	}
	m_appDB.DoInit();
	m_xxShowTimer.Savetime();
	return TRUE;
}

VOID CServer::CleanServer()
{
}

VOID CServer::Update()
{
	CClientObj* pObject = m_ObjectPool.First();
	while (pObject)
	{
		pObject->Update();
		pObject = m_ObjectPool.Next();
	}
	UpdateSCServer();
	//获取对象池 m_ObjectPool 中的第一个对象, 并将其赋值给指针 pObject.
	//进入循环, 只要 pObject 不为空, 就执行以下操作：
	//调用 pObject->Update() 来更新当前对象.
	//获取对象池中的下一个对象, 并将其赋值给 pObject.
	//循环结束后, 调用 UpdateSCServer() 函数来更新 SC 服务器.
	//调用 UpdateDBServer() 函数来更新数据库服务器.
}

VOID CServer::OnParseMsg(WORD wMsg)
{
	if (wMsg >= DM_START && wMsg < DM_END)
		this->m_dwMsgTimes[wMsg]++;
}
