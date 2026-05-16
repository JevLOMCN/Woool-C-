#pragma	once
#include "windows.h"
#include "time.h"
///////////////////////////////////////账号消息//////////////////////////////////////////////

// 注册账号
//cmd = 2002
#define CM_REGISTERACCOUNT		0x7d2
// 创建ID成功
//cmd = 504
#define	SM_REGISTERACCOUNTOK	0x1f8
// 创建ID失败
//cmd = 505
#define SM_REGISTERACCOUNTFAIL	0x1f9
// 检查账号是否存在
//cmd = 118
//dwFlag = bool is poptang
//data = account
#define CM_CHECKACCOUNTEXIST	0x76
// 找回密码成功 508
//cmd = 508
#define SM_CHECKACCOUNTEXISTRET	0x1fc
// 用户关闭客户端
//cmd = 106
#define CM_LEAVESERVER			0x6a
// 用户小退游戏 1009
//cmd = 1009
#define CM_SOFTCLOSE			0x3f1
// 普通帐号登陆
//cmd = 2001
//flag	lowword = 1
//highword	highbyte = account length + 1
//lowbyte = allstringlength
//data = "account/password"
#define CM_LOGIN				0x7d1
// 登陆失败
//cmd = 503
//dwFlag = 0	no this account
//dwFlag = -1 	password wrong
#define SM_LOGINFAIL			0x1f7
// PopTang帐号登陆
//cmd = 2008
//dwFlag = 0
//data = "account/password"
//or"account/password/protecter"
#define CM_PTLOGIN				0x7d8
//-修改密码
//cmd = 2003
//data = account oldpassword newpassword
#define CM_CHANGEPASSWORD		0x7d3
// 修改密码成功
//cmd = 506
#define SM_CHANGEPASSWORDOK		0x1fa
// 修改密码失败
//cmd = 507
#define SM_CHANGEPASSWORDFAIL	0x1fb
// 提示框
//cmd = 2810
//data = string
#define SM_TIPWINDOW			0xafa
// 密码验证成功
//cmd = 529
//data = "*loginid"
#define SM_LOGINOK				0x211
// 选择服务器
//cmd = 104
//data = "服务器名字"
#define CM_SELECTSERVER			0x68
// 服务器选择成功
//cmd = 530
//data = "选人服务器ip/选人服务器端口/选人id"
//{messages of select character}
#define SM_SELECTSERVEROK		0x212

///////////////////////////////////////角色服务器//////////////////////////////////////////////

// 请求列表
//cmd = 100
//data = "*loginid/选人id"
#define CM_QUERYCHARLIST		0x64
// 查询角色
//cmd = 520
//wParam[2] = 1
//dwFlag = 人数
//一个人的情况
//data = "*名字/职业/头发/级别/性别/"
//两个人的情况 * 在上次登陆（或者新创建的）的角色名字前面
//data = "名字/职业/头发/级别/性别/名字/职业/头发/级别/性别/"
#define SM_CHARLIST				0x208
// 查询角色失败
//cmd = 527
#define SM_QUERYCHR_FAIL		0x20f
// 请求删除人物列表
//cmd = 104
//data = "*loginid/选人id"
#define CM_QUERYDELCHARLIST		0x68
// 查询删除角色
//cmd = 534
//dwFlag = 个数
//wParam[2] = 1	
//data = "名字/职业/头发/级别/性别/yyyy-mm-dd hh:mm/"
#define SM_DELCHARLIST			0x216
// 创建人物标示消息
//cmd = 100
// wParam[0] 客户端版本标志 (值为1,表示支持selgate和rungate合并的新客户端)
// wParam[1] 高速版客户端标志 (值为1,表示高速版客户端)
// wParam[2] 客户端标志 (值为1)
// data = "*842274295/61507366"
// data = loginid/玩家ID
#define CS_SELCHAR_CHARSERVER	0x64
// 请求创建人物
//cmd = 101
//data = "*loginid/名字/头发/职业/性别"
#define CM_QUERYCREATECHAR		0x65
// 创建人物成功
//cmd = 521
//dwFlag = 1
#define SM_CREATECHAROK			0x209
// 创建角色失败
//cmd = 522
#define SM_CREATECHARFAIL		0x20a
// 请求删除人物
//cmd = 102
//data = "人名字"
#define CM_QUERYDELCHAR			0x66
// 删除人物成功
//cmd = 523
//dwFlag = 1
#define SM_DELCHAROK			0x20b
// 请求恢复角色
//cmd = 105
//data = "人名字"
#define CM_QUERYUNDELCHAR		0x69
// 恢复成功
//cmd = 533
//dwFlag = 1
#define SM_UNDELCHAROK			0x215
// 请求选人
//cmd = 103
//data = "*loginid/人名字"
#define CM_QUERYSELECTCHAR		0x67
// 选人成功
//cmd = 525
//data = "游戏服务器ip/游戏服务器端口"
#define SM_SELECTCHAROK			0x20d

///////////////////////////////////////MESSAGE OF GAMEWORLD//////////////////////////////////////////////

