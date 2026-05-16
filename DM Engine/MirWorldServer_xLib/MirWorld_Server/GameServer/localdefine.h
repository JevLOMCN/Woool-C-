#pragma once
#include "scriptdef.h"
#include "marketdef.h"
#include "stringdef.h"
#include <cmath>

#ifdef _DEBUG
#define DPRINT CServer::GetInstance()->GetIoConsole()->OutPut
#define DSYSMSG	SaySystem
#else
inline void empty_func(...) {}
#define DPRINT empty_func
#define	DSYSMSG empty_func
#endif
#define PRINT CServer::GetInstance()->GetIoConsole()->OutPut

typedef struct tagStringCacheNode
{
	char szString[256];
}StringCacheNode;

enum e_process //最大支持64个
{
	//公共的线程
	EP_NOTIFYCHANGENAME,	//更改名字通知
	EP_SETSYSTEMFLAG,		//设置系统标志
	EP_ENTERMAP,			//进入地图(MAPID,X,Y);
	EP_REFRESHFEATURE,		//刷新特征
	EP_SETSTATUS,			//设置状态
	EP_CLRSTATUS,			//清除状态
	EP_MAGHEALING,			//生命回复
	EP_RUSH,				//野蛮冲撞
	EP_RUSHHIT,				//突斩
	EP_FIREBURN,			//火墙
	EP_DEAD,				//死亡
	EP_BEATTACKED,			//攻击
	EP_CHANGEMAP,			//从连接点切换地图
	//玩家的线程
	EP_OPENSCRIPTPAGE,		//打开脚本页
	EP_CLOSEPAGE,			//关闭脚本页
	EP_FIRSTLOGINPROCESS,	//首次登录
	EP_RECOVERHP,			//回复生命值
	EP_RECOVERMP,			//回复魔法值
	EP_SCROLLTEXT,			//滚动文本
	EP_SECNONDTIMEOUT,		//倒计时
	EP_FIRERAIN,			//流星火雨
	EP_POISON,				//施毒术
	EP_STRAW,				//诅咒术
	EP_DEFENCEUP,			//幽灵盾
	EP_MAGDEFENCEUP,		//神圣战甲术
	EP_GROUPCLOAK,			//群体隐身术
	EP_GROUPMAGHEALING,		//群体治愈术
	EP_SKILLFLY,			//技能飞行
	EP_SENDCODEDTEXT,		//发送编码文本
	EP_HOME,				//回家
	EP_RANDOMTELEPORT,		//随机移动
	EP_ESCAPEFROMHOLE,		//离开洞穴
	EP_SABUKHOME,			//回沙城
	EP_ONMISPELLCAST,		//施放魔法(MAPID, X, Y, )
	EP_BIGMAGHEALING,		//最大魔法回复
	EP_CREATEBAGITEM,		//创建背包物品
	EP_MAGICBOOM,			//魔法爆炸(MAPID, (X,Y), nDamage, nWide )
	EP_WARSTART,			//攻城开始
	EP_WAREND,				//攻城结束
	EP_APPEAR,				//OBJECT对象出现
	EP_GODBLESS,			//附身EP_GO
	EP_SPECIALPOTION,		//特效药水-攻击神水、魔力神水、精神神水、极速神水、体力强效神水、魔力强效神水
	EP_CATCHHORSE,			//捕捉野生马匹
	EP_TIMECHANGED,			//时间改变
	EP_WEATHERCHANGED,		//天气改变
	EP_MAPTELEPORT,			//地图连接点
	EP_PUTITEM,				//提交物品
	EP_EXCHANGEBOX,			//宝箱开启中奖
	EP_MAX,					//分界线判断值
	//怪的线程
	EM_RELIVE,				//复活
	E_MAX,
};

enum e_humanattackmode
{
	HAM_CRIME,		//善恶攻击
	HAM_ALL,		//全体攻击
	HAM_PEACE,		//和平攻击
	HAM_GROUP,		//组队攻击
	HAM_GUILD,		//行会攻击
	HAM_MASTER,		//师徒攻击
	HAM_COUPLE,		//夫妻攻击
	HAM_MAX,
};

static const char* g_pAttackModeDesc[HAM_MAX] =
{
	"善&恶",
	"全体",
	"和平",
	"编组",
	"行会",
	"师徒",
	"夫妻"
};

enum e_chatchannel //聊天频道
{
	CCH_NORMAL,	//普通频道
	CCH_CRY,	//喊叫频道
	CCH_WISPER,	//密谈频道
	CCH_GM,		//红字频道
	CCH_GROUP,	//组队频道
	CCH_GUILD,	//行会频道
	CCH_COUPLE,	//夫妻频道
	CCH_FRIEND,	//好友频道
	CCH_MAX,	//	
};

static const char* g_pChatChannelDesc[CCH_MAX] =
{
	"普通",
	"大叫",
	"密聊",
	"GM",
	"组队",
	"行会",
	"夫妻",
	"好友",
};

const DWORD CC_RED = 0x38ff;		//红色
const DWORD CC_REDPET = 16529663ul;	//红色宠物
const DWORD CC_PETDIE = 0xfdff;		//宠物死亡
const DWORD CC_GREEN = 0xfcffdb;	//绿色
const DWORD CC_GREENS = 9961435ul;	//绿色S
const DWORD CC_GREENB = 3735515ul;	//宝宝B
const DWORD CC_MAGICTIPS = 0xdbffdb;//魔法提示
const DWORD CC_CRY = 0x389700;		//大喊
const DWORD CC_NORMAL = 0xff00;		//普通
const DWORD CC_WISPER = 0xfffc;		//密语
const DWORD CC_GROUP = 0xff2f;		//组队
const DWORD CC_EXCHANGE = 0xfcfffc;	//交易
const DWORD CC_GROUPTIPS = 0xfcff00;//组队提示

static const DWORD g_ChatChannelAttrib[CCH_MAX] =
{
	CC_NORMAL,
	CC_CRY,
	CC_WISPER,
	CC_RED,
	CC_GROUP,
	CC_GREEN,
	CC_MAGICTIPS,
	CC_GREEN,
};

static const WORD g_ChatChannelMsg[CCH_MAX] =
{
	0x28,
	0x64,
	0x67,
	0x64,
	0x68,
	0x68,
	0x64,
	0x64,
};

//Server.txt文件配置的Name值,字符串
enum e_nameindex //名字索引
{
	ENI_GOLD,				//金币
	ENI_MALE,				//男
	ENI_FEMALE,				//女
	ENI_GUILDNOTICE,		//门派公告
	ENI_ALLYGUILDS,			//联盟门派
	ENI_KILLGUILDS,			//敌对门派
	ENI_MEMBERS,			//成员
	ENI_GUILDDECLARATION,	//行会宣言
	ENI_GUILDAPPLYLIST,		//行会申请列表
	ENI_WARR,				//战士
	ENI_MAGICAN,			//法师
	ENI_TAOSHI,				//道士
	ENI_TOPOFWORLD,			//世界排行榜
	ENI_UPGRADEMINESTONE,	//升级矿石
	ENI_PHYSICSMAPPATH,		//物理地图路径
	ENI_PHYSICSCACHEPATH,	//物理缓存路径
	ENI_MAX,				//最大索引
};

