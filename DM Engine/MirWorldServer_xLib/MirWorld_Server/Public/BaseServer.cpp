#include "StdAfx.h"
#include "baseserver.h"
#include "tempclient.h"
#include "LogFile.h"

CBaseServer::CBaseServer(void) :m_xQTempClient(2048)
{
	m_szNickName[0] = 0;
	m_szServerState[0] = 0;
	m_pServerName = nullptr;
	m_Type = ST_UNKNOWN;
	m_Ident.dwId = 0;
	m_iDBServerCount = 0;
	GetLocalTime(&m_stStartTime);
	m_bStateInfoUpdated = FALSE;
	m_nLoopCount = 0;
	m_fLoopTime = 0.0f;
	m_dwLoopTimes = 0;
	m_dwLoopStartTime = 0;
	m_dwConnectedTimes = 0;
	m_dwDisconnectTimes = 0;
	m_dwPreConnections = 0;
	m_xIocpServer.setEventListener(this);
	this->m_pIoConsole = nullptr;
	srand(timeGetTime());
	this->m_szBanIpFile[0] = 0;
	this->m_szTrustIpFile[0] = 0;
}

CBaseServer::~CBaseServer(void)
{
}

BOOL CBaseServer::Start(CSettingFile& sf) //读取Config.ini
{
	//获取服务器类型
	m_Ident.bType = (BYTE)GetServerTypeByName(sf.GetString(m_pServerName, "Type"));
	if (m_Ident.bType == ST_UNKNOWN)
	{
		m_pIoConsole->OutPut(ERROR_RED, "得到一个未知服务器类型, 请检查配置文件!\n");
		return FALSE;
	}
	//获取group和id
	m_Ident.bGroup = (BYTE)sf.GetInteger(m_pServerName, "Group");
	m_Ident.bId = (BYTE)sf.GetInteger(m_pServerName, "Id");
	//获取名字
	SetNickName(sf.GetString(m_pServerName, "Name", "NONAME"));
	//获取servercenter的地址
	char szIpAddr[32];
	m_SCAddr.nPort = (UINT)sf.GetInteger("Public", "ScPort", 0);
	if (!GetAddrByName(sf.GetString("Public", "ScAddr", "127.0.0.1"), szIpAddr))
		strcpy(szIpAddr, "127.0.0.1");
	m_SCAddr.SetAddr(szIpAddr);
	//取得服务器的地址和端口
	m_ServerAddr.nPort = (UINT)sf.GetInteger(m_pServerName, "Port", 0);
	if (!GetAddrByName(sf.GetString(m_pServerName, "Addr", "127.0.0.1"), szIpAddr))
		strcpy(szIpAddr, "127.0.0.1");
	m_ServerAddr.SetAddr(szIpAddr);
	m_pIoConsole->OutPut(SUCCESS_GREEN, "正在启动服务器...\n");
	//加载IP限制列表
	char* pszIpFile = (char*)sf.GetString(m_pServerName, "BanIpList", ".\\BanIP.txt");
	o_strncpy(m_szBanIpFile, pszIpFile, 250);
	this->m_BanList.Load(pszIpFile);
	//加载IP信任列表
	pszIpFile = (char*)sf.GetString(m_pServerName, "TrustIpList", ".\\TrustIP.txt");
	o_strncpy(m_szTrustIpFile, pszIpFile, 250);
	this->m_TrustList.Load(pszIpFile);
	//启动服务器
	BOOL bRet = FALSE;
	bRet = m_xIocpServer.start();
	if (bRet)
	{
		bRet = m_xIocpServer.postListen("0.0.0.0", m_ServerAddr.nPort, 64, 100000);
		if (bRet)
		{
			if (!InitServer(sf))
				bRet = FALSE;
			m_dwLoopStartTime = timeGetTime();
			if (!xThread::Start(this))
				bRet = FALSE;
		}
	}
	else
		m_pIoConsole->OutPut(ERROR_RED, m_xIocpServer.getErrorMsg());
	if (bRet)
		m_pIoConsole->OutPut(SUCCESS_GREEN, "启动服务器成功...\n");
	else
		m_pIoConsole->OutPut(ERROR_RED, "(%d) %s\n", m_xIocpServer.getErrorCode(), m_xIocpServer.getErrorMsg());
	return bRet;
}

