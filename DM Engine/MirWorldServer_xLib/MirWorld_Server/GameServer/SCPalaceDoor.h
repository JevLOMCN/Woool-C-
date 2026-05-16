#pragma once
#include "changemapevent.h"

class CSCPalaceDoor : public CChangeMapEvent
{
public:
	CSCPalaceDoor(void);
	virtual ~CSCPalaceDoor(void);
	virtual VOID OnEnter(CMapObject* pObject);
};