//Server.txt文件配置的var值, 数值
enum e_varindex	//变量索引
{
	EVI_MAXGOLD,				//最大小包金币, 小金币包
	EVI_MAXBIGGOLD,				//最大大包金币, 大金币包
	EVI_MAXYUANBAO,				//最大元宝
	EVI_ITEMUPDATETIME,			//物品更新时间
	EVI_MAXGROUPMEMBER,			//最大组队成员数
	EVI_YELLOWPKPOINT,			//黄名PK上限
	EVI_REDPKPOINT,				//红名PK上限
	EVI_ONCEPKPOINT,			//一次PK点
	EVI_ONEPKPOINTTIME,			//一次PK点时间
	EVI_BODYTIME,				//尸体时间
	EVI_STOREAGESIZE,			//仓库大小
	EVI_SANDCITYTAKETIME,		//沙城占领时间
	EVI_WARENEMYCOLOR,			//敌对行会颜色
	EVI_WARALLYCOLOR,			//联盟行会颜色
	EVI_WARNORMALCOLOR,			//普通行会颜色
	EVI_WARTIMELONG,			//攻城时间
	EVI_WARSTARTTIME,			//攻城开始时间
	EVI_ENABLESAFEAREANOTICE,	//安全区通知
	EVI_ADDFRIENDLEVEL,			//添加好友等级
	EVI_PRIVATESHOPLEVEL,		//摆摊等级
	EVI_INITDRESSCOLOR,			//初始装备颜色
	EVI_REPAIRDAMAGEDURA,		//修复损坏装备
	EVI_GRAYNAMETIME,			//灰名时间
	EVI_DROPTARGETDISTANCE,		//丢弃目标距离
	EVI_DROPTARGETTIME,			//丢弃目标时间
	EVI_WEAPONDAMAGERATE,		//武器伤害倍数
	EVI_DRESSDAMAGERATE,		//装备伤害倍数
	EVI_DEFENCEDAMAGERATE,		//防御伤害倍数
	EVI_JEWELRYDAMAGERATE,		//宝石伤害倍数
	EVI_RANDOMUPGRADEITEMRATE,	//随机升级物品概率
	EVI_IMMDIERATE,				//立即死亡概率
	EVI_SUPERHITRATE,			//超级命中概率
	EVI_SHIELDRATE,				//躲避概率
	EVI_DOUBLEEXPRATE,			//双倍经验概率
	EVI_CHARINFOBACKUPTIME,		//角色信息备份时间
	EVI_DBUPDATEDELAY,			//数据库更新延迟
	EVI_SHAPE_BLACKSTONE,		//黑铁矿石
	EVI_MAXUPGRADETIMES,		//最大升级次数
	EVI_MONGENFACTOR,			//怪物生成倍数
	EVI_PKCURSERATE,			//PK点扣除倍数
	EVI_HPRECOVERPOINT,			//生命恢复点
	EVI_HPRECOVERTIME,			//生命恢复时间
	EVI_MPRECOVERPOINT,			//魔法恢复点
	EVI_MPRECOVERTIME,			//魔法恢复时间
	EVI_GUILDWARTIME,			//行会战时间
	EVI_STARTGUILDMEMBERCOUNT,	//行会创建成员数
	EVI_GUILDLEVELNUM,			//行会等级数量
	//EVI_STATICMAP,
	EVI_MAX,
};

typedef struct tagOBJECTPROCESS //对象线程
{
	tagOBJECTPROCESS()
	{
		FILLSELF(0);
	}
	e_process ident;
	DWORD dwParam[4];
	char* pszParam;
	int iStringSize;
	int repeattimes;		//当前剩余次数
	DWORD dwDelayTime;		//缓冲时间
	DWORD dwDeliverTime;	//添加时间
	BOOL btFirstTime;		//是否添加就执行
	//DWORD dwtarget;		//攻击目标
	//UINT listid;			//表中的id
	//e_object_type attacktype;
}OBJECTPROCESS;

//typedef struct start_point
//{
//	int	mapid;
//	int	x1, y1;
//	int x2, y2;
//}START_POINT;

// 幸运概率表
static const BYTE g_pLucky[11] =
{
	0,
	0,
	0,
	20,
	35,
	45,
	55,
	65,
	80,
	100,
	100,
};

constexpr int MAX_OBJECT_PROCESS = 100000;//最大对象线程数
constexpr int SEARCH_COUNT = 80;//掉落物品搜索数量

typedef struct tagNpcGoodsItemList //NPC列表物品
{
	tagNpcGoodsItemList()
	{
		FILLSELF(0);
	}
	tagNpcGoodsItemList* pNext;
	ITEM item;
	DWORD dwPutTime;
	UINT listid;
}NpcGoodsItemList;

typedef struct tagNpcGoodsList //NPC物品列表
{
	tagNpcGoodsList()
	{
		FILLSELF(0);
	}
	tagNpcGoodsList* pNext;
	char szTemplate[32];	//名字
	int	defaultcount;		//0
	int currentcount;		//金币数
	int refreshtime;		//刷新时间
	DWORD dwTemplatePrice;
	DWORD dwLastRefreshTime;
	UINT listid;
	NpcGoodsItemList* pItemList;
}NpcGoodsList;

#pragma pack(push)
#pragma pack(1)
typedef struct tagViewDetail //观察其他角色信息
{
	tagViewDetail()
	{
		FILLSELF(0);
	}
	DWORD dwFeature;			//外观特征 LOOKS (RaceImg+武器+头发+衣服)
	BYTE btNamelen;
	CHAR szName[15];			//玩家姓名
	DWORD dwNameColor;			//名字颜色
	BYTE btGuildNamelen;
	CHAR szGuildName[14];		//行会名字
	BYTE btTitleNameLen;
	CHAR szTitleName[15];		//称号
	BYTE btSex;					//性别
	ITEMCLIENT items[_U_MAX]; // 装备槽位
}VIEWDETAIL;

typedef struct tagViewDetailEx //观察其他角色扩展信息
{
	tagViewDetailEx()
	{
		FILLSELF(0);
	}
	BYTE btExt[10]; //扩展数据
	BYTE btJob; //职业
	BYTE btRideStatus; // 骑战状态
	BYTE btKown[86]; //未知属性
	WORD wLevel; // 玩家等级-凡人最高71级、72开始是天人1重-最高99重、171开始是天仙1重-最高99重、270开始是上仙1重-最高99重
	BYTE btExtKown[17]; // 未知数据
	DWORD btFunc; //功能-开启(用位计算开启：本体、魔器、骑术、境界、万兽谱、天书 等等...)
}VIEWDETAIL_EX;
#pragma pack(pop)

enum e_maptype //地图类型
{
	EMT_NOFIGHT,	//无战斗地图
	EMT_FIGHT,		//有战斗地图
};

enum e_mapflag //地图标志
{
	EMF_PKMAP = 1,				//PK地图
	EMF_NOPK = 2,				//不能PK
	EMF_NOTELEPORT = 4,			//不能传送
	EMF_NODROPITEM = 8,			//不能掉落物品
	EMF_NORIDE = 16,			//不能骑马
	EMF_UNDERGROUND = 32,		//地下地图
	EMF_SKY = 64,				//天空地图
	EMF_CANWALKTHROUGH = 128,	//允许穿人
};

enum e_object_type //对象类型
{
	OBJ_DOWNITEM,		//掉落物品
	OBJ_MONSTER,		//怪物
	OBJ_NPC,			//NPC
	OBJ_GUARD,			//卫士、弓箭手
	OBJ_PLAYER,			//玩家
	OBJ_VISIBLEEVENT,	//可见事件
	OBJ_PET,			//宠物
	OBJ_EVENT,			//事件
	OBJ_MAX,
};

enum e_object_ApprType //怪物外观分类类型
{
	APPR_MONSTER,		//怪物
	APPR_GUARD,			//卫士、弓箭手
	APPR_TREE,			//树木
	APPR_MAX,
};

enum e_class_type //类的类型
{
	CLS_UNKNOWN,	//未知
	CLS_ALIVEOBJECT,//生物
	CLS_EVENT,		//事件
	CLS_DOWNITEM,	//掉落物品
	CLS_NPC,		//NPC
	CLS_MAX,
};

#define	EVENTFLAG_ENTEREVENT	0x8000	//进入事件
#define	EVENTFLAG_LEAVEEVENT	0x4000	//离开事件
#define	EVENTFLAG_CITYEVENT		0x2000	//城市事件
#define	EVENTFLAG_WAREVENT		0x1000	//行会事件
#define	MAPCELLFLAG_NOPK		0x8000	//不能PK

struct tagMonItems;

typedef struct tag_monsterdesc //怪物描述
{
	tag_monsterdesc()
	{
		FILLSELF(0);
	}
	char szName[32];
	char szTitle[32];
	BYTE view;
	WORD wHp;
	BYTE mindc;
	BYTE maxdc;
	BYTE minac;
	BYTE maxac;
	BYTE minmac;
	BYTE maxmac;
	DWORD dwExp;
	DWORD dwDelay;
	BOOL bAuto;
	tagMonItems* pDownItems;
	DWORD nRace;
	DWORD nAiFlag;
	DWORD dwParam1;
	DWORD dwParam2;
}MONSTERDESC;