//确认第一个提示框
//cmd = 1018
#define	CM_CONFIRMFIRSTDIALOG	0x3fa
// 发送公告
//cmd = 658
#define	SM_FIRSTDIALOG			0x292
// 聊天文字
//cmd = 40
#define SM_CHAT					0x28
// 系统消息
//cmd = 100
#define SM_SYSCHAT				0x64
// 版本验证失败
//cmd = 1106
#define SM_READY				0x452
// 请求加载地图
//cmd = 1033
#define CM_MAPLOADED			0x409
// 死亡
//cmd = 32
#define SM_DEATH                32
// 骷髅状态
//cmd = 33
#define SM_SKELETON             33
// 立即死亡
//cmd = 34
#define SM_NOWDEATH             34
// 人物在地图的位置
//cmd = 51
//flag = id
//w1 = x
//w2 = y
//w3 = dir
//data = 地图文件名
//0x19efedb0 0033 0006 000a 0007[SH013]
#define SM_SETMAP				0x33
// 地图跳转
#define SM_CLEAROBJECTS         633
// 瞬移后人物所在的位置
#define SM_CHANGEMAP			0x27a
// 地图描述
//data = 地图名字
#define SM_SETMAPNAME			0x36
// 角色名字
//flag = id
//w1 = color
//data = 名字
#define	SM_SETPLAYERNAME		0x2a
// 人物外观更新（加密）
//flag = id
//w1 = x
//w2 = y
//w3bl = dir
//data = dword * 4
#define	SM_SETPLAYER			0x32
// 设置物品位置
//w3 = count
//data = 
//itemid(4) pos(2)
#define SM_SETITEMPOSITION		0x46
// 请求物品位置
//cmd = 69
#define	CM_SETITEMPOSITION		0x45
// 设置天气时间（昼夜变化）
#define SM_SETGAMEDATETIME		0x2e
// 行走
//cmd=11
//dwflag = id
//wparam1 = x;
//wparam2 = y
//wparam3 = dir
#define SM_WALK					0x0b   
// 停止移动
//cmd = 204
//dwflag = id
#define SM_STOP					0xcc
// 跑
//cmd = 13
//dwflag = id
//wParam1 = x
//wParam2 = y
//wParam3 = dir
#define SM_RUN					0x0d
// 攻击
//cmd = 14
//dwflag = id
//wparam1 = x
//wparam2 = y
//wparam3 = dir
#define SM_ATTACK				0x0e
// 作揖
//cmd = 3021
//dwflag = id
//wparam1 = x
//wparam2 = y
//wparam3 = dir
#define CM_ZUOYI				0xbcd
// 释放技能 17
//cmd = 17
//id向x,y坐标释放技能data
//w3 = atoi(data)-2
#define SM_SPELLSKILL			0x11
// 升级
//cmd = 45
#define	SM_LEVELUP				0x2d
// 经验值增加
//cmd = 44
#define	SM_GETEXP				0x2c
// 附加人物属性
//cmd = 752
#define	SM_UPDATESUBPROP		0x2f0
// 使用物品
//cmd = 1006
#define	SM_USEITEM				0x3ee
// 丢金币
//cmd = 1016
#define	SM_DROPGOLD				0x3f8
// 技能等级经验
//cmd = 640
#define SM_SPELLEXPCHG			0x280
// 野蛮冲撞
//cmd = 6
#define	SM_RUSH					0x06
// 野蛮冲撞失败
//cmd = 7
#define SM_RUSH_FAIL			0x07
// 魔法打中效果
//cmd = 638
//w1-2 = x,y
//w3 = ??
//data(dword[2])（index0 = id)
#define SM_PLAYSKILLEFFECT		0x27e
// 角色状态变化
//cmd = 657
//dwflag = id
//w1 = 0
//w2 = 0
//w3 = 0
////3 = ？
////13 = 灵魂火符？?
////17 = 召唤骷髅?
//0x14	骷髅出现
//id
//x,y,dir
//data = dword[4] (index0 = outview)
////6 = 施毒术？
//0x291
//w1 = 0
//w2 = 0
//w3 = 0
////18 = 隐身
//UNKNOW MESSAGE 657
//0x14add278 0291 0000 0080 0000[] 隐身
//00a0 0000[] +魔防
//0020 0000[] +魔防（骷髅）
//00e0 0000[] +防御
//0060 0000[] 隐身消失
//0040 0000[] 防御消失
//0020 0000[] 魔防消失
//4000 0000[] 中毒
#define SM_CHARSTATUSCHANGED	0x291
// 设置id隐身
//cmd = 10
//19 = 群体隐身
//14=+魔防
//15 = ＋防御
//NPC添加10
//id
//w1 = x
//w2 = y
//w3 = dir
//dword1 = outview
//dword2 = bodyeffect
//dword3 = hp %|mp%
#define SM_APPEAR				0x0a
// 被击中（后仰）
//cmd = 31
//id
//w1 = hp%
//w2 = mp%
//w3 = ?(0x11) -hp
//dword0 = outview
//dword1 = ?(0x40000000) 火符
//dword 2 = attacker
//dword3 = ?
#define SM_BEATTACK				0x1f
// 立即死亡
//cmd = 34
//id
//x,y
//dir
//dword0 = outview dword1 = 0x40000000 (dword1 = viewflag)
#define SM_DIE					0x22
// 消失
//id at x,y,dir disappeared
#define SM_DISAPPEAR			0x1e
// 更换外观（加密）
//id
//w1 = 1400　　14=银蛇
//w2 = 5f01　　 5f = 道士２２级衣服
//w3 = 0000
//0x16409fc0 0029 1400 5f01 0000[]
#define SM_FEATURECHANGED       0x29
// 请求穿戴物品
//dwFlag = id
//w1 = index
//data = name
#define CM_TAKEONITEM			1003
// 名字颜色变化
#define SM_CHANGENAMECOLOR      656
// 请求取下穿戴物品
//dwFlag = id
//w1 = index
//data = name
#define CM_TAKEOFFITEM          1004
// 穿戴成功
//0x5f011400 0267 0000 0000 0000[]
#define SM_TAKEON_OK            615
// 穿戴失败
//0xffffffff 0268 0000 0000 0000[]
#define SM_TAKEON_FAIL          616
//0x5f010000 026b 0000 0000 0000[]
//feather
// 取下穿戴成功
#define SM_TAKEOFF_OK           619
// 取下穿戴失败
#define SM_TAKEOFF_FAIL         620
// 请求丢弃物品
#define CM_DROPITEM				0x3e8
// 请求拾取物品
#define CM_PICKUPITEM			0x3e9
// 物品显示
//data droped at x,y
//w3 = image
//flag = itemid
#define SM_DOWNITEMAPPEAR		0x262
// 物品隐藏
//itemid = flag at x,y disappeared
#define SM_DOWNITEMDISAPPEAR	0x263
// 屏幕滚动文字
//data = 文字
#define SM_SCROLLTEXT			0x22ed
// 身体特效（金刚护体/护身真气效果）
//id
//w1 0a w2 64  w3 ff= clr				
#define SM_STARTBODYEFFECT		0x1d
// 生命/魔法值变化
//id 
//w1 = hp
//w2 = mp
//w3 = hp
#define SM_HPMPCHANGED			0x35
// 请求释放技能
//flag = y | x
//w2 = 技能索引
//w1 = target id low word
//w3 = target id hi word
#define CM_SPELLSKILL			0xbc9
// 技能经验更新
//flag = 技能索引
//w1 = 1		等级
//w2 = 022a	经验
#define SM_SKILLEXPCHANGED		0x280
// 耐久度变化
//flag = cur dura
//w1 = pos
//w2 = maxdura
#define SM_ITEMDURACHANGED		0x282
// 客户端停止移动
//服务器发送 flag = id cc的消息给周围, 不发给自己
#define CM_STOP					0xbcc
// 好友列表
//w1 = 1 师傅
//w1 = 0 好友
//data = 名字/名字....
#define SM_FRIENDLIST			0x1c1
// 负重更新
//flag = 背包负重
//w1 = 负重量
//w2 = 腕力
#define SM_WEIGHTCHANGED		0x26e
// 重新连接
//data = ip/port
#define SM_CHANGESERVER			0x322
// NPC对话
//id=npcid
//w3 = 1
//data = 内容
//名字/页面
//页面内 \ 表示换行
//<>表示超级链接
//<显示文字/@ident>
//@ident表示选择的ident
//@@ident表示输入的ident
#define SM_NPCPAGE				0x283
// 选择超级链接
//flag = npcid
//data = @ident
//或者 @@ident[0d]输入的内容
//关闭窗口要发@exit
#define CM_SELECTLINK			0x3f3
// 说话
//cmd = 3030
//data = 话
#define CM_SAY					0xbd6
// 错误
//flag = 错误编号
#define SM_ERRORDIALOG			0x2fb
// 金币变化
//flag = 金钱数量
#define SM_GOLDCHANGED			0x28d
// 后退
//flag = id
//wparam[0] = x
//wparam[1] = y
//wparam[2] = dir
//data = dword view dword flag word ?
#define SM_BACK					0x9
// 人物属性值
//dwFlag = gold
//cmd = 0x34
//wParam[0] = b1 = prof b2 = hair wparam[1] = b1 = sex
//data = 
#define SM_UPDATEPROP			0x34
// 请求跑
//wCmd = bc5(跑)bc3(走)bc6(攻击)bc2(转身)bc4(挖肉)bcd(作揖, dwFlag =0)
//dwFlag = y<<16|x (将要走去的坐标或者做动作的坐标)
//wParam[1] = 方向
#define CM_RUN					0xbc5
// 请求走
#define CM_WALK					0xbc3
// 请求普通攻击
#define CM_ATTACK				0xbc6
// 请求转身
#define CM_TURN					0xbc2
// 挖肉
#define CM_GETMEAL				0xbc4
// 发送已穿戴物品
#define SM_EQUIPMENTS			0x26d
// 请求查看摆摊信息返回
//dwFlag = 0;
//dword( w1 w2 ) = 行会旗帜id
//w3 = 0xff
//GET /woool/getpic.asp?Area=62&Server=龙隐&Pid=2000008d HTTP/1.1
//Accept: image/gif, image/x-xbitmap, application/x-shockware-flash, */*
//Host: qizhi.woool.com
//Cache-Control: no-cache
//end of http query
#define CM_QUERYSTARTPRIVATESHOP	0x5eb1

/////////////////////////-=[交易相关]=-/////////////////////////

// 请求开始交易
#define CM_QUERYTRADE			0x401
// 交易开始
//data = 对方名字
#define SM_TRADESTART			0x2a1
// 放入物品
//dwFlag = 物品id
//w3 = 1
//data = 物品名字
#define CM_PUTTRADEITEM			0x402
// 放入物品成功
#define SM_PUTTRADEITEMOK		0x2a3
// 放入物品失败
#define SM_PUTTRADEITEMFAIL		0x2a4
// 交易按钮
#define CM_QUERYTRADEEND		0x406
// 交易结束
#define SM_TRADEEND				0x2af
// 请求放入金币、元宝
//dwflag = count
//w1 = 0 放入金钱
//w1 = 1 放入元宝
#define CM_PUTTRADEGOLD			0x405
// 放入金币、元宝成功
//dwflag = 放入数量
//w1w2 = 剩下数量
//w3 = 0 金钱
//w3 = 1 元宝
#define SM_PUTTRADEGOLDOK		0x2ac
// 放入金币、元宝失败
#define SM_PUTTRADEGOLDFAIL		0x2ad
// 设置元宝
//dwflag = count
#define SM_SETSUPERGOLD			0xe679
// 设置金钱
//dwflag = count
#define SM_SETGOLD				0x28d
// 对方放入金钱
//flag = count
//w3 = type
#define SM_OTHERPUTTRADEGOLD	0x2ae
// 对方放入物品
//flag = 对方id
//data = 物品数据
#define SM_OTHERPUTTRADEITEM	0x2aa
// 添加背包物品
//flag = 自己id
//w3 = 1
//data = 物品数据
#define SM_ADDBAGITEM			0xc8
// 背包信息
//w3 = count
//flag = id
//data = array of ITEM
#define SM_BAGINFO				0xc9
// 取消交易
#define CM_CANCELTRADE			0x404
// 交易被取消681
#define SM_TRADECANCELED		0x2a9
// 负重改变
//dwflag = 背包重量
//w1 = 
//w2 = 
#define SM_WEIGHTCHANGED		0x26e

