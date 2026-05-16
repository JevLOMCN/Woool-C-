#include "..\header\xiocpserver.h"
#include "..\header\xserverobject.h"

xServerObject::xServerObject(void)
{
	m_pxIocpServer = nullptr;
}

xServerObject::~xServerObject(void)
{
	m_pxIocpServer = nullptr;
}