class CMonsterEx;
typedef struct tag_monstergen //怪物生成
{
	tag_monstergen()
	{
		FILLSELF(0);
	}
	char szName[32]; //怪类名
	int	mapid; //地图ID
	int	x; //X坐标
	int	y; //Y坐标
	int	range; //范围  1000 以上的数值是刷怪方式类型
	int	count; //数量
	DWORD dwRefreshDelay; //刷新间隔(分)
	int	curcount;
	int	errortime;
	DWORD dwLastRefreshTime;
	CServerTimer tmrRefresh;
	char* pszScriptPage;
	BOOL bStartWhenAllDead;
	xIndexPtrList<CMonsterEx> xMonsterList; // 怪物列表指针
}MONSTERGEN;

typedef struct tag_humandatadesc //人物数据描述
{
	tag_humandatadesc()
	{
		FILLSELF(0);
	}
	WORD wHp;
	WORD wMp;
	DWORD dwLevelupExp;
	BYTE minac;
	BYTE maxac;
	BYTE minmac;
	BYTE maxmac;

	BYTE mindc;
	BYTE maxdc;
	BYTE minmc;
	BYTE maxmc;

	BYTE minsc;
	BYTE maxsc;
	WORD bagweight;

	BYTE escape;		//躲避
	BYTE hitrate;		//命中
	BYTE bodyweight;
	BYTE handweight;

	BYTE magicrecover;
	BYTE hprecover;
	BYTE magescape;		//魔法躲避
	BYTE magicnicety;	//魔法命中
	BYTE poisonescape;	//中毒躲避
	BYTE poisonnicety;	//中毒命中

	WORD huoli;
	WORD wUnknown;
}HUMANDATADESC;

#define ROUND(f) (int)(f+0.5) //四舍五入
#define MAX(i,j) ((i)>(j)?(i):(j)) //取最大值

#define MAGICFLAG_NOEFFECT			0x80000000	//无效果
#define MAGICFLAG_ACTIVED			0x40000000	//开关式
#define MAGICFLAG_FORCED			0x20000000	//被动类型(0非被动1攻击2经验值)
#define MAGICFLAG_USEREDPOISON		0x10000000	//使用红毒
#define MAGICFLAG_USEGREENPOISON	0x08000000	//使用绿毒
#define MAGICFLAG_USECHARM			0x04000000	//使用符
#define MAGICFLAG_USESTRAWMAN		0x02000000	//使用稻草人(男)
#define MAGICFLAG_USESTRAWWOMAN		0x01000000	//使用稻草人(女)
#define MAGICFLAG_FORCED_EXP		0x00800000	//强制经验
#define MAGICFLAG_FORCED_ADDPROP	0x00400000	//强制加属性

enum e_skill_need_item //技能需要物品
{
	SNI_CHARM,		//符
	SNI_REDPOISON,	//红毒
	SNI_GREENPOISON,//绿毒
	SNI_STRAWMAN,	//稻草人(男)
	SNI_STRAWWOMAN,	//稻草人(女)
};

typedef struct tagMAGICCLASS //技能类
{
	tagMAGICCLASS()
	{
		FILLSELF(0);
	}
	char szName[20];
	DWORD id; // 序号
	BYTE btJob; // 职业
	BYTE btEffectType; // 效果类型
	BYTE btEffectValue; // 效果标志
	BYTE btNeedLv[4]; // 等级需求
	DWORD dwNeedExp[4]; // 经验需求
	WORD sSpell; // 施法值
	WORD sDefSpell; // 默认施法值
	WORD wDelay; // CD时间(毫秒)
	WORD wNeedMagic[3]; // 必须先学列表
	WORD wMutexMagic[3]; // 同类技能列表
	DWORD dwFlag; // 技能标识
	WORD wCharmCount; // 耗道符数
	WORD wRedPoisonCount; // 耗红毒点数
	WORD wGreenPoisonCount; // 耗绿毒点数
	WORD wStrawManCount; // 耗雄稻草人(男)点数
	WORD wStrawWomanCount; // 耗雌稻草人(女)点数
	char szSpecial[256]; // 附加参数
	//获取使用技能时消耗的魔法值
	int GetSpellPoint(int nLevel)const
	{
		return (sSpell * (nLevel + 1) >> 2) + sDefSpell;
	}
}MAGICCLASS;

#define USERMAGICFLAG_ACTIVED	0x80000000	//激活

typedef struct tagUSERMAGIC //使用技能
{
	tagUSERMAGIC()
	{
		FILLSELF(0);
		useTimer.Savetime();
	}
	MAGICDB	magic;
	MAGICCLASS* pClass;
	tagUSERMAGIC* pNext;
	//DWORD	dwLastUseTime;
	CServerTimer	useTimer;
	DWORD	dwFlag;
	int	nAddPower;
	DWORD	dwColor;
}USERMAGIC;

enum status_index //状态索引 0 - 31
{
	SI_FENGHUOLUN = 16,		// 风火轮
	SI_HUSHENZHENQI = 17,	// 护身真气
	SI_SPIRITWALL = 18,		// 灵魂墙
	SI_ITEMTRACED = 19,		// 夺宝追踪
	SI_BUBBLEDEFENCEUP = 20,// 魔法盾
	SI_DEFENCEUP = 21,		// 幽灵盾
	SI_MAGDEFENCEUP = 22,	// 神圣战甲
	SI_CLOAK = 23,			// 隐身
	SI_STRAWMAN = 25,		// 诅咒术(男)红咒-减魔法
	SI_PALSY = 26,			// 麻痹
	SI_STRAWWOMAN = 27,		// 诅咒术(女)蓝咒-降魔防
	SI_LOCKSPELL = 29,		// 魔法锁定
	SI_REDPOISON = 30,		// 红毒(减护甲)
	SI_GREENPOISON = 31,	// 绿毒(减血)
};

enum e_systemflag //系统标志索引 0 - 31
{
	SF_HIDED,				//隐藏
	SF_ATTACKSCITY,			//攻城
	SF_NODAMAGE,			//无损伤
	SF_SPECIALNAMECOLOR,	//特殊的名字颜色
	//	玩家标识
	SF_BAGLOADED,			//背包读取
	SF_BANKLOADED,			//仓库读取
	SF_PETBANKLOADED,		//宠物仓库读取
	SF_EQUIPMENTLOADED,		//身上装备读取
	SF_MAGICLOADED,			//魔法读取
	SF_COMMUNITYLOADED,		//人脉读取
	SF_WINDSHIELD,			//风影盾
	SF_STRONGSHIELD,		//金刚护体
	SF_SPECIALREPAIR,		//特殊修理
	SF_BANED,				//禁止发言
	SF_GODBLESS,			//护身-4种类型：神佑、神御、重击、秒杀
	SF_ALLOWSPECIALPOWER,	//天地合一
	SF_GAMEMASTER,			//管理员
	SF_SCROLLTEXTMODE,		//滚动文本显示
	SF_NOTICEMODE,			//系统消息模式
	SF_TRANSFORMED,			//变身
	SF_SLEEP,				//休眠
	SF_CRAZY,				//狂魔
	SF_FROZEN,				//冰冻
	//	怪物标识
	SF_BONE,				//骷髅
	SF_SOULWALL,			//灵魂墙
	SF_FONTCHANGED,			//字体改变
	SF_FAKETARGET,			//替身术
	SF_ALLOWALLY,			//允许联盟
};