///////////////-=[组队]=-///////////////////////////////

// 切换组队状态
//w1 = bGroup
#define CM_CHANGEGROUPMODE		0x3fb
// 组队状态
//w1 = bGroup
#define SM_GROUPMODE			0x293
// 请求组队
//data = 人名
#define CM_QUERYADDGROUPMEMBER	0x3fc
// 创建小组
#define	SM_GROUPCREATE			0x294
// 更新编组人位置
//data = [wx, wy] ....
#define SM_UPDATEMEMBERINFO		0x2e0
// 组队列表
//data = 人名/人名/
#define SM_GROUPMEMBERLIST		0x29b
// 删除组队人
//data = 人名
#define CM_DELETEGROUPMEMBER	0x3fe
// 删除成功
//data = ab 6c 63 4a ac 3a
#define SM_DELETECHARACTEROK	0x297
// 编组解散
#define SM_GROUPDESTROYED		0x29a
// 使用物品
#define SM_EAT_OK               635
// 使用物品失败
#define SM_EAT_FAIL             636
// 修理成功
//dwFlag = 当前的金钱数
//w1 = 持久
//w2 = 持久
#define SM_REPAIROK				0x29d
// 打开修理框
//dwFlag = Npcid
#define SM_OPENREPAIR			0x29c
// 放入休息物品成功
//dwFlag = 金钱
#define SM_PUTREPAIRITEMOK		0x29f

///////////////-=[全局变量定义]=-///////////////////////////////

#define MSGHEADERSIZE 12 //消息头大小

#define JOB_WAR 0 //战士职业
#define JOB_MAG 1 //法师职业
#define JOB_TAO 2 //道士职业

#define SEX_MALE 0 //性别男
#define SEX_FEMALE 1 //性别女

#define	MAX_LEVEL 99 //玩家最大等级
#define	MAX_TASK 50 //最大任务数, 最大不能超过254
#define	MAX_FENGHAO 58 //最大时长封号序号

#define GETOPPDIR(dir) (G_OPPDIR[(dir)%8]) //获取朝向
#define GETBACKDIR(dir) ((dir + 4)%8) //获取反向朝向
#define GETNEXTPOS( _x,_y ,dir) ((_x)+=G_DIROFS[(dir)%8].x,(_y)+=G_DIROFS[(dir)%8].y) //获取朝向的下一个坐标
#define GETNEXTNEXTPOS( _x,_y ,dir) ((_x)+=G_DIROFS[(dir)%8].x*2,(_y)+=G_DIROFS[(dir)%8].y*2) //获取朝向的下下一个坐标

#define MAKEDWORD2W(wl, wh) ((((DWORD)wh)<<16)|wl) //将两个16位的WORD值合并成一个32位的DWORD值
#define CLRMSG(msg)	memset( msg, 0, sizeof( MIRMSG )) //清空MIRMSG消息数据
#define	FILLSELF(fill) memset( this, (BYTE)(fill), sizeof( *this )) //将当前对象的所有内存空间填充为指定值

#define ONE_DAY_SECONDS 86400 // 1天的秒数

///////////////-=[消息结构体]=-///////////////////////////////

typedef enum MIRWORLD_ERROR
{
	ME_FAIL,
	ME_OK,
	ME_SOCKETWOULDBLOCK,
	ME_SOCKETCLOSED,
}MRESULT;

typedef struct tagMIRMSG
{
	DWORD dwFlag;
	WORD wCmd;
	WORD wParam[3];
	CHAR data[4];
}MIRMSG, * PMIRMSG;

typedef struct tagMIRMSGHEADER
{
	tagMIRMSGHEADER()
	{
		FILLSELF(0);
	}
	tagMIRMSGHEADER(DWORD _dwFlag, WORD _wCmd, WORD _w1, WORD _w2, WORD _w3)
	{
		dwFlag = _dwFlag;
		wCmd = _wCmd;
		w1 = _w1;
		w2 = _w2;
		w3 = _w3;
	}
	DWORD dwFlag;
	WORD wCmd;
	WORD w1;
	WORD w2;
	WORD w3;
}MIRMSGHEADER;

typedef struct tagLMIRMSG
{
	BOOL bUnCodedMsg;
	int	size;
	MIRMSG msg;
}LMIRMSG, * PLMIRMSG;


///////////////-=[自定义数据类型]=-///////////////////////////////

struct DELETED_DATE
{
	DWORD year : 12;
	DWORD month : 4;
	DWORD day : 5;
	DWORD hour : 4;
	DWORD minute : 6;
	DWORD bflag : 1;
};

///////////////-=[自定义枚举]=-///////////////////////////////

enum EM_PLAYERSTATE
{
	PS_STANDING,
	PS_TURNING,
	PS_WALKING,
	PS_RUNNING,
	PS_ATTACKING,
	PS_GETMEAL,
	PS_SPELLING,
	PS_ZUOYI,
};

///////////////-=[数据结构体]=-///////////////////////////////

typedef struct SELECT_CHAR_LIST
{
	char szName[19];
	BYTE btHair;
	BYTE btSex;
	BYTE btClass;
	WORD wLevel;
	DELETED_DATE date;
}SELECT_CHAR_LIST;

typedef struct DELETED_CHAR_LIST
{
	SELECT_CHAR_LIST character;
	DELETED_DATE datetime;
}DELETED_CHAR_LIST;

#pragma pack(push)
#pragma pack(1)
typedef struct tagP2PServer //微端服务器消息
{
	tagP2PServer()
	{
		FILLSELF(0);
	}
	BYTE btRec;
	int nIPAddr;
	WORD wPort;
}P2PServer;

typedef struct tagPETINFO
{
	WORD btLevel;
	CHAR sName[14];
	DWORD wCurHp;
	DWORD wMaxHp;
	WORD dc1;
	WORD dc2;
	WORD ac;
	WORD mac;
	BYTE flag;
}PETINFO;

typedef struct tagBASEITEM
{
	BYTE btNameLength;
	CHAR szName[14]; // 物品名字
	BYTE btStdMode; // 分类号
	BYTE btShape; // 装备外观
	BYTE btWeight; // 重量
	BYTE btAniCount; // 动画帧数
	BYTE btSpecialpower;
	union
	{
		struct
		{
			BYTE bNeedIdentify; // 需要鉴定
			BYTE btPriceType;
		};
		WORD wMapId;
	};
	WORD wImageIndex; // 物品外观
	WORD wMaxDura; // 持久上限
	union
	{
		struct
		{
			BYTE btMinDef;
			BYTE btMaxDef;
			BYTE btMinMagDef;
			BYTE btMaxMagDef;
			BYTE btMinAtk;
			BYTE btMaxAtk;
			BYTE btMinMagAtk;
			BYTE btMaxMagAtk;
			BYTE btMinSouAtk;
			BYTE btMaxSouAtk;
		};
		struct
		{
			BYTE Ac1; // 物理防御下限
			BYTE Ac2; // 物理防御上限
			BYTE Mac1; // 魔法防御下限
			BYTE Mac2; // 魔法防御上限
			BYTE Dc1; // 物理攻击下限
			BYTE Dc2; // 物理攻击上限
			BYTE Mc1; // 魔法攻击下限
			BYTE Mc2; // 魔法攻击上限
			BYTE Sc1; // 道术攻击下限
			BYTE Sc2; // 道术攻击上限
		};
		struct
		{
			WORD wAc; // 物理防御
			WORD wMac; // 魔法防御
			WORD wDc; // 物理攻击
			WORD wMc; // 魔法攻击
			WORD wSc; // 道术攻击
		};
	};
	BYTE needtype; // 需求类型
	BYTE needvalue; // 需求类型对应值
	union
	{
		WORD wFeature; // 特性
		struct
		{
			BYTE btFlag; // 标识
			BYTE btUpgradeTimes; // 物品升级次数
		};
	};
	DWORD nPrice; // 价格
}BASEITEM;

