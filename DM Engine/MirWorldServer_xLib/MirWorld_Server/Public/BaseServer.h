#pragma once
#include "virtualserver.h"
#include "clientobject.h"
#include "ioconsole.h"
#include "scclientobj.h"
#include "dbclientobj.h"
#include "baniplistex.h"
#include "inc.h"
#include "msgprocessor.h"

#define UPDATE_LOOP 100	//更新循环最大数
class CTempClient;

class CBaseServer :
	public CVirtualServer, public xEventListener, public CInputListener,
	public xThread, public CMsgProcessor
{
public:
	CBaseServer(void);
	virtual ~CBaseServer(void);
public:
	virtual VOID GetServerState(SERVERSTATE& state);
	BOOL Start(CSettingFile&);
	BOOL Stop();
	VOID	UpdateTotalInfo();
public:
	VOID OnUnCodeMsg(xClientObject* pObject, const char* pszMsg, int size);
	VOID OnCodedMsg(xClientObject* pObject, PMIRMSG	pMsg, int datasize);
	DWORD GetUpdateKey()const { return m_nLoopCount; }
	virtual CClientObject* NewClientObject() { return nullptr; }
	virtual VOID DeleteClientObject(CClientObject* pObject) {}
	virtual CClientObject* GetClientObject(UINT id) { return nullptr; }
	virtual BOOL InitServer(CSettingFile&) { return TRUE; }
	virtual VOID CleanServer() {}
	virtual VOID Update();
	virtual VOID OnDBMsg(PMIRMSG pMsg, int datasize) {}
	virtual VOID OnSCMsg(PMIRMSG pMsg, int datasize);
	virtual VOID OnSCServerReady() {}
	virtual VOID OnDBServerReady(int index) {}
	virtual VOID OnMASMsg(WORD wCmd, WORD wType, WORD wIndex, const char* pszData, int datasize) {}
	virtual VOID WillClose() {}
public:
	VOID OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam);
	UINT GetDisconnectId()
	{
		if (m_DisconnectQueue.GetCount() == 0)return 0;
		UINT id = 0;
		if (m_DisconnectQueue.Pop(id))
			return id;
		return 0;
	}
public:
	//VOID ExectueProc( LPVOID lpParam );
	CIOConsole* GetIoConsole() { return m_pIoConsole; }
	void SetIoConsole(const CIOConsole* pIoConsole) { m_pIoConsole = (CIOConsole*)pIoConsole; }
	void SetServerName(const char* pServerName) { m_pServerName = (char*)pServerName; }
	const char* GetServerName() { return m_pServerName; }
	void SetNickName(const char* pNickName) { strncpy(m_szNickName, pNickName, 64);m_szNickName[63] = 0; }
	const char* GetNickName() { return m_szNickName; }
	CDBClientObj* GetDBConnection(int index) { if (index < 0 || index >= m_iDBServerCount) return nullptr;return &m_DBClientObjs[index]; }
	CSCClientObj* GetSCConnection() { return &m_SCClientObj; }
	int	GetDBConnectionCount()const { return m_iDBServerCount; }
	// 打印消息日志, type 0未知消息 / 1接收消息 / 2发送消息
	VOID OnUnknownMsg(PMIRMSG pMsg, int datasize, int type = 0);
	ServerId& GetServerId() { return m_Ident; }
	SERVERADDR* GetDBAddr(int index) { if (index < 0 || index >= m_iDBServerCount) return nullptr; return m_DBAddrs + index; }
	BOOL	GetAddrByName(const char* pszHostName, char* pszIpAddress);
	virtual VOID OnInput(const char* pszString);
protected:
	VOID Execute(LPVOID lpParam);
	//	处理连接相关的事件, 连上和断开
	VOID ProcConnectionEvent();
	//	更新sc的消息
	VOID UpdateSCServer();
	//	更新db的消息
	VOID UpdateDBServer();
	//	连接sc的服务器
	BOOL ConnectSCServer();
	//	连接db的服务器
	BOOL ConnectDBServer();
	//VOID OnDisconnect(CSocketUnit*pUnit);
	CDQueue<UINT, 1024>	m_DisconnectQueue;
	char* m_pServerName;
	char m_szNickName[64];
	CIOConsole* m_pIoConsole;
	CSCClientObj m_SCClientObj;
	CDBClientObj m_DBClientObjs[2];
	servertype m_Type;
	ServerId m_Ident;
	SERVERADDR m_ServerAddr;
	SERVERADDR m_SCAddr;
	int m_iDBServerCount;
	SERVERADDR m_DBAddrs[2];
	//	for total
	SYSTEMTIME m_stStartTime;
	BOOL m_bStateInfoUpdated;
	FLOAT m_fLoopTime;
	DWORD m_dwLoopTimes;
	DWORD m_dwLoopStartTime;
	DWORD m_dwConnectedTimes;
	DWORD m_dwDisconnectTimes;
	CHAR m_szServerState[1024];
	DWORD m_dwPreConnections;
	xObjectPool<CTempClient > m_xTempClients;
	xPtrQueue<CTempClient> m_xQTempClient;
	char m_szBanIpFile[256];
	char m_szTrustIpFile[256];
	CIpListEx m_BanList;
	CIpListEx m_TrustList;
	xIocpServer	m_xIocpServer;
private:
	DWORD m_nLoopCount; // 整体分帧更新索引
};