class xStatus
{
	DWORD m_dwFlag;
	DWORD m_dwSetter[32];
	DWORD m_dwParam[32];
	DWORD m_dwLastTime[32];
	CServerTimer m_timer[32];
public:
	xStatus(){ FILLSELF(0); }
	DWORD GetFlag()const { return m_dwFlag; }
	VOID AddTime(int index, DWORD dwTime)
	{
		if (IsSeted(index))
			m_dwLastTime[index] = dwTime;
		m_timer[index].Savetime();
	}
	DWORD  GetTime(int index)const
	{
		if (IsSeted(index))
			return m_dwLastTime[index];
		return 0;
	}
	VOID DecTime(int index, DWORD dwTime)
	{
		if (IsSeted(index))
		{
			if (m_dwLastTime[index] > dwTime)
				m_dwLastTime[index] -= dwTime;
			else
				m_dwLastTime[index] = 1000;	//保留１秒钟
		}
	}
	DWORD GetParam(int index)const
	{
		if (!IsSeted(index))return 0;
		return m_dwParam[index];
	}
	VOID SetParam(int index, DWORD dwParam)
	{
		if (!IsSeted(index))return;
		m_dwParam[index] = dwParam;
	}
	DWORD GetSetter(int index)const
	{
		if (!IsSeted(index))return 0;
		return m_dwSetter[index];
	}
	VOID SetSetter(int index, DWORD dwSetter)
	{
		if (!IsSeted(index))return;
		m_dwSetter[index] = dwSetter;
	}
	VOID Clean() { FILLSELF(0); }
	BOOL SetStatus(int index, DWORD dwParam = 0, DWORD dwLastTime = 0)
	{
		if (index < 0 || index >= 32)return FALSE;
		DWORD f = (1 << index);
		if (f & m_dwFlag)
			return FALSE;
		m_dwFlag |= f;
		m_dwParam[index] = dwParam;
		m_dwLastTime[index] = dwLastTime;
		m_timer[index].Savetime();
		return TRUE;
	}
	BOOL ClrStatus(int index)
	{
		if (index < 0 || index >= 32)return FALSE;
		DWORD f = (1 << index);
		if (m_dwFlag & f)
			m_dwFlag ^= f;
		return TRUE;
	}
	BOOL IsSeted(int index)const
	{
		if (index < 0 || index >= 32)return FALSE;
		DWORD f = (1 << index);
		return ((m_dwFlag & f) != 0);
	}
	BOOL IsTimeOut(int index)
	{
		if (!IsSeted(index))return FALSE;
		if (m_dwLastTime[index] == 0xffffffff)return FALSE;
		return m_timer[index].IsTimeOut(m_dwLastTime[index]);
	}
	DWORD GetTimeOut(int index)
	{
		if (!IsSeted(index)) return 0;
		if (m_dwLastTime[index] == 0xffffffff)return 0;
		DWORD dwTimeOut = GetTimeToTime(m_timer[index].GetSavedTime(), timeGetTime());
		if (m_dwLastTime[index] > dwTimeOut)
			return (m_dwLastTime[index] - dwTimeOut);
		return 0;
	}
};

#pragma pack(push)
#pragma pack(1)
typedef struct tagGuildmgr // 行会管理-时长区
{
	tagGuildmgr()
	{
		FILLSELF(0);
	}
	char szTite[9];
	BYTE btCode;
	int nType;
}Guildmgr;

typedef struct tagGameTimeMgr // 充值游戏时长-时长区
{
	tagGameTimeMgr()
	{
		FILLSELF(0);
	}
	char szTite[12];
	BYTE btCode;
	BYTE btUnknow;
	int nValue[3];
}GameTimeMgr;

typedef struct tagExpBack // 每日经验-时长区
{
	tagExpBack()
	{
		FILLSELF(0);
	}
	char szTite[12];
	BYTE btCode;
	int nType;
}ExpBack;

typedef struct tagActivityScore2014 // 活动-时长区
{
	tagActivityScore2014()
	{
		FILLSELF(0);
	}
	char szTite[19];
	BYTE btCode;
}ActivityScore2014;

typedef struct tagMapJump // 服务器阶段-时长区
{
	tagMapJump()
	{
		FILLSELF(0);
	}
	char szTite[19];
	BYTE btCode;
	int nType;
	char szPage[21];
}MapJump;

typedef struct tagBossTJ // BOSS图解-时长区
{
	tagBossTJ()
	{
		FILLSELF(0);
	}
	char szTite[9];
	BYTE btCode;
	int nNum;
	char sName[21];
	int btUnKnow;
}BossTJ;

typedef struct tagClientKeyState // 自定义快捷键-时长区
{
	tagClientKeyState()
	{
		FILLSELF(0);
	}
	BYTE Key1;
	BYTE Key2;
	WORD unknow;
}ClientKeyState;

typedef struct tagHealthStatus // 健康状态-时长区
{
	tagHealthStatus()
	{
		FILLSELF(0);
	}
	DWORD dwId;
	int nHPChange;
	WORD wEffect;
	DWORD dwHP;
	DWORD dwMaxHP;
	DWORD dwMP;
	DWORD dwMaxMP;
}HealthStatus;

typedef struct tagPersonSetting // 个性化设置
{
	tagPersonSetting()
	{
		FILLSELF(0);
	}
	char szSetBan[20]; // 个性封号设置
	char szPersonSign[20]; // 个性化签名
	char szTempBan[60]; //
	
}PersonSetting;

typedef struct tagTreasureBoxItem // 宝箱开启物品
{
	tagTreasureBoxItem()
	{
		FILLSELF(0);
	}
	WORD Looks; // 物品外形
	BYTE btGoodType; // 物品类型 1普通, 2为贵重物品, 要求闪金光
	char szName[15]; // 物品名字
}TreasureBoxItem;
#pragma pack(pop)


#pragma pack(push)
#pragma pack(2)
struct tag_batfly_header //化身蝙蝠-头信息
{
	tag_batfly_header()
	{
		FILLSELF(0);
	}
	DWORD dwFeature;
	DWORD dwStatus;
	WORD wCurHp;
	WORD wMaxHp;
	WORD wX;
	WORD wY;
	BYTE bt1;
	BYTE btColor;
	BYTE btSm;
	BYTE btSm2;
};

struct tag_batfly //飞行移动技能-信息
{
	tag_batfly_header header;
	char szName[20] = { 0 };
};
#pragma pack(pop)

#define DF_NOSAFE			0x80000000	//不受伤害
#define DF_NOSTRUCKACTION	0x40000000	//不受击打动作
#define DF_SUPERHIT			0x20000000	//超级命中
#define DF_TARGETEFFECT		0x10000000	//破盾 破击 风火轮等新技能的击中后魔法效果

#define TE_POJI			6	//破击
#define TE_PODUN		7	//破盾
#define	TE_FIREWHEEL	3	//火轮
#define	TE_FIRE			1	//火墙

enum damage_type //伤害类型
{
	DT_PHYSICS,	//普通攻击
	DT_MAGIC,	//技能攻击
	DT_DIRECT,	//飞行攻击, 不计算防御
	DT_CUTTREE, //砍树攻击
	//	DT_DIRECT_NOSAFE = DF_DIRECT | 
	//	DT_DIRECT_NOSAFE_NOSTRUCKACTION,
};

enum damage_ReType //伤害返回的类型
{
	DRT_NONE,	//没有成功
	DRT_POJI,	//破甲成功
	DRT_PODUN,	//破盾成功
};

enum pet_type //宠物类型
{
	PT_SKELETON,		//骷髅
	PT_DOG,				//麒麟
	PT_SUPERSKELETON,	//强化骷髅
	PT_MAX,
};

enum dbserver_index //数据库服务器
{
	DI_CHARINFO,	//角色信息
	DI_ITEMDB,		//物品信息
};

enum money_type //金钱类型
{
	MT_GOLD,	//金币
	MT_YUANBAO,	//元宝
};

typedef struct start_point //玩家出生点
{
	char szName[32];
	WORD mapid;
	WORD x;
	WORD y;
	WORD range;
	bool bFighterBorn;
	bool bMagicianBorn;
	bool bTaoshiBorn;
	bool bTemp;
	DWORD index;
}START_POINT;

enum e_visible_event //可见事件
{
	VE_ZOMBIHOLE = 0x1,	//僵尸洞
    VE_STONEMINE = 0x2,	//矿石
	VE_MINESTONE = 0x3,	//堆石 矿石被挖
	VE_SOULWALL = 0x4,	//光锥
	VE_FIREWALL = 0x5,	//火墙
	VE_SCULPEICE = 0x6,	//破壳
	VE_BLUEFIRE = 0x15, //魔炎-蓝色火焰
	VE_FIRERAIN = 0X16,	//火云
};

#define SMALLBAG_SLOT 46	//小背包格子数
#define BIGBAG_SLOT	66		//大背包格子数
#define TYPEFLAG_TARGETOBJECT ((DWORD)((1<<OBJ_PLAYER) | (1<<OBJ_MONSTER)) | (1<<OBJ_PET))	//可被攻击的对象
#define DISTANCE( x1, y1, x2, y2 ) (UINT)(max( abs((int)x1 - x2 ), abs( (int)y1 - y2 ) )) // 切比雪夫距离