typedef struct tagPLAYERACTION
{
	tagPLAYERACTION()
	{
		action = PS_STANDING;
		x = 0;
		y = 0;
		dir = 0;
		dwStartTime = timeGetTime();
	}
	DWORD dwStartTime;
	EM_PLAYERSTATE action;
	WORD x, y;
	BYTE dir;
}PLAYERACTION;

typedef struct tagITEM
{
	tagITEM()
	{
		FILLSELF(0);
	}
	BASEITEM baseitem;
	DWORD dwMakeIndex;
	WORD wCurDura;
	WORD wMaxDura;
	BYTE btItemExt[360];
	DWORD dwParam[4]; //引擎判断使用 0 ItemLimit.txt中配置的限制物品ID, 1 未使用 2 物品位置POS, 3 物品使用的状态
	CHAR szFullName[16]; //完整名字, 用于搜索ITEMCLASS对象绑定指针
	size_t nItemClassPtr; // 指向ITEMCLASS的指针ID
	BOOL operator == (const char* pszName)
	{
		BYTE btStdMode = baseitem.btStdMode;
		baseitem.btStdMode = 0;
		BOOL bRet = (strcmp(baseitem.szName, pszName) == 0);
		baseitem.btStdMode = btStdMode;
		return bRet;
	}
	BOOL operator == (tagITEM& item)
	{
		if (baseitem.btNameLength != item.baseitem.btNameLength)
			return FALSE;
		if (strncmp(baseitem.szName, item.baseitem.szName, baseitem.btNameLength > 14 ? 14 : baseitem.btNameLength) == 0)return TRUE;
		return FALSE;
	}
	//设置客户端物品长度
	VOID SetLen(BYTE btLen = 70, BYTE btNum = 1)
	{
		btItemExt[32] = btLen;
		btItemExt[33] = btNum;
	}
	// 获取豹子喂养时间
	DWORD GetPetTime() const
	{
		return *reinterpret_cast<const DWORD*>(&baseitem.wAc); // 直接读4字节
	}
	// 更新喂养宠物时间
	VOID SetPetTime()
	{
		time_t t;
		time(&t);
		DWORD dwT2 = static_cast<DWORD>(t);
		*reinterpret_cast<DWORD*>(&baseitem.wAc) = dwT2; // 直接写4字节
	}
	// 判断物品是否绑定
	BOOL IsBind() const
	{
		return btItemExt[4] == static_cast<BYTE>(1);
	}
	// 设置物品为绑定
	VOID SetBind(BOOL bBind)
	{
		btItemExt[4] = bBind ? 1 : 0;
	}
	// 获取物品扩展名字
	const char* GetExName() const
	{
		return (char*)&btItemExt[122];
	}
	// 设置物品扩展名字
	VOID SetExName(const char* szName)
	{
		if (szName == nullptr) return;
		strncpy_s((char*)&btItemExt[122], 20, szName, _TRUNCATE);
	}
}ITEM;

//客户端物品数据结构
typedef struct tagITEMCLIENT
{
	tagITEMCLIENT()
	{
		FILLSELF(0);
	}
	BASEITEM baseitem;
	DWORD dwMakeIndex;
	WORD wCurDura;
	WORD wMaxDura;
	BYTE btItemExt[360];
	//设置客户端物品长度
	VOID SetLen(BYTE btLen = 70, BYTE btNum = 1)
	{
		btItemExt[32] = btLen;
		btItemExt[33] = btNum;
	}
}ITEMCLIENT;

//	个人商店察看/看见(0xfca0)
//dwFlag = id
//w1 = x
//w2 = y;
//w3 = dir
//data = tagprivateshop
//w2 = 1( show box ) 0 (hide box )
//wCount = 0;
typedef struct tagPRIVATESHOPHEADER
{
	tagPRIVATESHOPHEADER()
	{
		FILLSELF(0);
	}
	WORD w1;
	BYTE w2;
	BYTE btFlag;
	DWORD dw1;
	WORD wCount;
	CHAR szName[52];
}PRIVATESHOPHEADER;

typedef struct tagPRIVATESHOPSHOW
{
	tagPRIVATESHOPSHOW()
	{
		FILLSELF(0);
	}
	PRIVATESHOPHEADER header;
	ITEMCLIENT items[10];
}PRIVATESHOPSHOW;

typedef struct tagPrivateShopItemQuery
{
	DWORD dwMakeIndex;
	DWORD dwPrice;
	WORD wPriceType;
}PS_ITEMQUERY;

typedef struct tagPrivateShopQuery
{
	char szName[52];
	PS_ITEMQUERY items[1];
}PRIVATESHOPQUERY;

typedef struct tagEQUIPMENT // 穿戴物品
{
	WORD pos = 0;
	ITEMCLIENT item;
}EQUIPMENT;

typedef struct tagBAGITEMPOS
{
	DWORD ItemId;
	WORD wPos;
}BAGITEMPOS;

typedef struct tagDBITEM
{
	ITEM item;
	WORD pos = 0;
	BYTE btFlag = 0;
}DBITEM;

typedef struct tagHUMANPROP // 人物属性 279字节
{
	tagHUMANPROP()
	{
		FILLSELF(0);
	}
	WORD wLevel;			//等级
	WORD btMinDef;			//防御下限
	WORD btMaxDef;			//防御上限
	WORD btMinMagicDef;		//魔御下限
	WORD btMaxMagicDef;		//魔御上限
	WORD btMinAtk;			//攻击下限
	WORD btMaxAtk;			//攻击上限
	WORD btMinMagAtk;		//魔法下限
	WORD btMaxMagAtk;		//魔法上限
	WORD btMinSprAtk;		//道术下限
	WORD btMaxSprAtk;		//道术上限
	WORD wCurHp;			//生命值
	WORD wCurMp;			//魔法值
	WORD wMaxHp;			//最大生命值
	WORD wMaxMp;			//最大魔法值
	DWORD PKlevel;			//PK值
	UINT64 dwCurexp;		//当前经验
	UINT64 dwMaxexp;		//最大经验
	WORD wCurBagWeight;		//当前背包重量
	WORD wMaxBagWeight;		//最大背包重量
	BYTE btCurBodyWeight;	//负重
	BYTE btMaxBodyWeight;	//最大负重
	BYTE btCurHandWeight;	//腕力
	BYTE btMaxHandWeight;	//最大腕力
	WORD wManito;			//灵力值
	WORD wEnergy;			//元气最大值
	//[1.978飞升属性]
	WORD wCurGodHP;			//元力(神、魔)
	WORD wMaxGodHP;			//元力最大值(神、魔)
	BYTE byGodRenew;		//元力恢复(神、魔)
	BYTE byGodType;			//圣殿分类 (0=无, 3=天人神, 4=天人魔, 5=天仙神, 6=天仙魔, 7=上仙神、8=上仙魔)
	BYTE byGodLevel;		//天人境界重数
	//[1.979装备分拆]
	BYTE btDecomPoseLevel;	//拆卸等级
	DWORD dwDecomPoseExp;	//拆卸经验
	DWORD dwDecomPoseExpMax;//拆卸最大经验
	WORD wStamina;			//精力值
	WORD wMaxStamina;		//最大精力值
	//[1.979VIP机制]
	BYTE btVIPState;		//VIP状态(0=不是, 1=银卡, 2=金卡)
	//[绑定金币元宝]
	DWORD dwBindMoney;		//绑定金币
	DWORD dwBindYuanBao;	//绑定元宝

	WORD wFightValue;		//融合元神 战斗值
	DWORD wSeaDemon;		//海魔值累计值总值2000, 1星200、2星400、3星600、4星800
	CHAR szMemo[10];		//备注/签名信息
	BYTE btVipNextLevel;	//VIP商行下一等级
	DWORD dwVipNextExp;		//VIP商行下一等级经验值
	DWORD dwActiveRate;		//活跃度
	BYTE btVipLevel;		//VIP等级
	DWORD dwPrizeBit;		//奖励领取情况
	BYTE byWingColor;		//翅膀颜色
	DWORD dwGC;				//妖力攻击？分上下限
	DWORD dwHP;				//当前生命值
	DWORD dwMaxHP;			//最大生命值
	WORD wBaseDC1;			//基础物理攻击力下限
	WORD wBaseDC2;			//基础物理攻击力上限
	WORD wBaseMC1;			//基础魔法攻击力下限
	WORD wBaseMC2;			//基础魔法攻击力上限
	WORD wBaseSC1;			//基础道术攻击力下限
	WORD wBaseSC2;			//基础道术攻击力上限
	WORD wBaseGC1;			//基础妖力攻击力下限
	WORD wBaseGC2;			//基础妖力攻击力上限
	DWORD dwMP;				//当前魔法值
	DWORD dwMaxMP;			//最大魔法值
	DWORD dwElementForce;	//元力值(神、魔)
	DWORD dwElementForceMax;//元力最大值(神、魔)
	DWORD dwExtendAC1;		//物理防御力下限
	DWORD dwExtendAC2;		//物理防御力上限
	DWORD dwExtendMAC1;		//魔法魔御力下限
	DWORD dwExtendMAC2;		//魔法魔御力上限
	DWORD dwExtendDC1;		//物理攻击力下限
	DWORD dwExtendDC2;		//物理攻击力上限
	DWORD dwExtendMC1;		//魔法攻击力下限
	DWORD dwExtendMC2;		//魔法攻击力上限
	DWORD dwExtendSC1;		//道术攻击力下限
	DWORD dwExtendSC2;		//道术攻击力上限
	DWORD dwExtendGC1;		//妖力攻击力下限
	DWORD dwExtendGC2;		//妖力攻击力上限
	BYTE btMilitaryRank;	//军衔等级
	WORD wPersonalCode;		//特殊封号-个性设置
	BYTE btGuildMember;		//行会会员等级
	BYTE unKownAbi[61];
}HUMANPROP;