BOOL CBaseServer::GetAddrByName(const char* pszHostName, char* pszIpAddress)
{
	LPHOSTENT m_pHost = gethostbyname(pszHostName);
	if (m_pHost == nullptr)return FALSE;
	ULONG lAddr = *(ULONG*)m_pHost->h_addr_list[0];
	sprintf(pszIpAddress, "%u.%u.%u.%u", lAddr & 0xff, (lAddr & 0xff00) >> 8, (lAddr & 0xff0000) >> 16, (lAddr & 0xff000000) >> 24);
	return TRUE;
}

VOID CBaseServer::GetServerState(SERVERSTATE& state)
{
	state.bUpdated = m_bStateInfoUpdated;
	m_bStateInfoUpdated = FALSE;
	SYSTEMTIME stNow;
	GetLocalTime(&stNow);
	state.dwRunningSeconds = GetT1toT2Second(m_stStartTime, stNow);
	state.bUpdated = m_bStateInfoUpdated;
	state.dwRecvBytes = m_xIocpServer.getRecvKBytes();
	state.dwSendBytes = m_xIocpServer.getSendKBytes();
	state.fLoopTime = m_fLoopTime;
	state.numConnected = (int)m_dwConnectedTimes;
	state.numDisconnected = (int)m_dwDisconnectTimes;
	state.numConnection = m_dwConnectedTimes - m_dwDisconnectTimes;
	state.dwLoopTimes = m_dwLoopTimes;
	sprintf(m_szServerState, "%d天%d小时%d分%d秒",
		state.dwRunningSeconds / 86400, (state.dwRunningSeconds / 3600) % 24, (state.dwRunningSeconds / 60) % 60,
		state.dwRunningSeconds % 60);
	state.pServerDescript = m_szServerState;
}

BOOL CBaseServer::Stop()
{
	xThread::Terminate();// 通知线程终止
	xThread::WaitFor();// 等待线程结束（线程内不再访问网络资源）
	m_DisconnectQueue.Clean();// 清理队列
	//停止网络
	m_SCClientObj.close(); // 先关闭客户端连接
	m_xIocpServer.stop();// 再停止IOCP服务器（释放网络资源）
	CleanServer(); // 清理服务器数据（此时网络已完全关闭）
	return TRUE;
}

VOID CBaseServer::Execute(LPVOID lpParam)
{
	while (!IsTerminated())
	{
		const DWORD dwUpdateKey = m_nLoopCount;
		if (++m_nLoopCount >= UPDATE_LOOP) m_nLoopCount = 0; // 循环计数递增
		Update(); //主循环
		if ((dwUpdateKey & 1) == 0)// 偶数帧：处理客户端消息
		{ 
			m_xIocpServer.update();
			UpdateTotalInfo();
		}
		else
		{
			if (m_xIocpServer.hasPendingEvents())
				SwitchToThread();
			else
				Sleep(1);
			ProcConnectionEvent(); //处理连接和断开连接的事件
		}
	}
}

BOOL CBaseServer::ConnectSCServer()
{
	if (m_SCClientObj.IsConnected())
		m_SCClientObj.close();
	m_SCClientObj.Clean();
	m_SCClientObj.setServer(&m_xIocpServer);
	m_SCClientObj.SetMsgProcessor(this);
	if (m_xIocpServer.postConnection(m_SCAddr.addr, m_SCAddr.nPort, m_SCClientObj))//OpenConnection(m_SCClientObj,m_SCAddr.addr, m_SCAddr.nPort );
	{
		m_SCClientObj.postRecv();
		return TRUE;
	}
	return FALSE;
}

BOOL CBaseServer::ConnectDBServer()
{
	for (int i = 0; i < m_iDBServerCount; i++)
	{
		if (m_DBClientObjs[i].IsConnected())
			continue;
		m_DBClientObjs[i].setServer(&m_xIocpServer);
		m_DBClientObjs[i].SetMsgProcessor(this);
		if (!m_xIocpServer.postConnection(m_DBAddrs[i].addr, m_DBAddrs[i].nPort, m_DBClientObjs[i]))
			return FALSE;
		m_DBClientObjs[i].postRecv();
		OnDBServerReady(i);
	}
	return TRUE;
}

VOID CBaseServer::UpdateSCServer()
{
	PLMIRMSG pmsg = nullptr;
	if (m_dwPreConnections != (m_dwConnectedTimes - m_dwDisconnectTimes))
	{
		m_dwPreConnections = (m_dwConnectedTimes - m_dwDisconnectTimes);
		m_SCClientObj.UpdateInfo(m_dwPreConnections);
	}
	if (m_SCClientObj.IsConnected())
	{
		m_SCClientObj.Update();
	}
	else
	{
		if (!ConnectSCServer()) //重新连接
			m_pIoConsole->OutPut(ERROR_RED, "连接服务器中心(%s:%u)失败...\n",
				m_SCAddr.addr, m_SCAddr.nPort);
		else
		{
			m_SCClientObj.setId(ID_SERVERCENTERCONNECTION);
			m_SCClientObj.RegisterServer(m_ServerAddr, m_Ident, m_szNickName);
			OnSCServerReady();
		}
	}
}