typedef struct tagMonsterBase // 怪物基础信息
{
	char szClassName[20]; // 类名字
	char szViewName[20]; // 显示名字
	BYTE btRace; // 族系
	WORD btImage; // 外观-普通
	BYTE btLevel; // 等级
	BYTE btNameColor; // 颜色
	BYTE raceimg; // Raceimg
	DWORD dwFeature; // 特性值-由武器值和衣服、衣服颜色、头发、头发颜色 、性别通过位运算组合后的值
}MonsterBase;

enum monster_race // 怪物种族
{
	MR_ANIMAL,		//动物
	MR_UNDEAD,		//不死系
	MR_VEGETABLE,	//食人花
	MR_HUMAN,		//人形
	MR_BOSS,		//BOSS
};

typedef struct tagMonsterProp // 怪物属性
{
	WORD hp; // HP
	WORD mp; // MP
	BYTE hit; // 命中
	BYTE speed; // 躲避
	BYTE ac1; // 防御下限
	BYTE ac2; // 防御上限
	BYTE dc1; // 攻击下限
	BYTE dc2; // 攻击上限
	BYTE mac1; // 魔御下限
	BYTE mac2; // 魔御上限
	BYTE mc1; // 魔攻下限
	BYTE mc2; // 魔攻上限
	WORD aidelay; // AI时间
	WORD walkdelay; // 行走等待
	DWORD exp; // 经验
	WORD recoverhp; // 回血点
	WORD recoverhptime; // 回血时间
	WORD recovermp; // 回蓝点
	WORD recovermptime; // 回蓝时间
}MonsterProp;

typedef struct tagSpecialProp // 特殊属性
{
	DWORD pFlag;			// 特殊属性标识
	BYTE CallRate;			// 召唤系数
	BYTE AntSoulWall;		// 抗灵魂墙
	BYTE AntTrouble;		// 抗困魔咒
	BYTE MonsterRate;		// 生怪几率
	char MonsterName[64];	// 生小怪的名字列表, 例如：逆魔|逆魔弓箭手, 用|分割
}SpecialProp;

enum sprop_flag //特殊属性标识
{
	SF_AUTOATTACK = 0x1,			//主动攻击
	SF_CANBECALLED = 0x2,			//不可召唤
	SF_ANTFAKETARGET = 0x4,			//反替身
	SF_ANTSOULWALL = 0x8,			//抗灵魂墙
	SF_ANTTROUBLE = 0x10,			//抗困魔
	SF_BODYITEM = 0x20,				//挖尸体
	SF_HURTESCAPE = 0x40,			//受伤逃跑
	SF_AUTOHIDE = 0x80,				//自动隐藏
	SF_HOLYWORD = 0x100,			//圣言有效
	SF_NOBODY = 0x200,				//没有尸体
	SF_SHIN = 0x400,				//瞬移
	SF_LOCKTARGET = 0X800,			//固定仇恨
	SF_LOCKDIRECTION = 0x1000,		//固定朝向
	SF_BODYNOTDISAPPEAR = 0x2000,	//尸体不消失
	SF_NOREATTACK = 0x4000,			//不反击
	SF_ALLOWRUN = 0x8000,			//允许跑步
	SF_PICKUPITEM = 0x10000,		//自动拾物品
	SF_CANBECUT = 0x20000,			//可用变骨头
	SF_SHARETARGET = 0x40000,		//同类共享目标
	SF_ANTTRANSFORM = 0x80000,		//抗变身
	SF_ANTPALSY = 0x100000,			//抗麻痹
	SF_MINION = 0x200000,			//受伤生小怪
	SF_SHOW = 0x400000,				//显身动画
	SF_ZOMBIHOLE = 0x800000,		//僵尸洞事件
	SF_SCULPEICE = 0x1000000,		//破壳事件
	SF_GROUPSHARETARGET = 0x2000000,//同组共享目标
};

typedef struct tagAttackSituation //攻击条件
{
	int	Situation;
	int	Param;
}AttackSituation;

enum attack_situation //变身条件
{
	AS_NOSITUATION,		//无条件
	AS_HPBEYOUND,		//HP大于
	AS_HPBELOW,			//HP小于
	AS_AROUNDHUMAN,		//周围人数大于
	AS_MPBELOW,			//MP小于
	AS_MPBEYOUND,		//MP大于
	AS_EXPBEYOUNDEQUAL,	//EXP大于等于
	AS_IDLETIMEBEYOUND,	//空闲时间超过多少秒
	AS_CHANGEOWNER,		//被召唤
	AS_KILLCOUNTABOVE,	//打死怪物数量超过
	AS_GOTTARGET,		//获得目标
	AS_DEATH,			//死亡
};

typedef struct tagAiAttack //攻击设置
{
	BYTE AttackStyle;	//攻击方式
	BYTE DamageStyle;	//伤害方式
	BYTE DamageType;	//伤害类型
	BYTE AppendEffect;	//附加效果
	BYTE AppendRate;	//附加几率
	BYTE AttackDistance;//攻击距离
	BYTE DamageRange;	//伤害范围
	WORD Delay;			//等待时间
	WORD CostHp;		//消耗HP
	WORD CostMp;		//消耗MP
	WORD Action;		//动作
	WORD AppendTime;	//附加时间
	WORD AppendType;	//附加效果子类型
}AiAttack;

enum ai_attack_style //攻击方式
{
	AAS_NOATTACK,		//不攻击
	AAS_DIRECTATTACK,	//直接攻击
	AAS_THROWATTACK,	//投掷攻击
	AAS_MAGICATTACK,	//施法攻击
	AAS_MOYAN,			//魔眼攻击
	AAS_BLOODKING,		//铁血魔王
};

enum ai_damage_style //伤害方式
{
	ADS_RANGE,	//范围型
	ADS_LINE,	//直线型
	ADS_CROSS,	//十字型
	ADS_XSHAPE,	//X型
	ADS_ALLDIR,	//米字型
	ADS_SECTOR,	//扇形
};

enum ai_damage_type //伤害类型
{
	ADT_PHYSICS,	//物理攻击
	ADT_MAGIC,		//魔法攻击
	ADT_DIRECT,		//直接攻击
};

enum ai_attack_append //附加效果
{
	AAA_NONE,		//无
	AAA_REDPOISON,	//红毒
	AAA_GREENPOISON,//绿毒
	AAA_STONE,		//石化
	AAA_STRAWMAN,	//红咒
	AAA_STRAWWOMAN,	//蓝咒
	AAA_FROZEN,		//冰冻
	AAA_MABI,		//麻痹
	AAA_PUSHED,		//冲撞
	AAA_LIONPUSHED,	//突斩
	AAA_FIREWALL,	//火墙
};

typedef struct tagAiSetting //AI设置
{
	BYTE MoveStyle;		//移动
	BYTE DieStyle;		//死亡
	BYTE TargetSelect;	//目标选择
	BYTE TargetFlag;	//目标状态
	BYTE ViewDistance;	//视野范围
	BYTE CoolEyes;		//反隐范围
	BYTE EscapeDistance;//逃跑距离
	BYTE LockDir;		//固定方向
}AiSetting;

enum ai_move_style //移动方式
{
	AMS_STATIC,			//静止不动
	AMS_FOLLOW,			//跟踪目标
	AMS_ESCAPE,			//逃跑
	AMS_KEEPDISTANCE,	//保持距离
	AMS_KEEPLINE,		//保持直线
	AMS_STUPID,			//傻瓜乱跑
	AMS_BIANZOUBIANDA,	//边走边打
	AMS_BACKHOME,		//归位移动
};

enum ai_die_style //死亡方式
{
	ADS_NORMAL,		//普通
	ADS_BOOM,		//爆炸
};

enum ai_target_select //目标选择
{
	ATS_DISTANCE,	//距离优先
	ATS_LEVEL,		//级低优先
	ATS_HP,			//血少优先
	ATS_HIT,		//仇恨优先
	ATS_DAMAGE,		//受伤害优先
};

enum ai_target_flag //目标状态
{
	ATF_HUMAN = 1,			//人类
	ATF_MONSTER = 2,		//主攻怪物
	ATF_ANIMAL = 4,			//被动怪物
	ATF_CRIMER = 8,			//罪犯
	//	ATF_BOSS = 16,
	ATF_ATTACKSANDCITY = 16,//攻城者
	ATF_PETS = 32,			//宠物
};