typedef struct tagHUMANSUPROP
{
	tagHUMANSUPROP()
	{
		FILLSELF(0);
	}
	WORD wHuoli;			//活力值
	WORD wHuoliMax;			//最大活力值
	BYTE bColor;			//颜色
	DWORD dwForgePoint;		//极品值
	BYTE bLucky;			//幸运
	BYTE bDawn;				//诅咒
	BYTE btMagicNicety;		//魔法命中
	BYTE btPoisonNicety;	//中毒命中
	BYTE unKownAbil1[15];	//未知
	BYTE btThump;			//暴击概率
	BYTE unKownAbil2[54];	//未知
	DWORD dwSpeedPoint;		//躲避
	DWORD dwUnKnow;			//极品修炼值
	WORD wThump;			//暴击等级
	BYTE unKownAbil3[4];	//未知
	DWORD dwHitPoint;		//命中
	DWORD dwMagicNicety;	//魔法命中
	DWORD dwAntiMagic;		//魔法躲避
	DWORD dwPoisonNicety;	//中毒命中
	DWORD dwAntiPoison;		//中毒躲避
	BYTE unKownAbil4[8];	//未知
	DWORD dwToughness;		//韧性
	BYTE unKownAbil5[56];	//未知
}HUMANSUPROP;

typedef struct tagMAGIC
{
	tagMAGIC()
	{
		FILLSELF(0);
	}
	CHAR cKey;			//快捷键索引
	BYTE btLevel;		//技能等级
	WORD wIdx;			//ID-客户端没用-未使用
	WORD iCurExp;		//当前技能经验值
	WORD iCurNewExp;	//当前新技能经验值-未使用
	WORD wId;			//技能编号
	BYTE btNameLength;	//技能名称长度
	CHAR szName[12];	//技能名称
	BYTE btEffectType;	//效果类型
	BYTE btEffect;		//效果标志
	BYTE btNewDefence;	//新防御技能-未使用
	WORD wSpell;		//施法值
	WORD wPower;		//威力值-未使用
	BYTE btNeedLevel[4];//升级需要的等级
	WORD wUnknow1;		//保留
	DWORD iLevelupExp[4];//修炼经验值
	BYTE btUnknow1;		//保留
	BYTE job;			//职业类型
	WORD wUnknow2;		//保留
	WORD wDelayTime;	//CD时间
	WORD wUnknow3;		//保留
	BYTE Defence;		//防御值-未使用
	BYTE DefencePower;	//防御威力-未使用
	BYTE wMaxPower;		//最大威力-未使用
	BYTE btUnknow2;		//保留
	BYTE wDefMaxPower;	//防御最大威力-未使用
	BYTE btUnknow3;		//保留
	//额外技能修炼
	BYTE btNeedLevel2[7];//升级所需等级-未使用
	DWORD iLevelupExp2[7];//升级所需技能值-未使用
}MAGIC;

typedef struct tagREGISTERACCOUNT
{
	BYTE btAccount;
	char szAccount[10];
	BYTE btPassword;
	char szPassword[10];
	BYTE btName;
	char szName[20];
	BYTE btIdCard;
	char szIdCard[19];
	BYTE btPhoneNumber;
	char szPhoneNumber[14];
	BYTE btQ1;
	char szQ1[20];
	BYTE btA1;
	char szA1[20];
	BYTE btEmail;
	char szEmail[40];
	BYTE btQ2;
	char szQ2[20];
	BYTE btA2;
	char szA2[20];
	BYTE btBirthday;
	char szBirthday[10];
	BYTE btMobileNumber;
	char szMobileNumber[11];
}REGISTERACCOUNT;
#pragma pack(pop)

typedef struct tagMONSKILL
{
	DWORD tId;
	WORD wX;
	WORD wY;
	WORD wMagicID;
}MONSKILL;

typedef struct tagMIRPOSITION
{
	WORD x;
	WORD y;
}MIRPOSITION;

typedef struct tagCOLTYPE
{
	WORD type;
	WORD length;
}COLTYPE;

typedef struct tagEXECSQLRECORD
{
	DWORD dwCount;
	DWORD dwColCount;
	COLTYPE coldef[1];
}EXECSQLRECORD;

typedef struct tagEXECSQLCOMMAND
{
	DWORD dwLength;
	char command[1];
}EXECSQLCOMMAND;

typedef struct tagITEMOPERATION
{
	DWORD dwMakeIndex;
	BYTE btOperation;
	BYTE btParam;
	WORD wParam;
	DWORD dwParam;
}ITEMOPERATION;

typedef struct tagCREATEGUILD
{
	char szName[32];
	UINT nCreatorId;
}CREATEGUILD;

typedef struct tagMEMBERINFO
{
	char szName[32];
	char szTitle[32];
	UINT GuildId;
	UINT CharId;
	int	nTitleLevel;
	SYSTEMTIME EnterTime;
}MEMBERINFO;

typedef struct tagGUILDINFO
{
	UINT Id;
	char szName[32];
	char szNotice[1024];
	SYSTEMTIME	CreateTime;
	DWORD KillGuilds[10];
	DWORD AllyGuilds[10];
	int	nMemberCount;
}GUILDINFO;

typedef struct tagMAGICDB
{
	BYTE btLevel;
	BYTE btKey;
	WORD wId;
	DWORD dwCurTrain;
}MAGICDB;

typedef struct tagCREATEITEM
{
	DWORD dwClientKey;
	DWORD dwOwner;
	BYTE btFlag;
	WORD wPos;
	ITEM item;
}CREATEITEM;

typedef struct tagCREATECHARDESC
{
	tagCREATECHARDESC()
	{
		FILLSELF(0);
	}
	DWORD dwKey;
	char szName[32];
	char szServer[32];
	char szAccount[14];
	BYTE btClass;
	BYTE btSex;
	BYTE btHair;
	BYTE btLevel;
}CREATECHARDESC;

typedef struct tagFEATHER
{
	BYTE btRace;
	BYTE btWeapon;
	BYTE btHair;
	BYTE btDress;
}FEATHER;

