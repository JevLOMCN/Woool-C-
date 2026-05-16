#include "StdAfx.h"
#include ".\server.h"

CServer* CServer::m_pInstance = nullptr;
CServer::CServer(void)
{
	m_pInstance = this;
	m_bDisableRegister = FALSE;
	m_pszServerTips = nullptr;
	m_pszLoginOkTips = nullptr;
	m_pszRegisterTips = nullptr;
	m_pszWAddr = nullptr;
	m_nWPort = 0;
}

CServer::~CServer(void) 
{
}

CClientObject* CServer::NewClientObject()
{
	return newObject();
}

VOID CServer::DeleteClientObject(CClientObject* pObject)
{
	deleteObject((CClientObj*)pObject);
}

BOOL CServer::InitServer(CSettingFile& s)
{
	int	t = s.GetInteger(m_pServerName, "DisableRegister");
	if (t == 0)
	{
		CBaseServer::GetIoConsole()->OutPut(SUCCESS_GREEN, "允许注册新帐号!\n");
		m_bDisableRegister = FALSE;
	}
	else
	{
		CBaseServer::GetIoConsole()->OutPut(ERROR_RED, "禁止注册新帐号!\n");
		m_bDisableRegister = TRUE;
	}

	m_pszWAddr = _strdup(s.GetString(m_pServerName, "WAddr", "127.0.0.1"));
	m_nWPort = s.GetInteger(m_pServerName, "WPort", 7501);
	int maxconnection = s.GetInteger(m_pServerName, "MaxConnection", MAX_CLIENTOBJECT);
	create(maxconnection);
	CBaseServer::GetIoConsole()->OutPut(SUCCESS_GREEN, "最大连接数 %d!\n", maxconnection);

	m_pszServerTips = (char*)LoadFile(".\\Data\\Config\\UserTip.txt");		//读取Data下的目录
	m_pszLoginOkTips = (char*)LoadFile(".\\Data\\Config\\LoginTip.txt");	//读取Data下的目录
	m_pszRegisterTips = (char*)LoadFile(".\\Data\\Config\\RegisterTip.txt");//读取Data下的目录
	return TRUE;
}

VOID CServer::CleanServer()
{
	if (m_pszServerTips)
	{
		delete[] m_pszServerTips;
		m_pszServerTips = nullptr;
	}
	if (m_pszLoginOkTips)
	{
		delete[] m_pszLoginOkTips;
		m_pszLoginOkTips = nullptr;
	}
	if (m_pszRegisterTips)
	{
		delete[] m_pszRegisterTips;
		m_pszRegisterTips = nullptr;
	}
	if (m_pszWAddr)
	{
		free(m_pszWAddr);
		m_pszWAddr = nullptr;
	}
	m_bDisableRegister = FALSE;
	m_nWPort = 0;
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
	UpdateDBServer();
	//获取对象池 m_ObjectPool 中的第一个对象, 并将其赋值给指针 pObject.
	//进入循环, 只要 pObject 不为空, 就执行以下操作：
	//调用 pObject->Update() 来更新当前对象.
	//获取对象池中的下一个对象, 并将其赋值给 pObject.
	//循环结束后, 调用 UpdateSCServer() 函数来更新 SC 服务器.
	//调用 UpdateDBServer() 函数来更新数据库服务器.
}