enum ai_pet_type //宠物类型
{
	APT_DEF,	//普通类
	APT_CALL,	//召唤类
	APT_RIDE,	//骑乘类
	APT_PET,	//宠物类
	APT_TISHEN, //替身类
};

enum ai_stop_at //停靠位置
{
	ASA_BACK,		//后方
	ASA_BACKLEFT,	//左后
	ASA_LEFT,		//左边
	ASA_FRONTLEFT,	//左前
	ASA_FRONT,		//前方
	ASA_FRONTRIGHT,	//右前
	ASA_RIGHT,		//右边
	ASA_BACKRIGHT,	//右后
};

typedef struct tagPetSetting //宠物设置
{
	BYTE Type;		//宠物类型
	BYTE StopAt;	//停靠在玩家身边的位置
}PetSetting;

typedef struct tagChangeInto //变身
{
	BOOL bEnabled;				//是否启动
	BOOL bAnim;					//是否显示动画
	AttackSituation situation1;	//变身条件1
	AttackSituation situation2;	//变身条件2
	int	AppendEffect;			//附加效果
	char szChangeInto[20];		//变身后名字
}ChangeInto;

enum e_changeappend //变身附加效果
{
	ACA_FULLHPMP = 1,		//满血满蓝
	ACA_CLEANKILLCOUNT = 2,	//清除杀怪数量
	ACA_CLEANEXP = 4,		//清除经验
	ACA_CANCLETARGET = 8,	//清除目标
};

typedef struct tagMonsterClass //怪物类
{
	tagMonsterClass()
	{
		FILLSELF(0);
	}
	MonsterBase base;
	MonsterProp prop;
	SpecialProp sprop;
	AiSetting aiset;
	PetSetting petset;
	AiAttack attackdesc;
	ChangeInto changeinto[3];
	tagMonItems* pDownItems;
	UINT nCount;
	//StringCacheNode * pBornScriptPage;
	//StringCacheNode * pHurtScriptPage;
	//StringCacheNode * pScriptPage;
	//StringCacheNode * pBornScriptPage;
	//StringCacheNode * pBornScriptPage;
	//StringCacheNode * pBornScriptPage;
	//char	szBornScriptPage[256];
	char* pszBornScript;
	char* pszGotTargetScript;
	char* pszKillTargetScript;
	char* pszHurtScript;
	char* pszDeathScript;
}MonsterClass;

#define LINEATTACK_XSTYLE 0x80000000	//保持直线攻击-X型
#define LINEATTACK_CROSS 0x40000000		//保持直线攻击-十字型

//检查是否自己面向目标的方向
inline BOOL	CheckIsNextPos(int x, int y, int nx, int ny, int step)
{
	//x -= nx;
	//y -= ny;
	x = abs(x - nx);
	y = abs(y - ny);

	if (x == 0 && y == step)return TRUE;
	if (y == 0 && x == step)return TRUE;
	if (x == step && x == y)return TRUE;
	return FALSE;
}

//获取自己面向目标的方向
inline int GetDirectionTo(int x, int y, int xt, int yt)
{
	x -= xt;
	y -= yt;

	if (x > 0)
	{
		if (y < 0)return 5;
		if (y == 0)return 6;
		return 7;
	}
	if (x < 0)
	{
		if (y < 0)return 3;
		if (y == 0)return 2;
		return 1;
	}
	if (y < 0)
		return 4;
	return 0;
}

typedef struct tagThrowdesc //投掷设置
{
	WORD x;
	WORD y;
	DWORD dwTargetId;
}ThrowDesc;

enum e_map_flag //地图标志, 可扩展到32个
{
	MF_SABUKPALACE,		//沙城
	MF_FIGHTMAP,		//战斗地图
	MF_NORANDOMMOVE,	//禁止随机移动
	MF_NORECONNECT,		//禁止重进入
	MF_RIDEHORSE,		//骑乘
	MF_LEVELABOVE,		//等级高于
	MF_LEVELBELOW,		//等级低于
	MF_LIMITJOB,		//限制职业
	MF_PKPOINTABOVE,	//PK点高于
	MF_PKPOINTBELOW,	//PK点低于
	MF_NOESCAPE,		//禁止逃跑
	MF_NOHOME,			//禁止回家
	MF_MINE,			//矿洞
	MF_WEATHER,			//天气
	MF_DAY,				//白天
	MF_NIGHT,			//晚上
	MF_NOGROUPMOVE,		//禁止组队移动
	MF_NOSANDCITYHOME,	//禁止沙城回城
	MF_NODMOVE,			//禁止特殊装备移动
	MF_NOFLASHMOVE,		//禁止闪现移动
	MF_NOUSEITEM,		//禁止使用物品
	MF_NOCMD,			//禁止命令
	MF_NOSKILL,			//禁止使用技能
	MF_USERDEFINE1,		//用户定义1
	MF_USERDEFINE2,		//用户定义2
	MF_USERDEFINE3,		//用户定义3
	MF_USERDEFINE4,		//用户定义4
	MF_MAX,
};

static const char* g_map_flag[MF_MAX] =
{
	"SABUKPALACE",
	"FIGHTMAP",
	"NORANDOMMOVE",
	"NORECONNECT",
	"RIDEHORSE",
	"LEVELABOVE",
	"LEVELBELOW",
	"LIMITJOB",
	"PKPOINTABOVE",
	"PKPOINTBELOW",
	"NOESCAPE",
	"NOHOME",
	"MINE",
	"WEATHER",
	"DAY",
	"NIGHT",
	"NOGROUPMOVE",
	"SANDCITYHOME",
	"NODMOVE",
	"NOFLASHMOVE",
	"NOUSEITEM",
	"NOCMD",
	"NOSKILL",
	"USERDEFINE1",
	"USERDEFINE2",
	"USERDEFINE3",
	"USERDEFINE4",
};

//从字符串获取地图标志
inline e_map_flag GetMapFlagFromString(const char* pszString)
{
	for (int i = 0; i < MF_MAX; i++)
	{
		if (_stricmp(pszString, g_map_flag[i]) == 0)
			return (e_map_flag)i;
	}
	return MF_MAX;
}

class CGuildEx;
typedef struct tagAttackSabukRequest //沙城城攻城请求
{
	tagAttackSabukRequest()
	{
		FILLSELF(0);
	}
	char guild[32];
	SYSTEMTIME	stTime;
	CGuildEx* pGuild;
}AttackSabukRequest;

typedef struct tagOneDayAttackRequest //一天一次的攻城请求
{
	tagOneDayAttackRequest()
	{
		FILLSELF(0);
	}
	AttackSabukRequest attackrequest[20];
	int	m_iAttackRequestCount;
}OneDayAttackRequest;

class CTimeEventObject //时间事件对象
{
public:
	virtual VOID OnSecondChange(CSystemTime& curTime) {}
	virtual VOID OnMinuteChange(CSystemTime& curTime) {}
	virtual VOID OnHourChange(CSystemTime& curTime) {}
	virtual VOID OnDayChange(CSystemTime& curTime) {}
	virtual VOID OnMonthChange(CSystemTime& curTime) {}
	virtual VOID OnYearChange(CSystemTime& curTime) {}
};

enum e_item_update_type //物品更新类型
{
	IUT_UPDATEALL,	//更新所有
	IUT_UPDATEDURA,	//更新耐久
	IUT_UPDATENAME,	//更新名字
};

struct stLionBack //狮子吼
{
	DWORD dwFeature;
	DWORD dwStatus;
	WORD wEndFlag;
};

typedef struct tagPrivateShopItemCache //摆摊物品缓存
{
	ITEM* pItem;
	money_type pricetype;
	DWORD dwPrice;
}PrivateShopItemCache;

enum friend_error //好友错误
{
	FE_NOERROR,				//没有错误
	FE_ACCEPT_FULL,			//对方好友已满
	FE_ADD_OFFONLINE,		//对方不在线
	FE_ADD_FULL,			//自己好友已满
	FE_ADD_REFUSED,			//对方拒绝添加
	FE_ADD_ALREADYFRIEND,	//对方已经是好友
	FE_ADD_ERROR,			//添加失败
	FE_DEL_ERROR,			//删除失败
	FE_ADD_LOWLEVEL,		//对方等级太低
	FE_ADD_TARGETLOWLEVEL,	//自己等级太低
	FE_ADD_TARGETNOTALOW,	//对方不是好友
};