typedef struct tagCHARDBINFO
{
	tagCHARDBINFO()
	{
		FILLSELF(0);
	}
	DWORD dwClientKey; // 客户端钥匙
	char szName[20]; // 名称
	DWORD dwDBId; // 数据ID
	DWORD mapid; // 地图ID
	WORD x; // X坐标
	WORD y; // Y坐标
	BYTE dir; // 方向
	BYTE btAttackMode; // 攻击模式
	int nGameTime; // 游戏时间
	DWORD dwGold; // 金币
	DWORD dwYuanbao; // 元宝
	BOOL bBigGold; // 是否是大金币包
	BOOL bBigBag; // 是否是大背包
	WORD wPersonCode; // 个性化代码
	char szPersonSign[20]; //个性化签名
	char szTempRank[60]; //临时称号
	DWORD dwCurExp; // 当前经验
	WORD wLevel; // 等级
	BYTE btClass; // 职业
	BYTE btHair; // 头发
	BYTE btSex; // 性别
	BYTE flag;
	WORD hp; // 生命值
	WORD mp; // 魔法值
	WORD maxhp; // 最大生命值
	WORD maxmp; // 最大魔法值
	BYTE mindc; // 攻击下限
	BYTE maxdc; // 攻击上限
	BYTE minmc; // 魔法下限
	BYTE maxmc; // 魔法上限
	BYTE minsc; // 道术下限
	BYTE maxsc; // 道术上限
	BYTE minac; // 防御下限
	BYTE maxac; // 防御上限
	BYTE minmac; // 魔御下限
	BYTE maxmac; // 魔御上限
	WORD weight; // 重量
	BYTE handweight; // 腕力
	BYTE bodyweight; // 负重
	DWORD dwForgePoint; // 极品修炼值
	DWORD dwProp[8]; // 全部-未使用
	DWORD dwFlag[4]; // 0低位玩家声望值,脚本变量 "credit" 0的31位是否开启组队模式  1玩家PK杀人数值  2全32位标识0-31,脚本自定义标志位  3全32位标识32-63,脚本自定义标志位
	char szStartPoint[40]; // 出生点
	char szGuildName[32]; // 行会名称
}CHARDBINFO;

typedef struct tagUPGRADEADDMASK
{
	tagUPGRADEADDMASK()
	{
		FILLSELF(0);
	}
	union
	{
		struct
		{
			DWORD addtype1 : 3;
			DWORD addtype2 : 3;
			DWORD addvalue1 : 2;
			DWORD addvalue2 : 2;
			DWORD adddura : 2;
			DWORD badddura : 1;
			DWORD flag : 3;
			DWORD left : 16;
		};
		struct
		{
			WORD wAddMask;
			WORD wItemLimit;
		};
		DWORD dwValue;
	};
}UPGRADEADDMASK;

typedef struct tagTASKNODE
{
	WORD wTaskId;
	WORD wStep;
	DWORD vParam[10];  // 玩家任务数值参数, 每个任务独立存储
	char sParam[10][32];  // 玩家任务字符串参数, 每个任务独立存储
}TASKNODE;

typedef struct tagTASKINFO
{
	DWORD dwAchievement; //积分
	DWORD dwTotalTaskCount; // 已完成任务数
	TASKNODE tasks[MAX_TASK];
}TASKINFO;

struct FengHaoRow
{
	BOOL  boActivation; //是否激活
	DWORD dwLastDate; //持续时间(秒)
};

typedef struct tagFenghaoInfo // // 时长封号玩家信息
{
	tagFenghaoInfo()
	{
		FILLSELF(0);
	}
	BYTE btType1;//普通封号
	BYTE btType2;//特殊封号
	BYTE btType3;//节日封号
	FengHaoRow mFengHaoRow[MAX_FENGHAO];
	// 获取激活的称号数量
	BYTE GetCount()const
	{
		BYTE btCount = 0;
		for (int i = 0; i < MAX_FENGHAO; ++i)
		{
			if (mFengHaoRow[i].boActivation)
				btCount++;
		}
		return btCount;
	}
}FenghaoInfo;

///////////////-=[数据服务器]=-///////////////////////////////
enum dbmsg
{
	DM_START,
	//	请求检查帐号
	//	send...
	//	data = account/password
	//	recv...
	//	w1 = success?
	//	w2 = reason
	DM_CHECKACCOUNT,
	//	检查账号是否存在
	//	send...
	//	data = account
	//	recv ...
	//	w1 = bool
	DM_CHECKACCOUNTEXIST,
	//	创建帐号
	//	send...
	//	data = account/password/name/birthday/q1/a1/q2/a2/
	//	recv...
	//	w1 = success?
	//	w2 = reason
	DM_CREATEACCOUNT,
	//	修改密码
	//	send...
	//	data = account/password/newpassword
	//	recv...
	//	w1 = success?
	//	w2 = reason
	DM_CHANGEPASSWORD,
	//	请求帐号角色列表
	//	send...
	//	data = account
	//	recv...
	//	data = list
	//	w1 = success?
	//	w2 = count
	DM_QUERYCHARLIST,
	//	请求创建角色
	//	send...
	//	data = CREATECHARDESC
	//	recv...
	//	w1 = success?
	DM_CREATECHARACTER,
	//	请求删除角色
	//	send..
	//	data = charname/account/server
	//	recv..
	//	w1 = success?
	DM_DELETECHARACTER,
	//	请求删除角色列表
	//	send..
	//	recv...
	//	w1 = success?
	DM_DELETEDCHARLIST,
	//	请求恢复角色, 
	DM_RESTORECHARACTER,
	//	请求角色地图名和位置
	//	send...
	//	data = account/server/name
	//	recv...
	//	data = mapname/x/y
	DM_GETCHARPOSITIONFORSELCHAR,
	//	请求角色属性信息
	//	send...
	//	data = dwClientKey | account/server/name
	//	recv...
	//	data = dwClientKey | charinfo
	DM_GETCHARDBINFO,
	//	修改角色属性信息
	//	wParam[0] = 1 need return value
	//	data = CHARDBINFO struct
	DM_PUTCHARDBINFO,
	//	请求创建一个物品  FLAG 指定物品创建在何处
	//	send...
	//	CREATEITEM
	//	recv...
	//	CREATEITEM
	DM_CREATEITEM,
	//	请求删除一个物品
	//	send...
	//	CLIENTKEY | ITEM ID | FLAG
	DM_DELETEITEM,
	//  更新物品时间
	//	SEND...
	//	ITEM STRUCT
	DM_UPDATEITEM,
	//	更新多个物品时间
	//	ITEM / POS
	//	dwFlag = ownerid
	//	wParam[0] = count
	//	wParam[1] = flag
	//	data = DBITEM
	DM_UPDATEITEMS,
	//	更新物品位置
	//	id	pos
	DM_UPDATEITEMPOS,
	//	更新
	//	id	pos	owner
	//	更新pos和owner
	DM_UPDATEITEMOWNER,
	//	查询物品
	//	wParam[1] = btFlag
	//	wParam[2] = max length
	//	data = clientkey|ownerid
	//	recv
	//	wParam[0] = result
	//	wParam[1] = btFlag
	//	wParam[2] = length
	//	data = clientkey|items
	DM_QUERYITEMS,
	//  更新技能
	//	dwFlag = dbid
	//	wParam[0] = 
	//	wParam[1] = 
	//	wParam[2] = count
	//	data = MAGICDB * count
	DM_UPDATEMAGIC,
	//  查询技能
	//	send
	//	dwFlag = clientid
	//	w1 w2 = clientkey
	//	data = [dword dbid]
	//	recv
	//	dwFlag = clientid
	//	w1 w2 = clientkey
	//	w3 = count
	//	data = MAGICDB[count]
	DM_QUERYMAGIC,
	//	dwFlag = serverid
	//	w1 = state
	//	data = account
	DM_UPDATEACCOUNTSTATE,
	//	w1 = count
	//	w2 = flag
	//	data = BAGITEMPOS*xxx
	DM_UPDATEITEMPOSEX,
	//	行会相关
	//	1-	创建行会
	//	data = CREATEGUILD
	//	更新多个ITEMPOS
	//	w1 = count
	//	data = UpdateItemPos * count
	DM_UPDATEITEMPOSEX2,
	//	w1 = count
	//	data = UpdateItemOwner * count
	DM_UPDATEITEMOWNEREX,
	//	w1 = count
	//	data = ITEMSTRUCT * count
	DM_UPDATEITEMEX,
	//	dwFlag = ownerid
	//	data = .....
	//	wife/student1/student.../friend1/friend2.....
	DM_UPDATECOMMUNITY,
	//	send
	//	dwFlag = ownerid
	//	w1 w2 = clientkey
	//	recv
	//	dwFlag = ownerid
	//	w1 w2 = clientkey
	//	data = wife/student1/stud..../friend1/friend2.....
	DM_QUERYCOMMUNITY,
	//	data = c1/c2
	DM_BREAKFRIEND,
	//	data = c1/c2
	DM_BREAKMARRIAGE,
	//	data = c1/c2
	DM_BREAKMASTER,
	//	dwFlag = dbid
	DM_CACHECHARDATA,
	//	dwFlag = ownerid
	//	w1 w2 = findkey
	DM_UPGRADEITEM,
	//	dwFlag = ownerid
	//	w1 w2 = clientkey
	//	data = ITEM
	DM_QUERYUPGRADEITEM,
	//	szName/szGuildName
	DM_RESTOREGUILDNAME,
	//	执行SQL语句
	//	send 
	//	[DWORD dwTransId] [DWORD length] [char sql[]] [DWORD RECORD COUNT] [DWORD cols type count] [cols typs]
	//	recv
	//	[DWORD dwTransId] [DWORD result] [DWORD RECORD COUNT] [RECORDS]
	DM_EXECSQL,
	//	SEND
	//	[UINT nAdd] [char name[20]]
	DM_ADDCREDIT,
	//更新玩家的行会名
	DM_UPDATECHARGUILDNAME,
	//	
	DM_CHECKFREE,
	//  删除技能
	DM_DELETEMAGIC,
	//  查询任务
	//	dwFlag = clientid
	//	w1 w2 = clientkey
	//	data = id
	//	data = taskinfo
	DM_QUERYTASKINFO,
	//  更新任务
	//	dwFlag = ownerdbid
	//	data = TaskInfo
	DM_UPDATETASKINFO,
	//  查询时长封号
	//	dwFlag = clientid
	//	w1 w2 = clientkey
	//	data = id
	//	data = FenghaoInfo
	DM_QUERYFENGHAO,
	//  更新时长封号
	//	dwFlag = ownerdbid
	//	data = FenghaoInfo
	DM_UPDATEFENGHAO,
	DM_END,
};

