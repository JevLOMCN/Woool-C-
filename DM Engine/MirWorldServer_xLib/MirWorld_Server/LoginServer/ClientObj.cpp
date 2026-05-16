#include "StdAfx.h"
#include <ws2tcpip.h>
#include ".\clientobj.h"
#include "server.h"

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
	m_szAccount[0] = 0;
	m_nLid = 0;
	m_nFailCount = 0;
}

VOID CClientObj::OnConnection()  //如果连接
{
	//SendLoginSuccess( 10000 );
	//m_nSid = 10001;
	//m_SelectCharServer.addr.SetAddr( "127.0.0.1" );
	//m_SelectCharServer.addr.nPort = 7200;
	//SendSelectServerOk();
	//PostMsg( "#+DES!", 6 );

	char * p = (char*)CServer::GetInstance()->GetServerTips();  //获取Usertip.txt
	if( p[0] != 0 )
	{
		SendMsg( 0, 0xafa, 0, 0, 0, (LPVOID)p );
	}
	m_TimeOut.Savetime();
}

VOID CClientObj::Update()
{
	if (m_TimeOut.IsTimeOut(5 * 60 * 1000))
	{
		Disconnect();
		return;
	}
	if (m_nFailCount >= 16)
	{
		Disconnect();
		return;
	}
	CClientObject::Update();
}

VOID CClientObj::OnDBMsg(PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	CIOConsole* pConsole = (CIOConsole*)pServer->GetIoConsole();
	switch (pMsg->wCmd)
	{
	case DM_CHANGEPASSWORD: //修改密码
	{
		if (pMsg->wParam[0] == SE_OK)
		{
			SendMsg(0, SM_CHANGEPASSWORDOK, 0, 0, 0, 0, 0);
		}
		else
		{
			int error = 0;
			if (pMsg->wParam[0] == SE_LOGIN_PASSWORDERROR)
				error = -1;
			else if (pMsg->wParam[0] == SE_LOGIN_ACCOUNTNOTEXIST)
				error = 0;
			else
				error = -2;
			SendMsg(static_cast<DWORD>(error), SM_CHANGEPASSWORDFAIL, 0, 0, 0, 0, 0);
			LG2("修改密码失败：%s, %s, %u\n", getAddress(), m_szAccount, error);
		}
	}
	break;
	case DM_CHECKACCOUNT:  //请求检查账号
	{
		//pConsole->OutPut( ERROR_RED, "result = %d\n", pMsg->wParam[0] );
		m_nLid = static_cast<UINT>(Getrand(1000000000));
		if (pMsg->wParam[0] == SE_OK)
		{
			SendLoginSuccess(m_nLid); //登录成功
			LG2("登录成功：%s, %s\n", getAddress(), m_szAccount);
			pConsole->OutPut(SUCCESS_GREEN, "地址: %s 帐号: %s 登陆成功!\n", getAddress(), m_szAccount);
			char* p = (char*)CServer::GetInstance()->GetLoginOkTips();
			if (p[0] != 0)
				SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)p);
		}
		else
		{
			int error = 0;
			if (pMsg->wParam[0] == SE_LOGIN_ACCOUNTNOTEXIST)
				error = 0;
			else if (pMsg->wParam[0] == SE_LOGIN_PASSWORDERROR)
			{
				error = -1;
			}
			else
				error = -2;
			SendLoginFail(error);
			LG2("登录失败：%s, %s, %u\n", getAddress(), m_szAccount, error);
		}
	}
	break;
	case DM_CHECKACCOUNTEXIST:
	{
		if (pMsg->wParam[0] == SE_LOGIN_ACCOUNTNOTEXIST)
			SendMsg(FALSE, SM_CHECKACCOUNTEXISTRET, 0, 0, 0, nullptr, 0);
		else
			SendMsg(TRUE, SM_CHECKACCOUNTEXISTRET, 0, 0, 0, nullptr, 0);
	}
	break;
	case DM_CREATEACCOUNT:
	{
		if (pMsg->wParam[0] == SE_OK)
		{
			SendMsg(0, SM_REGISTERACCOUNTOK, 0, 0, 0, nullptr, 0);
			LG2("创建账号成功：%s, %s\n", getAddress(), m_szAccount);
			char* p = (char*)CServer::GetInstance()->GetRegisterTips();
			if (p[0] != 0)
				SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)p);
		}
		else
		{
			SendMsg(0, SM_REGISTERACCOUNTFAIL, 0, 0, 0, nullptr, 0);
			LG2("创建账号失败：%s, %s\n", getAddress(), m_szAccount);
		}
	}
	break;
	}
}