#define	ITEMMODIFY_DURACHANGED	0x80	//耐久度改变
#define	ITEMMODIFY_COLORCHANGED	0x40	//颜色改变
#define	ITEMMODIFY_FORGED		0x20	//锻造

enum weapon_prop_index //武器属性
{
	WPI_LUCKY,	//幸运
	WPI_HITRATE,//命中
	WPI_DAWN,	//诅咒
	WPI_UNKNOWN,//未知
	WPI_DC1,	//攻击下限
	WPI_DC2,	//攻击上限
	WPI_MC1,	//魔法下限
	WPI_MC2,	//魔法上限
	WPI_SC1,	//道术下限
	WPI_SC2,	//道术上限
};

enum normal_prop_index //普通属性
{
	NPI_AC1,	//防御下限
	NPI_AC2,	//防御上限
	NPI_MAC1,	//魔防下限
	NPI_MAC2,	//魔防上限
	NPI_DC1,	//攻击下限
	NPI_DC2,	//攻击上限
	NPI_MC1,	//魔法下限
	NPI_MC2,	//魔法上限
	NPI_SC1,	//道术下限
	NPI_SC2,	//道术上限
};
//ac2p = PI_MAGESCAPE,	//	魔法躲避+300%	maxdef*10
//mac2p = PI_LUCKY,		//	幸运+50		maxmagdef
//mac1p = PI_DAWN;		//	诅咒+40		minmagdef

enum special19_prop_index //19特殊属性
{
	S19_UNKNOWN,
	S19_MAGESCAPE,
	S19_DAWN,
	S19_LUCKY,
	S19_DC1,
	S19_DC2,
	S19_MC1,
	S19_MC2,
	S19_SC1,
	S19_SC2,
};
//ac2p = PI_HITRATE,		//	命中+30		maxdef
//mac2p = PI_ESCAPE;		//	躲避+50		maxmagdef

enum special20_prop_index //20特殊属性
{
	S20_UNKNOWN1,
	S20_HITRATE,
	S20_UNKNOWN2,
	S20_ESCAPE,
	S20_DC1,
	S20_DC2,
	S20_MC1,
	S20_MC2,
	S20_SC1,
	S20_SC2,
};
//ac2p = PI_HPRECOVER,		//体力恢复+30	maxdef
//ac1p = PI_ATTACKSPEED,		//攻击速度+20	mindef
//mac2p = PI_MPRECOVER,		//魔法恢复+50	maxmagdef
//mac1p = PI_ATTACKSPEED * -1;//攻击速度-40	minmagdef

enum special21_prop_index //21特殊属性
{
	S21_ATTACKSPEED,
	S21_HPRECOVER,
	S21_ATTACKSPEEDDEC,
	S21_MPRECOVER,
	S21_DC1,
	S21_DC2,
	S21_MC1,
	S21_MC2,
	S21_SC1,
	S21_SC2,
};
//ac2p = PI_POISONESCAPE,			//毒物躲避+300%	maxdef*10
//ac1p = PI_ATTACKSPEED,			//攻击速度+20	mindef
//mac2p = PI_POISONRECOVER,		//中毒恢复+500%	maxmagdef*10
//mac1p = PI_ATTACKSPEED * -1;	//攻击速度-40	minmagdef

enum special23_prop_index //23特殊属性
{
	S23_ATTACKSPEED,
	S23_POISONESCAPE,
	S23_ATTACKSPEEDDEC,
	S23_POISONRECOVER,
	S23_DC1,
	S23_DC2,
	S23_MC1,
	S23_MC2,
	S23_SC1,
	S23_SC2,
};

//#define	USE_FREE_MEMORY

enum special_godbless //附身
{
	SG_IMMDIE = 1,		//秒杀
	SG_BIGHIT = 2,		//重击
	SG_SHIELD = 3,		//神御
	SG_DOUBLEEXP = 4,	//神佑
};

typedef struct tagTopCharInfo //玩家排行榜信息
{
	tagTopCharInfo()
	{
		FILLSELF(0);
	}
	char szName[32];
	char szGuild[64];
	BYTE btSex;
	BYTE btClass;
	WORD wLevel;
	BYTE btPerCent;
	DWORD dwDBId;
	SYSTEMTIME stRanking;
}TopCharInfo;

enum special_equipment_func //特殊装备函数
{
	SEF_TELEPORT,		//传送
	SEF_TELEPORTGROUP,	//组传送-记忆套
	SEF_CLOAK,			//隐身
	SEF_PROTECT,		//护身
	SEF_OVERLOAD,		//身体负重扩大到两倍
	SEF_RELIVE,			//复活
	SEF_TRAINSKILL,		//技能翻倍经验
	SEF_SEARCH,			//寻宝
	SEF_HEALHALO,		//幻彩
	SEF_MIRAGE,			//幻影
	SEF_BAGOVERLOAD,	//背包承受重量扩大到两倍
	SEF_BIGHIT,			//重击
	SEF_SHIELD,			//神御
	SEF_DEFENCEUP,		//防御提升
	SEF_IMMDIE,			//秒杀
	SEF_PALSY,			//麻痹
	SEF_SHIXUEALL,		//嗜血套
	SEF_SHIXUEHALF,
	SEF_SHIXUE01,
	SEF_SHIXUE02,
	SEF_SHIXUE03,
	SEF_SHIXUE04,
	SEF_SHIXUE05,
	SEF_MOXUEALL,		//魔血套
	SEF_MOXUEHALF,
	SEF_MOXUE01,
	SEF_MOXUE02,
	SEF_MOXUE03,
	SEF_MOXUE04,
	SEF_MOXUE05,
	SEF_SHENWUALL,		//神武全套
	SEF_SHENWUHALF,		//神武半套
	SEF_SHENWU01,
	SEF_SHENWU02,
	SEF_SHENWU03,
	SEF_SHENWU04,
	SEF_SHENWU05,
	SEF_SHENWU06,
	SEF_SHENWU07,
	SEF_SHENWU08,
	SEF_SHENWU09,
	SEF_SHENWU10,
	SEF_HUANMOALL,		//幻魔全套
	SEF_HUANMOHALF,		//幻魔半套
	SEF_HUANMO01,
	SEF_HUANMO02,
	SEF_HUANMO03,
	SEF_HUANMO04,
	SEF_HUANMO05,
	SEF_HUANMO06,
	SEF_HUANMO07,
	SEF_HUANMO08,
	SEF_HUANMO09,
	SEF_HUANMO10,
	SEF_TIANXUANALL,	//天玄全套
	SEF_TIANXUANHALF,	//天玄半套
	SEF_MAX,
};

static const char* g_special_equipment_table[SEF_MAX] =
{
	"Teleport",
	"TeleportGroup",
	"Cloak",
	"Protect",
	"OverLoad",
	"Relive",
	"TrainSkill",
	"Search",
	"Healhalo",
	"Mirage",
	"BagOverLoad",
	"BigHit",
	"Shield",
	"Defenceup",
	"ImmDie",
	"Palsy",
	"ShiXueAll",
	"ShiXueHalf",
	"ShiXue01",
	"ShiXue02",
	"ShiXue03",
	"ShiXue04",
	"ShiXue05",
	"MoXueAll",
	"MoXueHalf",
	"MoXue01",
	"MoXue02",
	"MoXue03",
	"MoXue04",
	"MoXue05",
	"ShenWuAll",
	"ShenWuHalf",
	"ShenWu01",
	"ShenWu02",
	"ShenWu03",
	"ShenWu04",
	"ShenWu05",
	"ShenWu06",
	"ShenWu07",
	"ShenWu08",
	"ShenWu09",
	"ShenWu10",
	"HuanMoAll",
	"HuanMoHalf",
	"HuanMo01",
	"HuanMo02",
	"HuanMo03",
	"HuanMo04",
	"HuanMo05",
	"HuanMo06",
	"HuanMo07",
	"HuanMo08",
	"HuanMo09",
	"HuanMo10",
	"TianXuanAll",
	"TianXuanHalf",
};

