#pragma once
#include "findfile.h"

//行会管理层的权限值
static const WORD nGuildOfficialPermission[11] = { 0, 65535, 8184, 0, 3864, 0, 1032, 0, 1032, 0, 1024 };
//行会管理层的各层数量
static const BYTE nGuildOfficialCountMax[11] = { 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
static const BYTE btGuildOfficialCrony[11] = { 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0 };
static const DWORD nGuildOfficialPrice[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //{ 0, 500, 400, 0, 300, 0, 200, 0, 100, 0, 50 };
static const BYTE nGuildOfficialCount[11][11] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},//0位置不使用
	{0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
	{0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1},
	{0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1},
	{0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1},
	{0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 2},
	{0, 1, 1, 0, 1, 0, 1, 0, 2, 0, 2},
	{0, 1, 1, 0, 1, 0, 2, 0, 2, 0, 2},
	{0, 1, 1, 0, 2, 0, 2, 0, 2, 0, 2},
	{0, 1, 2, 0, 2, 0, 2, 0, 2, 0, 2}
};
static const char* sGuildOfficial[11] = {
	"成员",
	"掌门人",
	"副会长",
	"副会长亲信",
	"长老",
	"长老亲信",
	"护法",
	"护法亲信",
	"堂主",
	"堂主亲信",
	"香主"
};

struct GuildRankInfo
{
	CGuildEx* pGuild;
	UINT nLevel;
	UINT nExp;
};

#pragma pack(push)
#pragma pack(1)
typedef struct tagGuildRank
{
	tagGuildRank()
	{
		FILLSELF(0);
	}
	WORD wIndex;              // 名次
	char szGuildName[30];     // 名称
	WORD wNum46;              // 46等级以上人数
	WORD wNumYs;              // 元神人数
	WORD wNum;                // 总人数
	DWORD dwExp;              // 行会经验
	WORD unkown1;			//未知
	WORD wLevel;			//通灵塔等级
	WORD unkown2;			//未知
} GuildRank; // 行会排行榜
#pragma pack(pop)

class CGuildEx;
class CHumanPlayer;
class CGuildManagerEx : public CFindFile, public xSingletonClass<CGuildManagerEx>
{
public:
	CGuildManagerEx(void);
	virtual ~CGuildManagerEx(void);
	CGuildEx* FindGuild(const char* pszName);
	void LoadGuildUpExp(const char* pszfile, DWORD dwCount);
	BOOL BuildGuild(CHumanPlayer* pCreator, const char* pszName);
	void SaveAll();
	DWORD GetUpExp(int index) { return m_UpExp[index]; }
	// 所有行会排序返回,记住使用了记得 delete[] pTempArray;
	VOID GuildsRankInfo(GuildRankInfo*& pTempArray, int& nValidCount);
	// 获取行会排行榜等级前的数据
	int GetTopGuilds(GuildRank* pGuilds, int maxCount = 10);
	// 获取玩家的行会排行数据
	VOID GetPlayerTopGuild(CGuildEx* pGuild, GuildRank& pGuildRank);
	// 获取行会招募列表
	std::string GetRecruitStateList();
protected:
	void OnFoundFile(const char* pszFileName, UINT nParam = 0);
	CGuildEx* NewGuild(const char* pszName = nullptr);
	void DeleteGuild(CGuildEx* pGuild);
	xObjectPool<CGuildEx> m_xGuildPool;
	xStringList<100> m_xGuildList;
protected:
	std::vector<DWORD> m_UpExp;
};