///////////////-=[服务中心]=-///////////////////////////////
enum scmsg
{
	SCM_START,
	//	注册服务器
	//	send...
	//	dwFlag = id( server = 0 other id != 0 )
	//	data = REGISTER_SERVER_INFO
	//	recv...
	//	dwFlag = id
	//	w1 = success?
	//	w2 = reason
	//	data = REGISTER_SERVER_RESULT
	SCM_REGISTERSERVER,
	//	取得选人服务器地址
	//	send...
	//	data = loginid/servername
	//	recv...
	//	w1 = success?
	//	w2 = reason
	//	data = ip/port/selectid
	SCM_GETSELCHARSERVERADDR,
	//	取得游戏世界服务器地址
	//	send...
	//	
	//	mapname/x/y/servername
	//	recv
	//	w1 = success?
	//	w2 = reason
	//	data = ip/port
	SCM_GETGAMESERVERADDR,
	//	更新服务器信息
	//	send...
	//	w1 = connections
	//	dwFlag = float loop time
	//	every one second
	SCM_UPDATESERVERINFO,
	//	取得服务器地址
	//	send...
	//	w1 = type
	//	data = name
	//	recv
	//	w1 = success?reason
	//	data = SERVERADDR
	SCM_FINDSERVER,
	//	发送服务器间消息
	//	send...
	//	dwflag = 0
	//	w1 = cmd
	//	w2 = sendtype
	//	w3 = sendparam
	//	data = data
	//	recv...
	//	dwflag = 0
	//	w1 = cmd
	//	w2 = sendservertype
	//	w3 = sendserverindex
	//	data = data
	SCM_MSGACROSSSERVER,
};

enum MSG_ACROSS_SERVER
{
	//	踢掉某人
	//	让数据库里的标记设置成不在线
	//	data = account
	MAS_KICKCONNECTION,
	//	登陆服务器请求进入选人服务器
	//	send...
	//	data = loginid/account
	//	recv...
	//	data = selcharid
	MAS_ENTERSELCHARSERVER,
	//	进入游戏世界服务器
	//	send...
	//	data = ENTERGAMESERVER
	//	recv...
	//	data = fail or success
	MAS_ENTERGAMESERVER,
	//	send...
	//	data = LID/ACCOUNT/SID
	//	recv
	//	data = 
	MAS_RESTARTGAME,
};

//{方向表}
enum E_DIR
{
	DR_UP,			//上
	DR_UPRIGHT,		//右上
	DR_RIGHT,		//右
	DR_DOWNRIGHT,	//右下
	DR_DOWN,		//下
	DR_DOWNLEFT,	//左下
	DR_LEFT,		//左
	DR_UPLEFT,		//左上
};

enum dbITEMOPERATION//物品操作
{
	DIO_DELETE,		//删除
	DIO_UPDATEPOS,	//更新位置
	DIO_UPDATEOWNER,//更新拥有者
	DIO_UPDATEDURA	//更新耐久度
};

enum CLIENT_STATE	//客户端状态
{
	CS_NOSTATE,		//未登录
	CS_LOGIN,		//登录
	CS_SELECTCHAR,	//选角色
	CS_GAMEWORLD,	//游戏世界
};

enum PRICE_TYPE		//计价类型
{
	PT_GOLD,		//金币
	PT_YUANBAO,		//元宝
};

enum E_COLTYPE		//列类型
{
	CT_STRING,		//字符串
	CT_TINYINT,		//8位整数
	CT_SMALLINT,	//16位整数
	CT_INTEGER,		//32位整数
	CT_BIGINT,		//64位整数
	CT_DATETIME,	//时间
	CT_CODEDARRAY,	//编码存的数据
};

enum ITEM_DB_FLAG	//物品数据标记
{
	IDF_GROUND,		//地面
	IDF_BAG,		//背包
	IDF_EQUIPMENT,	//装备
	IDF_NPC,		//NPC
	IDF_BANK,		//仓库
	IDF_CACHE,		//缓存
	IDF_PETBANK,	//宠物背包
	IDF_UPGRADE,	//升级
};

//属性索引
enum PROP_INDEX
{
	PI_MINAC,			//防御下限
	PI_MAXAC,			//防御上限
	PI_MINMAC,			//魔防下限
	PI_MAXMAC,			//魔防上限
	PI_MINDC,			//攻击下限
	PI_MAXDC,			//攻击上限
	PI_MINMC,			//魔法下限
	PI_MAXMC,			//魔法上限
	PI_MINSC,			//道术下限
	PI_MAXSC,			//道术上限
	PI_HITRATE,			//命中
	PI_ESCAPE,			//躲避
	PI_MAGESCAPE,		//魔法躲避 百分比
	PI_MAGHITRATE,		//魔法命中 百分比
	PI_POISONESCAPE,	//中毒躲避 百分比
	PI_POISONHITRATE,	//中毒命中 百分比
	PI_ATTACKSPEED,		//攻速
	PI_LUCKY,			//幸运
	PI_DAWN,			//诅咒
	PI_HPRECOVER,		//生命回复
	PI_MPRECOVER,		//魔法回复
	PI_POISONRECOVER,	//中毒恢复
	PI_HARD,			//强度
	PI_HOLLY,			//神圣
	PI_LEVEL,			//等级
	PI_CURBAGWEIGHT,	//当前背包负重
	PI_MAXBAGWEIGHT,	//背包最大负重
	PI_CURHANDWEIGHT,	//当前腕力
	PI_MAXHANDWEIGHT,	//最大腕力
	PI_CURBODYWEIGHT,	//当前负重
	PI_MAXBODYWEIGHT,	//最大负重
	PI_CURHP,			//当前生命值
	PI_CURMP,			//当前魔法值
	PI_MAXHP,			//最大生命值
	PI_MAXMP,			//最大魔法值
	PI_HPPERCENT,		//生命值百分比的分子
	PI_EXP,				//经验
	PI_LEVELUPEXP,		//升级所需经验
	PI_EXPPERCENT,		//升级经验百分比的分子
	PI_PROP_COUNT,		//属性数量
};

//{装备位置对应表}
enum E_EQUIPPOD
{
	_U_DRESS,	//0衣服
	_U_WEAPON,	//1武器
	_U_CHARM,	//2勋章
	_U_NECKLACE,//3项链
	_U_HELMET,	//4头盔
	_U_ARMRINGR,//5右手手镯
	_U_ARMRINGL,//6左手手镯
	_U_RINGR,	//7右手戒指
	_U_RINGL,	//8左手戒指
	_U_SHOE,	//9靴子
	_U_BELT,	//10腰带
	_U_STONE,	//11宝石
	_U_POISON,	//12毒药
	_U_MAX,		//最大
};