//通过名字获取特殊装备函数
inline special_equipment_func get_special_equipment_func_by_name(const char* pszName)
{
	for (int f = 0; f < SEF_MAX; f++)
	{
		if (_stricmp(g_special_equipment_table[f], pszName) == 0)return (special_equipment_func)f;
	}
	return SEF_MAX;
}

typedef struct tagSpecialEquipment //特殊装备
{
	tagSpecialEquipment()
	{
		FILLSELF(0);
	}
	special_equipment_func func;
	int	stdmode; //分类
	int	image; //外观
	int	pos; //位置
	int	group; //分组
}SpecialEquipment;

typedef struct tagSpecialEquipmentFunction //特殊装备函数
{
	tagSpecialEquipmentFunction()
	{
		FILLSELF(0);
	}
	char* pszSpecial;
	int paramcount;
	DWORD* pParams;
	int index;
}SpecialEquipmentFunction;

typedef struct tagGM_COMMAND //GM命令
{
	char* pszCmd;
	DWORD wFlag;
	DWORD btFitLevel;
}GM_COMMAND;

typedef struct tagITEMCLASS //物品类
{
	tagITEMCLASS()
	{
		FILLSELF(0);
	}
	char szFullName[32];
	char szName[32];
	BYTE stdMode;
	BYTE btShape;
	WORD wImage;
	BYTE btWeight;
	char specialpower;
	BYTE ac[2];
	BYTE mac[2];
	BYTE dc[2];
	BYTE mc[2];
	BYTE sc[2];
	BYTE needtype;
	BYTE needlevel;
	WORD maxdura;
	WORD duratimes;
	DWORD nPrice;
	BYTE btMagicNicety;
	BYTE btPoisonNicety;

	// 优化：使用字符串池ID存储脚本路径
	WORD wPageId;              // szPage的ID, 从字符串池获取
	WORD wDropPageId;          // szDropPage的ID, 从字符串池获取
	WORD wPickupPageId;        // szPickupPage的ID, 从字符串池获取
	DWORD dwDropPageDelay;
	DWORD dwDropPageExecuteTimes;
	WORD wLimit;
}ITEMCLASS;

typedef struct tagMPetInfo //宠物升级经验信息
{
	tagMPetInfo()
	{
		FILLSELF(0);
	}
	int	lv0;
	int	lv1;
	int	lv2;
	int	lv3;
	int	lv4;
	int	lv5;
	int	lv6;
	int	lv7;
}m_PetInfo;

static int g_iErrorCode = 0;
#define	CATCHERROR() (g_iErrorCode)	//获取错误码
#define	SETERROR( code ) g_iErrorCode = (int)(code)	//设置错误码

//通过名字获取聊天频道
inline e_chatchannel GetChannelFromString(const char* pszString)
{
	for (int i = 0; i < CCH_MAX; i++)
	{
		if (strcmp(g_pChatChannelDesc[i], pszString) == 0)
			return (e_chatchannel)i;
	}
	return CCH_MAX;
}

enum actiontype //动作类型
{
	AT_STAND,		//站立
	AT_WALK,		//走动
	AT_RUN,			//跑动
	AT_BACK,		//后退
	AT_TURN,		//转身
	AT_FLY,			//飞地图
	AT_ATTACK,		//攻击
	AT_GETMEAL,		//挖肉
	AT_BEATTACK,	//被攻击
	AT_SPELLSKILL,	//施放技能
	AT_RUSH,		//冲锋
	AT_AIACTION,	//AI动作
	AT_PRIVATESHOP,	//摆摊
	AT_SPECIALHIT,	//战士攻击
	AT_SHOW,		//出现动作
	AT_HIDED,		//隐藏动作
	AT_MAX,
};

//通过名字获取动作类型
inline actiontype GetActionType(const char* pszName)
{
	if (_stricmp(pszName, "stand") == 0) return AT_STAND;
	if (_stricmp(pszName, "walk") == 0) return AT_WALK;
	if (_stricmp(pszName, "run") == 0) return AT_RUN;
    if (_stricmp(pszName, "back") == 0) return AT_BACK;
    if (_stricmp(pszName, "turn") == 0) return AT_TURN;
	if (_stricmp(pszName, "attack") == 0) return AT_ATTACK;
    if (_stricmp(pszName, "getmeal") == 0) return AT_GETMEAL;
	if (_stricmp(pszName, "beattack") == 0) return AT_BEATTACK;
	if (_stricmp(pszName, "spellskill") == 0) return AT_SPELLSKILL;
    if (_stricmp(pszName, "rush") == 0) return AT_RUSH;
    if (_stricmp(pszName, "aiaction") == 0) return AT_AIACTION;
	if (_stricmp(pszName, "privateshop") == 0) return AT_PRIVATESHOP;
    if (_stricmp(pszName, "specialhit") == 0) return AT_SPECIALHIT;
	return AT_MAX;
}

enum item_limit //物品限制
{
	IL_NODROP,			//禁止扔掉
	IL_NODEADDROP,		//死亡不掉
	IL_DEADDROP,		//死亡必掉
	IL_NOEXCHANGE,		//禁止交易
	IL_NOSELL,			//禁止卖给NPC
	IL_NOPRIVATESHOP,	//禁止放入摆摊物品
	IL_NOREPAIR,		//禁止修理
	IL_NOTAKEOFF,		//禁止卸下来
	IL_NOSTORAGE,		//禁止存仓库
	IL_NOUPDATETODB,	//禁止更新到数据库
	IL_TRACEDITEM,		//夺宝物品
	IL_NOUPGRADE,		//禁止升级物品
	IL_DEADDELETE,		//死亡销毁物品
	IL_EQUDEADDELETE,	//死亡销毁装备
	IL_MAX,
};

#define	COMPANDRETURNITEMLIMIT( name ) if( _stricmp( pszLimit, #name ) == 0 )return IL_##name;	//通过名字获取物品限制

//通过名字获取物品限制
inline item_limit GetItemLimit(const char* pszLimit)
{
	COMPANDRETURNITEMLIMIT(NODROP);			//不能扔掉
	COMPANDRETURNITEMLIMIT(NODEADDROP);		//死亡不掉
	COMPANDRETURNITEMLIMIT(DEADDROP);		//死亡必掉
	COMPANDRETURNITEMLIMIT(NOEXCHANGE);		//无法交易
	COMPANDRETURNITEMLIMIT(NOSELL);			//无法卖给ＮＰＣ
	COMPANDRETURNITEMLIMIT(NOPRIVATESHOP);	//无法放入摆摊物品
	COMPANDRETURNITEMLIMIT(NOREPAIR);		//不能修理
	COMPANDRETURNITEMLIMIT(NOTAKEOFF);		//不能卸下来
	COMPANDRETURNITEMLIMIT(NOSTORAGE);		//不能存仓库
	COMPANDRETURNITEMLIMIT(NOUPDATETODB);	//不能更新到数据库
	COMPANDRETURNITEMLIMIT(TRACEDITEM);		//跟踪该物品
	COMPANDRETURNITEMLIMIT(NOUPGRADE);		//不能升级武器
	COMPANDRETURNITEMLIMIT(DEADDELETE);		//死亡销毁
	COMPANDRETURNITEMLIMIT(EQUDEADDELETE);	//装备死亡销毁
	return IL_MAX;
}

typedef struct tagAbilityShell
{
	tagAbilityShell()
	{
		FILLSELF(0);
	}
	//SHORT	sProp[PI_PROP_COUNT];
	WORD wAddHp;
	WORD wAddMp;
	WORD wAddAc;
	WORD wAddMac;

	WORD wMaxHp;
	WORD wReqSkillLevel;
	DWORD dwLevelupExp;
}AbilityShell;

typedef struct tagAbilityShellRef
{
	tagAbilityShellRef()
	{
		FILLSELF(0);
	}
	AbilityShell* pShell;
	WORD wCurHp;//这个用来表示当前持久
	WORD wMaxdura;//这个用来表示最大持久
	WORD wLevel;
	DWORD dwCurExp;
	WORD wSkillId;
}AbilityShellRef;

typedef struct tagWeather
{
	tagWeather()
	{
		FILLSELF(0);
	}
	DWORD dwBGColor;
	DWORD dwWeatherColor;
	WORD wWeatherIndex;
	WORD wFlag;
}Weather;