VOID CClientObj::OnSCMsg(PMIRMSG pMsg, int datasize)
{
	switch (pMsg->wCmd)
	{
	case SCM_FINDSERVER:
	{
		if (pMsg->wParam[0] == SE_OK)
		{
			CHAR szData[200];
			sprintf_s(szData, sizeof(szData), "%u/%u/%s", getId(), m_nLid, m_szAccount);
			//	找到选人服务器
			m_SelectCharServer = *((FINDSERVER_RESULT*)pMsg->data);
			//	跟选人服务器说,我要进去
			CSCClientObj* pObj = (CSCClientObj*)CServer::GetInstance()->GetSCConnection();
			if (pObj)pObj->SendMsgAcrossServer(0, MAS_ENTERSELCHARSERVER, MST_SINGLE, m_SelectCharServer.Id.bIndex,
				szData, static_cast<int>(strlen(szData)));
		}
		else
		{
			//	找不到服务器就关闭连接
			//	清理本地分配的东西
			//Close();
			SendMsg(0, 0xafa, 0, 0, 0, "您选择的服务器不存在!", static_cast<int>(strlen("您选择的服务器不存在!")));
		}
	}
	break;
	default:
		break;
	}
}

VOID CClientObj::OnCodedMsg(xClientObject* pObject, PMIRMSG pMsg, int datasize)
{
	CServer* pServer = CServer::GetInstance();
	BOOL bSaveTime = TRUE;
	switch (pMsg->wCmd)
	{
	case CM_SELECTSERVER:
	{
		if (m_nLid == 0)
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"您还没有登陆!");
			break;
		}
		CSCClientObj* pSCClientObj = (CSCClientObj*)pServer->GetSCConnection();
		if (pSCClientObj != nullptr)
		{
			pSCClientObj->FindServer(getId(), ST_SELCHARSERVER, pMsg->data);
		}
		else
			Disconnect(3000); // 关闭连接
	}
	break;
	case CM_CHANGEPASSWORD:
	{
		pMsg->dwFlag = getId();
		pMsg->wCmd = DM_CHANGEPASSWORD;
		CDBClientObj* pDBClientObject = (CDBClientObj*)pServer->GetDBConnection(0);
		if (pDBClientObject != nullptr)
		{
			pDBClientObject->SendMsg(pMsg, datasize);
		}
		else
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"后台数据库错误, 请联系管理员~");
		}
	}
	break;
	case CM_REGISTERACCOUNT:
	{
		if (CServer::GetInstance()->RegisterDisabled())
		{
			SendMsg(0, SM_REGISTERACCOUNTFAIL, 0, 0, 0, nullptr, 0);
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"帐号注册被禁止, 请稍候再进行注册~");
			break;
		}
		if (datasize != 226)
		{
			SendMsg(0, SM_REGISTERACCOUNTFAIL, 0, 0, 0, nullptr, 0);
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"请使用兼容的登陆器注册账号~!");
			break;
		}

		REGISTERACCOUNT* pReg = (REGISTERACCOUNT*)pMsg->data;
		if (pReg->btAccount > 10 ||
			pReg->btPassword > 10 ||
			pReg->btName > 20 ||
			pReg->btIdCard > 19 ||
			pReg->btPhoneNumber > 14 ||
			pReg->btA1 > 20 ||
			pReg->btQ1 > 20 ||
			pReg->btEmail > 40 ||
			pReg->btA2 > 20 ||
			pReg->btQ2 > 20 ||
			pReg->btBirthday > 10 ||
			pReg->btMobileNumber > 11)
		{
			SendMsg(0, SM_REGISTERACCOUNTFAIL, 0, 0, 0, nullptr, 0);
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"注册信息有误, 请使用兼容的登陆器注册账号~!");
			break;
		}
		pMsg->dwFlag = getId();
		pMsg->wCmd = DM_CREATEACCOUNT;
		CDBClientObj* pDBClientObject = (CDBClientObj*)pServer->GetDBConnection(0);
		if (pDBClientObject != nullptr)
		{
			pDBClientObject->SendMsg(pMsg, datasize);
		}
		else
		{
			SendMsg(0, SM_REGISTERACCOUNTFAIL, 0, 0, 0, nullptr, 0);
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"后台数据库错误, 请联系管理员~");
		}
	}
	break;
	case CM_CHECKACCOUNTEXIST:
	{
		pMsg->dwFlag = getId();
		pMsg->wCmd = DM_CHECKACCOUNTEXIST;
		CDBClientObj* pDBClientObject = (CDBClientObj*)pServer->GetDBConnection(0);
		if (pDBClientObject != nullptr)
		{
			pDBClientObject->SendMsg(pMsg, datasize);
		}
		else
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"后台数据库错误, 请联系管理员~");
		}
	}
	break;
	case CM_LOGIN:
	case CM_PTLOGIN:
	{
		pMsg->dwFlag = getId();
		pMsg->wCmd = DM_CHECKACCOUNT;
		CDBClientObj* pDBClientObject = (CDBClientObj*)pServer->GetDBConnection(0);
		if (pDBClientObject != nullptr)
		{
			// first get account as local account
			char* p = strchr(pMsg->data, '/');
			if (p != nullptr)
			{
				*p = 0;
				strncpy(m_szAccount, pMsg->data, 10);
				m_szAccount[10] = 0;
				*p = '/';
			}
			pDBClientObject->SendMsg(pMsg, datasize);
		}
		else
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"后台数据库错误, 请联系管理员");
		}
	}
	break;
	case 0x1c9: // 微端消息
	{
		const char* p = CServer::GetInstance()->GetWAddr();
		if (p && p[0] != 0)
		{
			P2PServer p2pServer;
			p2pServer.btRec = 01;
			inet_pton(AF_INET, p, &p2pServer.nIPAddr);
			p2pServer.wPort = CServer::GetInstance()->GetWPort();
			SendMsg(0, 0xafe, 0, 0, 0, &p2pServer, sizeof(P2PServer));
		}
	}
	break;
	case 0x7e3:
	{
		DWORD dwTime = MAKEDWORD2W(pMsg->wParam[0], pMsg->wParam[1]);
		if (dwTime != 20161104) // 如果不等, 就说明引擎不支持这个客户端
		{
			SendMsg(0, 0xafa, 0, 0, 0, (LPVOID)"你的客户端版本不支持!");
			Disconnect(3000); // 关闭连接
		}
	}
	break;
	default:
	{
		bSaveTime = FALSE;
		m_nFailCount++;
#ifdef _DEBUG
		pServer->OnUnknownMsg(pMsg, datasize);
#endif
	}
	break;
	}
	if (bSaveTime)
		m_TimeOut.Savetime();
}

