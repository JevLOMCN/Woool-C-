#include "..\header\xiocpunit.h"

xIocpUnit::xIocpUnit(void)
{
	m_type = IO_NOTSET;
	m_UnitStruct.pUnit = this;
	m_lpData = nullptr;
}

xIocpUnit::~xIocpUnit(void)
{
}

VOID xIocpUnit::OnComplete(DWORD dwNumberOfBytes, DWORD dwCompletionKey)
{
	sendEvent(IUE_ID + m_type, (int)dwNumberOfBytes, (LPVOID)this);
}