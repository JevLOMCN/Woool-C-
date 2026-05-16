#include "..\header\xlistenobject.h"
#include "..\header\xiocpunit.h"
#include "..\header\xclientobject.h"
#include "..\header\xInc.h"
#include "..\header\xiocpserver.h"

xListenObject::xListenObject(void)
{
	m_Id = 0;
}

xListenObject::~xListenObject(void)
{
}

VOID xListenObject::OnEvent(xEventSender* pSender, int iEvent, int iParam, LPVOID lpParam)
{
	sockaddr* p1 = nullptr, * p2 = nullptr;
	int	len1, len2;
	if (iEvent == xIocpUnit::IUE_ACCEPT)
	{
		xIocpUnit* pIocpUnit = static_cast<xIocpUnit*>(lpParam);

		xAcceptSocket* pAcceptSocket = static_cast<xAcceptSocket*>(pIocpUnit->getData());
		GetAcceptExSockaddrs(static_cast<LPVOID>(pAcceptSocket->getBuffer()), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &p1, &len1, &p2, &len2);
		if (p2 != nullptr)
		{
			char* p = inet_ntoa(reinterpret_cast<SOCKADDR_IN*>(p2)->sin_addr);
			if (p != nullptr)
				pAcceptSocket->setEndPoint(p, ntohs(reinterpret_cast<SOCKADDR_IN*>(p2)->sin_port));
		}
		pAcceptSocket->setState(SS_ACCEPTED);
		getServer()->onConnection(pAcceptSocket, getId());
		pAcceptSocket->reset();
		postAccept((xIocpUnit*)pSender, pAcceptSocket);
	}
}

void xListenObject::postAccept(int nAccept)
{
	xIocpUnit* pIocpUnit = nullptr;
	xAcceptSocket* pAcceptSocket = nullptr;
	
	// 批量预分配对象，减少分配开销
	int actualAccept = max(nAccept, 128);  // 确保至少128个Accept
	for (int i = 0; i < actualAccept; i++)
	{
		if (pIocpUnit == nullptr)pIocpUnit = getServer()->newIocpUnit();
		if (pIocpUnit == nullptr)continue;
		if (pAcceptSocket == nullptr)pAcceptSocket = newAcceptSocket();
		if (pAcceptSocket == nullptr)continue;
		
		// 对新socket应用性能优化
		pAcceptSocket->setReuseAddr(TRUE);
		pAcceptSocket->setTcpNoDelay(TRUE);
		
		pIocpUnit->setEventListener(this);
		pIocpUnit->setType(IO_ACCEPT);
		pIocpUnit->setData(pAcceptSocket);
		if (postAccept(pIocpUnit, pAcceptSocket))
		{
			pIocpUnit = nullptr;
			pAcceptSocket = nullptr;
		}
	}
	if (pIocpUnit)getServer()->releaseIocpUnit(pIocpUnit);
	if (pAcceptSocket)releaseAcceptSocket(pAcceptSocket);
}

BOOL xListenObject::postAccept(xIocpUnit* pIocpUnit, xAcceptSocket* pAcceptSocket)
{
	return acceptEx(*pAcceptSocket, static_cast<LPVOID>(pAcceptSocket->getBuffer()), 0, pIocpUnit->getOverlappedEx()->dwNumberOfBytes, pIocpUnit->getOverlapped());
}
