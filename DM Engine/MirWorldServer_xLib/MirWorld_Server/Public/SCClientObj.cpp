#include "StdAfx.h"
#include ".\scclientobj.h"

CSCClientObj::CSCClientObj(void)
{
	Clean();
}

CSCClientObj::~CSCClientObj(void)
{
}

void CSCClientObj::Update()
{
	if (this == nullptr)return;
	CClientObject::Update();
}

VOID CSCClientObj::RegisterServer(SERVERADDR& Addr, ServerId& Id, const char* pszNickName)
{
	if (this == nullptr)return;
	REGISTER_SERVER_INFO info;
	info.addr = Addr;
	info.Id = Id;
	strncpy(info.szName, pszNickName, 64);
	SendMsg(0, SCM_REGISTERSERVER, 0, 0, 0, &info, sizeof(info));
}

VOID CSCClientObj::UpdateInfo(DWORD dwConnections)
{
	if (this == nullptr)return;
	SendMsg(dwConnections, SCM_UPDATESERVERINFO, 0, 0, 0, nullptr, 0);
}

VOID CSCClientObj::FindServer(UINT id, servertype type, const char* pszServerName)
{
	if (this == nullptr)return;
	SendMsg(id, SCM_FINDSERVER, (WORD)type, 0, 0, (LPVOID)pszServerName, static_cast<int>(strlen(pszServerName)));
}

VOID CSCClientObj::SendMsgAcrossServer(UINT id, WORD wCmd, MAS_SENDTYPE sendtype, WORD wParam, const char* pszData, int datasize)
{
	if (this == nullptr)return;
	SendMsg(id, SCM_MSGACROSSSERVER, wCmd, (WORD)sendtype, wParam, (LPVOID)pszData, datasize);
}

VOID CSCClientObj::GetGameServerAddr(UINT id, const char* pszServerName, const char* pszMapInfo)
{
	if (this == nullptr)return;
	SendMsg(id, SCM_GETGAMESERVERADDR, 0, 0, 0, (LPVOID)pszServerName);
}
