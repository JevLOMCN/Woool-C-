#pragma once
class CSe_Page;
class CSe_Goods;

typedef struct tagScriptVersion
{
	tagScriptVersion()
	{
		FILLSELF(0);
	}
	union
	{
		struct
		{
			BYTE v1;
			BYTE v2;
			BYTE v3;
			BYTE v4;
		};
		DWORD dwVersion;
	};
}ScriptVersion;

class CScriptObject :
	public xVariableProvider
{
public:
	CScriptObject(void);
	virtual ~CScriptObject(void);
	BOOL Load(const char* pszFilename);
	VOID Destroy();
	CSe_Page* GetPage(const char* pszPage);

	const char* getName() { return m_szName; }
	ScriptVersion& getVersion() { return m_Version; }
	CSe_Goods* getGoodsList() { return m_pGoodsList; }
	BOOL IsItemTradeble(BYTE btStdMode);
	VOID Reload();
	BOOL IsBigBox()const { return m_bBigBox; }
	CScriptObject* GetLeftPage();
private:
	char m_szFileName[1024];
	VOID AddTradeItemType(BYTE btType);
	xStringList<32> m_xPageList;
	xVarList<32> m_xVarList;
	CSe_Page* m_pPageList;
	CSe_Goods* m_pGoodsList;
	ScriptVersion m_Version;
	char m_szName[64];
	BYTE m_btTradeItemType[256];
	int	 m_iTradeItemCount;
	BOOL m_bBigBox;
	char m_szLeftPage[256];
	CScriptObject* m_pLeftPage;
};