VOID CBaseServer::UpdateDBServer()
{
	for (int i = 0; i < m_iDBServerCount; i++)
	{
		if (m_DBClientObjs[i].IsConnected())
			m_DBClientObjs[i].Update();
		else
		{
			if (!ConnectDBServer()) //重新连接
				m_pIoConsole->OutPut(ERROR_RED, "连接数据库服务器失败...\n");
			else
				m_pIoConsole->OutPut(SUCCESS_GREEN, "连接数据库服务器成功...\n");
		}
	}
}

VOID CBaseServer::ProcConnectionEvent()
{
	CTempClient* pTClient;
	int count = static_cast<int>(m_xQTempClient.getcount());
	if (count == 0) return;
	for (int i = 0; i < count; i++)
	{
		pTClient = m_xQTempClient.pop();
		if (pTClient->IsTimeOut(3000))
		{
			pTClient->close();
			m_xTempClients.deleteObject(pTClient);
			continue;
		}
		m_xQTempClient.push(pTClient);
	}
}

VOID CBaseServer::OnSCMsg(PMIRMSG pMsg, int datasize)
{
	switch (pMsg->wCmd)
	{
	case SCM_MSGACROSSSERVER:
	{
		OnMASMsg(pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
	}
	break;
	case SCM_REGISTERSERVER:
	{
		if (pMsg->wParam[0] == SE_OK)
		{
			REGISTER_SERVER_RESULT* presult = (REGISTER_SERVER_RESULT*)pMsg->data;
			m_pIoConsole->OutPut(SUCCESS_GREEN, "注册服务器成功,index = %d\n",
				presult->Id.bIndex);
			m_Ident = presult->Id;
			m_iDBServerCount = presult->nDbCount;
			if (m_iDBServerCount > 2)m_iDBServerCount = 2;
			for (int i = 0; i < m_iDBServerCount; i++)
			{
				m_DBAddrs[i] = presult->DbAddr[i];
				m_pIoConsole->OutPut(STRING_GREEN, "数据库服务器 %s:%d\n",
					m_DBAddrs[i].addr, m_DBAddrs[i].nPort);
			}
		}
		else
		{
			m_pIoConsole->OutPut(ERROR_RED, "注册服务器失败,错误 = %d\n",
				pMsg->wParam[0]);
		}
	}
	break;
	}
}

VOID CBaseServer::Update()
{
}

VOID CBaseServer::UpdateTotalInfo()
{
	m_dwLoopTimes++;
	if (m_dwLoopTimes >= 5000)
	{
		DWORD dwCurTime = timeGetTime();
		m_dwLoopTimes = 0;
		m_fLoopTime = (dwCurTime - m_dwLoopStartTime) / 1000.0f;
		m_dwLoopStartTime = dwCurTime;
		m_bStateInfoUpdated = TRUE;
	}
}

VOID CBaseServer::OnUnknownMsg(PMIRMSG pMsg, int datasize, int type)
{
	CIOConsole* pConsole = (CIOConsole*)GetIoConsole();
	DWORD dwColor = 0;
	switch (type)
	{
	case 0:
	{
		dwColor = ERROR_RED;
		LG1("未知消息 Ident:%#x\n", pMsg->wCmd);
		pConsole->OutPut(dwColor, "未知消息 Ident:%#x\n", pMsg->wCmd);
	}
	break;
	case 1:
	{
		dwColor = ORANGE;
		LG1("接收消息 Ident:%#x\n", pMsg->wCmd);
		pConsole->OutPut(dwColor, "接收消息 Ident:%#x\n", pMsg->wCmd);
	}
	break;
	case 2:
	{
		dwColor = WARN_YELLOW;
		LG1("发送消息 Ident:%#x\n", pMsg->wCmd);
		pConsole->OutPut(dwColor, "发送消息 Ident:%#x\n", pMsg->wCmd);
	}
	break;
	}
	LG1("F:%d I:%d P1:%d P2:%d P3:%d D:%s S:%d\n", pMsg->dwFlag, pMsg->wCmd, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
	LG1("[");
	for (int i = 0; i < datasize; i++)
	{
		LG1("%d,", (BYTE)((*(pMsg->data + i)) & 0xff));
	}
	LG1("]\n\n");
	pConsole->OutPut(dwColor, "F:%d I:%d P1:%d P2:%d P3:%d D:%s S:%d\n\n", pMsg->dwFlag, pMsg->wCmd, pMsg->wParam[0], pMsg->wParam[1], pMsg->wParam[2], pMsg->data, datasize);
}

VOID CBaseServer::OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam)
{
	switch (iEvent)
	{
	case xIocpServer::ISE_ONCONNECTION:
	{
		xTempClient* pTempClient = (xTempClient*)lpParam;

		if (!(m_TrustList.IsEmpty() || m_TrustList.AddressIn(pTempClient->getAddress())))
		{
			m_pIoConsole->OutPut(ERROR_RED, "非信任IP %s 连接被阻止...\n", pTempClient->getAddress());
		}
		else if (!m_BanList.IsEmpty() && m_BanList.AddressIn(pTempClient->getAddress()))
		{
			m_pIoConsole->OutPut(ERROR_RED, "被禁止IP %s 连接被阻止...\n", pTempClient->getAddress());
		}
		else
		{
			CClientObject* pObj = NewClientObject();
			if (pObj != nullptr)
			{
				pObj->setServer(&m_xIocpServer);
				if (!pObj->steelSocket(*(xSocket*)lpParam))
				{
					DeleteClientObject(pObj);
				}
				else
				{
					pObj->OnConnection();
					m_pIoConsole->OutPut(STRING_GREEN, "有连接来自( %s )...\n", pObj->getAddress());
					m_dwConnectedTimes++;
					pObj->postRecv();
				}
			}
		}
	}
	break;
	case xIocpServer::ISE_ONDISCONNECTION:
	{
		CClientObject* pObj = (CClientObject*)lpParam;
		UINT id = pObj->getId();
		pObj->OnDisconnect();
		CClientObject* pObject = GetClientObject(id);
		if (pObject == nullptr)
		{
			if (id == ID_SERVERCENTERCONNECTION)
			{
				m_pIoConsole->OutPut(ERROR_RED, "与服务器中心失去连接!\n");
			}
		}
		else if (pObject == pObj)
		{
			m_pIoConsole->OutPut(ERROR_RED, "来自( %s )的连接断开...\n", pObject->getAddress());
			DeleteClientObject(pObject);
			m_dwDisconnectTimes++;
		}
		else
		{
			m_pIoConsole->OutPut(ERROR_RED, "致命错误 ( %n )!\n", id);
		}
	}
	break;
	}
}

VOID CBaseServer::OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size)
{
	//++++
}