VOID CClientObj::SendLoginFail(DWORD error)
{
	SendMsg(error, SM_LOGINFAIL, 0, 0, 0, nullptr, 0);
}

VOID CClientObj::SendLoginSuccess(UINT nLoginId)
{
	char szBuffer[64];
	sprintf_s(szBuffer, sizeof(szBuffer), "*%u", nLoginId);
	SendMsg(0, SM_LOGINOK, 0, 0, 0, szBuffer, static_cast<int>(strlen(szBuffer)));
}

VOID CClientObj::OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize)
{
	switch (wCmd)
	{
	case MAS_ENTERSELCHARSERVER:
	{
		//	data = "lid/sid/account"
		char* Params[5];
		int nParam = SearchParam((char*)pszData, Params, 5, '/');
		if (nParam == 3)
		{
			if (m_nLid == static_cast<UINT>(atoi(Params[0])) && strcmp(m_szAccount, Params[2]) == 0)
			{
				m_nSid = static_cast<UINT>(atoi(Params[1]));
				SendSelectServerOk();
			}
		}
		else
		{
			close();
		}
	}
	break;
	}
}

VOID CClientObj::SendSelectServerOk()
{
	CHAR szData[200];
	sprintf_s(szData, sizeof(szData), "%s/%u/%u", m_SelectCharServer.addr.addr, m_SelectCharServer.addr.nPort, m_nSid);
	SendMsg(getId(), 0xaff, 1, 0, 0); // 1是1.9人物选择界面、3是时长专用
	SendMsg(0, SM_SELECTSERVEROK, 0, 0, 0, (LPVOID)szData, static_cast<int>(strlen(szData)));
}