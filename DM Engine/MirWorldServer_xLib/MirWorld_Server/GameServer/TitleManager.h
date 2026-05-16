#pragma once
typedef char TITLE_NAME[64];
typedef struct tag_Title
{
	tag_Title()
	{
		FILLSELF(0);
	}
	DWORD dwLevel;
	DWORD dwExp;
	TITLE_NAME strTitle[3];
}TITLE;
class CHumanPlayer;

// 玩家封号管理
class CTitleManager : public xSingletonClass<CTitleManager>
{
public:
	CTitleManager(void);
	virtual ~CTitleManager(void);
	// 加载玩家封号标题数据
	VOID LoadData(const char* pszData, BOOL bCSV = FALSE);
	// 获取封号标题
	BOOL GetTitle(CHumanPlayer* player, char* pszTitle);
	// 获取封号标题
	BOOL GetTitle(CHumanPlayer* player, char* pszTitle, int& index);
private:
	TITLE* m_pTitles;
	int	m_iTitleCount;
};