VOID CBaseServer::OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize)
{
	if (pObject == &this->m_SCClientObj)
	{
		if (pMsg->dwFlag == 0)
			OnSCMsg(pMsg, datasize);
		else
		{
			CClientObject* pObject = (CClientObject*)GetClientObject(pMsg->dwFlag);
			if (pObject != nullptr)
				pObject->OnSCMsg_T(pMsg, datasize);
		}
	}
	else if (pObject == &this->m_DBClientObjs[0] || pObject == &this->m_DBClientObjs[1])
	{
		if (pMsg->dwFlag == 0)
			OnDBMsg(pMsg, datasize);
		else
		{
			CClientObject* pObject = GetClientObject(pMsg->dwFlag);
			if (pObject != nullptr)
				pObject->OnDBMsg(pMsg, datasize);
		}
	}
}

VOID CBaseServer::OnInput(const char* pszString)
{
	char szLine[256];
	o_strncpy(szLine, pszString, 250);
	xStringsExtracter<16> cmd(szLine, " \t,", " \t");
	if (static_cast<int>(cmd.getCount()) > 0)
	{
		if (_stricmp(cmd[0], "reloadbanlist") == 0)
		{
			this->m_BanList.Load(this->m_szBanIpFile);
			this->GetIoConsole()->OutPut(SUCCESS_GREEN, "Ban列表 %s 已加载!\n", m_szBanIpFile);
		}
		else if (_stricmp(cmd[0], "reloadtrustlist") == 0)
		{
			this->m_TrustList.Load(this->m_szTrustIpFile);
			this->GetIoConsole()->OutPut(SUCCESS_GREEN, "Trust列表 %s 已加载!\n", m_szTrustIpFile);
		}
		else if (_stricmp(cmd[0], "exit") == 0 || _stricmp(cmd[0], "quit") == 0)
		{
			this->GetIoConsole()->OutPut(SUCCESS_GREEN, "收到关闭命令, 开始优雅关闭服务器...\n");
			this->Stop();
		}
	}
}
