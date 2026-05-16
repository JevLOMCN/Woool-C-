#pragma once
typedef struct tagEventMapPosition
{
	UINT nMapId;
	UINT x;
	UINT y;
	UINT nDelay;
	DWORD dwFlag;
}EventMapPosition;

class CMapScriptManager :
	public xSingletonClass<CMapScriptManager>
{
public:
	CMapScriptManager(void);
	virtual ~CMapScriptManager(void);
	VOID Load(const char* pszFilename);
	VOID AddMapScript(EventMapPosition* pPos, const char* pszPage);
protected:
};