enum E_ITEM_NEEDTYPE
{
	INT_LEVEL,		//等级限制
	INT_DC,			//攻击限制
	INT_MC,			//魔法限制
	INT_SC,			//道术限制
	INT_PKVALUE,	//PK值限制
	INT_CREDIT,		//声望限制
	INT_SABUKOWNER,	//沙城主限制
};

//职业
enum E_PRO
{
	PRO_WARRIOR,	//战士
	PRO_MAGICIAN,	//法师
	PRO_TAOSHI,		//道士
	PRO_MAX,		//最大
};

//注册帐号索引
enum REGISTER_ACCOUNT_INDEX
{
	RAI_ACCOUNT = 0,			//账号
	RAI_PASSWORD = 11,			//密码
	RAI_NAME = 22,				//名字
	RAI_IDCARD = 43,			//身份证
	RAI_PHONENUMBER = 63,		//电话号码
	RAI_Q1 = 78,				//问题1
	RAI_A1 = 99,				//答案1
	RAI_MAIL = 120,				//邮箱
	RAI_Q2 = 161,				//问题2
	RAI_A2 = 182,				//答案2
	RAI_BIRTHDAY = 203,			//生日
	RAI_MOBILEPHONENUMBER = 214,//手机号码
	RAI_UNKNOWN = 226,			//未知
};

//物品分类
enum ITEM_STDMODE
{
	ISM_DRUG = 0,			//药			
	ISM_FOOD0 = 1,			//食物1
	ISM_FOOD1 = 2,			//食物2
	ISM_USABLEITEM = 3,		//可使用物品
	ISM_BOOK = 4,			//书
	ISM_WEAPON0 = 5,		//武器1
	ISM_WEAPON1 = 6,		//武器2
	ISM_DRESS_MALE = 10,	//衣服(男)
	ISM_DRESS_FEMALE = 11,	//衣服(女)
	ISM_HELMENT = 15,		//头盔
	ISM_NECKLACE0 = 19,		//项链1
	ISM_NECKLACE1 = 20,		//项链2
	ISM_NECKLACE2 = 21,		//项链3
	ISM_RING0 = 22,			//戒指1
	ISM_RING1 = 23,			//戒指2
	ISM_BRACELET0 = 24,		//手镯1
	ISM_BRACELET1 = 26,		//手镯2
	ISM_POISON = 25,		//毒药
	ISM_CANDLE = 30,		//蜡烛
	ISM_BUNDLE = 31,		//捆
	ISM_CHARM = 34,			//符
	ISM_OTHERBUNDLE = 35,	//其他捆
	ISM_SCROLL0 = 36,		//卷轴
	ISM_MEAT = 40,			//肉
	ISM_MISSION = 41,		//任务物品
	ISM_MATERIAL = 42,		//材料
	ISM_MINE = 43,			//矿
	ISM_SPECIAL = 44,		//特殊物品
	ISM_DICE = 45,			//骰子类
	ISM_SPECIAL1 = 46,		//特殊物品
	ISM_GOLDBUNDLE = 47,	//金条类
	ISM_SCROLL1 = 50,		//卷轴
	ISM_BELT = 58,			//腰带
	ISM_SHOES = 81,			//靴子
};

//方向偏移表
const static POINT G_DIROFS[8] =
{
	{ 0, -1},{ 1, -1 },
	{ 1, 0 },{ 1, 1 },
	{ 0, 1 },{ -1, 1 },
	{ -1, 0 },{ -1, -1 },
};

//方向表
const static int G_OPPDIR[8] =
{
	4, 5, 6, 7, 0, 1, 2, 3,
};

//解密
inline int _UnGameCode(CHAR* pIn, BYTE* pOut)
{
	int ilen = (int)strlen(pIn);
	int	iptr = 0;
	int optr = 0;
	BYTE	b1, b2, b3, b4;
	int i = 0;
	for (i = 0; i < ilen / 4; i++)
	{
		b1 = pIn[iptr++] - 0x3b;
		b2 = pIn[iptr++] - 0x3b;
		b3 = pIn[iptr++] - 0x3b;
		b4 = pIn[iptr++] - 0x3b;
		pOut[optr++] = ((b1 & 3) | ((b1 & 0x3c) << 2) | (b4 & 0x0c)) ^ 0xeb;
		pOut[optr++] = ((b2 & 3) | ((b2 & 0x3c) << 2) | ((b4 & 0x03) << 2)) ^ 0xeb;
		pOut[optr++] = ((b3 & 0x3f) | ((b4 & 0x30) << 2)) ^ 0xeb;

	}
	ilen -= i * 4;
	if (ilen == 2)
	{
		b1 = pIn[iptr++] - 0x3b;
		b2 = pIn[iptr++] - 0x3b;
		pOut[optr++] = ((b1 & 3) | ((b1 & 0x3c) << 2) | ((b2 & 0x03) << 2)) ^ 0xeb;
	}
	else if (ilen == 3)
	{
		b1 = pIn[iptr++] - 0x3b;
		b2 = pIn[iptr++] - 0x3b;
		b3 = pIn[iptr++] - 0x3b;
		pOut[optr++] = ((b1 & 3) | ((b1 & 0x3c) << 2) | (b3 & 0x0c)) ^ 0xeb;
		pOut[optr++] = ((b2 & 3) | ((b2 & 0x3c) << 2) | ((b3 & 0x03) << 2)) ^ 0xeb;
	}
	pOut[optr] = 0;
	return optr;
}

//加密
inline int _CodeGameCode(BYTE* pIn, int size, BYTE* pOut)
{
	BYTE b1 = 0, bcal = 0;
	BYTE bflag1 = 0, bflag2 = 0;
	int i = 0;
	int iptr = 0;
	int optr = 0;
	while (iptr < size)
	{
		b1 = pIn[iptr++] ^ 0xeb;//0xeb;
		if (i < 2)
		{
			bcal = b1;
			bcal >>= 2;
			bflag1 = bcal;
			bcal &= 0x3c;
			b1 &= 3;
			bcal |= b1;
			bcal += 0x3b;
			pOut[optr++] = (char)bcal;
			bflag2 = (bflag1 & 3) | (bflag2 << 2);
		}
		else
		{
			bcal = b1;
			bcal &= 0x3f;
			bcal += 0x3b;
			pOut[optr++] = (char)bcal;
			b1 >>= 2;
			b1 &= 0x30;
			b1 |= bflag2;
			b1 += 0x3b;
			pOut[optr++] = (char)b1;
			bflag2 = 0;
		}
		i++;
		i %= 3;
	}
	pOut[optr] = 0;
	if (i == 0)
		return optr;
	pOut[optr++] = bflag2 + 0x3b;
	pOut[optr] = 0;
	return optr;
}

//加密消息
inline int EncodeMsg(char* buffer, DWORD dwFlag, WORD wCmd, WORD w1, WORD w2, WORD w3, LPVOID lpdata = nullptr, int datasize = -1)
{
	MIRMSGHEADER header(dwFlag, wCmd, w1, w2, w3);   //包头数据, 这个函数用组包
	int	codedsize = 1;
	buffer[0] = '#';
	codedsize += _CodeGameCode((BYTE*)&header, sizeof(header), (BYTE*)buffer + codedsize);
	if (lpdata != nullptr)
	{
		if (datasize < 0)
			datasize = (int)strlen((char*)lpdata);
		codedsize += _CodeGameCode((BYTE*)lpdata, datasize, (BYTE*)buffer + codedsize);
	}
	buffer[codedsize++] = '!';
	buffer[codedsize] = 0;
	return codedsize;
}

//数据加密
inline int DBEncode(LPVOID lpData, int iDataSize, char* pszBuffer)
{
	int length = _CodeGameCode((BYTE*)lpData, iDataSize, (BYTE*)pszBuffer);
	for (int i = 0; i < length; i++)
	{
		if (pszBuffer[i] == '\'')pszBuffer[i] = '*';
	}
	return length;
}

//数据解密
inline int DBDecode(char* pszBuffer, int iCodeSize, LPVOID lpData)
{
	for (int i = 0; i < iCodeSize; i++)
	{
		if (pszBuffer[i] == '*')pszBuffer[i] = '\'';
	}
	return _UnGameCode((char*)pszBuffer, (BYTE*)lpData);
}