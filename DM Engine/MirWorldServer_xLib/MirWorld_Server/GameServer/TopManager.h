#pragma once

#pragma pack(push)
#pragma pack(1)
typedef struct tagLevelRank
{
	tagLevelRank()
	{
		FILLSELF(0);
	}
	WORD wIndex;         // 名次
	char szName[14];     // 名称
	BYTE btLevel;        // 等级
	BYTE btJob;          // 职业(0,1,2)
	BYTE btSex;          // 性别(0,1)
	char szGuildName[30];// 行会
	BYTE btPerCent;      // 当前经验百分数
	BYTE btYsJob;        // 元神职业(0,1,2)
	BYTE btYsLevel;      // 元神等级
	BYTE unkown[16];	 // 未知
} LevelRank; // 等级榜

typedef struct tagPetRank
{
    tagPetRank()
    {
		FILLSELF(0);
    }
    WORD wIndex;        // 名次
    char szName[14];    // 名称
	WORD wLevel;        // 等级(0-7)
	WORD btPerCent;     // 当前经验百分数, 客户端处理时值会除以10显示
    char szFengHao[14]; // 称号
    char szMaster[14];  // 主体名字
	BYTE btMasterJob;   // 主人职业
} PetRank; // 宠物榜

typedef struct tagShadowRank
{
	tagShadowRank()
	{
		FILLSELF(0);
	}
	WORD wIndex; //名次
	char szName[14]; // 名字
	BYTE btLevel; // 等级
	BYTE btJob; // 职业
	BYTE btSex; // 性别
	BYTE btJiPin; // 体魄
	BYTE btTiPo; // 体魄
	char szMasterName[14]; // 主体名字
	BYTE btMasterLevel; // 主体等级
	char szMasterGuildName[30]; // 主体行会
} ShadowRank; // 元神榜

typedef struct tagMasterRank
{
    tagMasterRank()
    {
		FILLSELF(0);
    }
    WORD wIndex;                 // 名次
    char szName[14];             // 名字
    WORD wTudiCount;             // 出师徒弟数
    BYTE btLevel;                // 等级
    BYTE btJob;                  // 职业(0,1,2)
    BYTE btSex;                  // 性别(0,1)
    char szGuild[30];            // 行会
	WORD wRepute;                // 声望
	BYTE btNowTudiCount;         // 当前徒弟数
} MasterRank; // 师傅榜
#pragma pack(pop)

class CHumanPlayer;
class CScriptNpc;
// 英雄榜和天下第一
class CTopManager : public xSingletonClass<CTopManager>
{
public:
	CTopManager(void);
	virtual ~CTopManager(void);
	// 进入前100排行榜
	VOID EnterTop100(CHumanPlayer* pPlayer);
	// 加入天下第一
	BOOL EnterProfessionTop(CHumanPlayer* pPlayer);
	// 发送排行榜数据到客户端
	VOID SendRank(CHumanPlayer* pPlayer, int nType, int StartPos);
	// 加载天下第一数据
	VOID LoadFigure(const char* pszFile);
	// 加载排行榜数据
	VOID Load(const char* pszFile);
	// 保存排行榜数据
	VOID Save(const char* pszFile)const;
	// 设置天下第一NPC
	VOID SetTopNpc(CScriptNpc* pNpc, BYTE btPro, BYTE btSex);
	// 更新排行榜信息
	BOOL UpdateTopInfo(CHumanPlayer* pPlayer);
	// 获取天下第一NPC显示
	int GetTopView(int index);
	// 获取天下第一玩家信息
	TopCharInfo* GetTopCharInfo(int index)
	{
		if (index < 0 || index >= 6)return nullptr;
		return &m_ProfessionTop[index];
	}
	// 获取排行榜玩家信息
	TopCharInfo* GetTop100CharInfo(int index, int iType)
	{
		if (index < 0 || index >= 100)return nullptr;
		TopCharInfo* cahrInfo = &m_Top100[index];
		switch (iType)
		{
		case 0x100: // 全部
			return cahrInfo;
		break;
		case 0x101: // 战士
			if (cahrInfo->btClass == JOB_WAR) return cahrInfo;
		break;
		case 0x102: // 魔法师
			if (cahrInfo->btClass == JOB_MAG) return cahrInfo;
		break;
		case 0x103: // 道士
			if (cahrInfo->btClass == JOB_TAO) return cahrInfo;
		break;
		default:
			if (iType >= 0x104 && iType <= 0x113)
			{
				if (cahrInfo->wLevel == iType - 225) return cahrInfo;
				if (iType == 0x113 && cahrInfo->wLevel >= 50) return cahrInfo;
			}
		break;
		}
		return nullptr;
	}
	// 获取玩家排行序号
	int GetTopIndex(const char* szName)const;
protected:
	TopCharInfo m_Top100[100]; // 排行榜100玩家
	int	m_iTop100Count; // 排行榜数量
	TopCharInfo m_ProfessionTop[6]; // 天下第一玩家
	CScriptNpc* m_ProTopNpc[6]; // 天下第一NPC
	char m_szListFile[1024]; // 排行榜文件名
};