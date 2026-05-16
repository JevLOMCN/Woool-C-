#include "StdAfx.h"
#include ".\eventprocessor.h"

CEventProcessor::CEventProcessor(void) : xListHost<CEventProcessor>::xListNode(this)
{
}

CEventProcessor::~CEventProcessor(void)
{
}