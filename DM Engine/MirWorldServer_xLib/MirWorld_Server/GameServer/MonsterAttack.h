#pragma once
#include "stdafx.h"
#include <vector>

// 前向声明
class CMonsterEx;
class CAliveObject;
class CLogicMap;

// 特殊怪物攻击处理器
class CMonsterAttack
{
public:
	CMonsterAttack(CMonsterEx* p) { pMon = p; }
	~CMonsterAttack() {}
	// 黑袍辅助
	VOID BlackRobeHelper(CMonsterEx* pFriend);
	// 魔眼攻击
	VOID MoYan(int nDamage, damage_type damagetype);
	// 铁血魔王
	VOID BloodKing(int nDamage);
private:
	// 铁血魔王6大技能

	//1 - 狂潮飞刺
	//	铁甲相击红袍舞动, 魔君躬身以雷霆之势聚力飞刺, 排山
	//	倒海可破千军.
	//	攻击模式：普通攻击
	//	攻击范围：身前两格敌人均会受到重创.
	//	威力指数：★★★★★☆☆☆
	VOID KuangChaoFeiCi(int myX, int myY, int nDir, int nDamage);
	//2 - 召唤魔炎
	//	仰面怒吼在地面上召唤出一片蓝色魔炎, 熊熊燃烧的蓝色
	//	烈焰恐怖异常, 足以致命.
	//	攻击模式：魔法攻击
	//	攻击范围：魔炎所在四格敌人均会受到重创.
	//	威力指数：★★★★★☆☆☆
	VOID ZhaoHuanMoYan(int myX, int myY, int nDamage);
	//3 - 横扫千军
	//	长戟横空扫出, 威力无比, 所过之处生息全无.做为魔王的
	//	杀手锏, 通常能看到此招的人难有生还可能.
	//	攻击模式：物理攻击
	//	攻击范围：魔王身前的半圆范围内均受重创.
	//	威力指数：★★★★★★★★
	VOID HengSaoQianJun(CAliveObject* pTarget, int myX, int myY, int nDir, int nDamage);
	//4 - 退敌魔咒
	//	魔王默念退敌法咒, 指向进攻的敌人, 敌人将会立刻被传
	//	传送到当前区域的随机位置, 使团体作战更加艰难.
	//	攻击模式：魔法战术
	//	攻击范围：指向的敌人将被瞬移
	//	威力指数：★★☆☆☆☆☆☆
	VOID TuiDiMoZhou(CAliveObject* pTarget);
	//5 - 怒震山河
	//	魔王举三叉戟怒撞地面, 产生惊天动地的震荡, 周围敌人均被震伤.
	//	攻击模式：魔法战术
	//	攻击范围：所在四格敌人均会被受重创, 并眩晕2秒.
	//	威力指数：★★★★☆☆☆☆
	VOID NuZhenShanHe(CAliveObject* pTarget, int myX, int myY, int nDir, int nDamage);
	//6 - 移形换位
	//	短距离的移形大法, 魔王瞬间就会变幻到他处, 令对手
	//	眼花缭乱, 攻击也处处落空.魔王的逃生绝技, 当濒临
	//	死亡, 就会使用移形换位试图逃生.
	//	攻击模式：魔法战术
	//	移动范围：当前地图内
	//	威力指数：★★☆☆☆☆☆☆
	VOID YiXingHuanWei(int nDir);

private:
	CMonsterEx* pMon; // 怪物指针
};
